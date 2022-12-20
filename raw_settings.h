#ifndef __RAW_SETTINGS_H__
#define __RAW_SETTINGS_H__

/**********************************************************************/
/*** Data, structs, and Routines to configure serial line           ***/
/*** (/dev/tty*) for raw data transmission using tcsetattr(3)       ***/
/**********************************************************************/

/* Contents
 * ========
 * #include "stty_info.h"    - Parameterize members of struct termios
 * raw_settings[]            - Raw data configuration settings from stty
 * dump_raw_settings(...)    - Dump content of raw_settings
 * find_name_in_control(...) - Find entry in control data in stty_info.h
 * find_name_in_mode(...)    - Find entry in mode data in stty_info.h
 * find_name_in_speeds(...)  - Find entry in speed data in stty_info.h
 * mode_tcflag_pointer(...)  - Return pointer to flag in struct termios
 * stty_process_token(...)   - Process one line of raw_settings[] above
 * stty_raw_config(...)      - Configure serial port for raw data
 * stty_set_speed(...)       - Configure serial port baudrate
 */

#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>

/* mode_info, control_info, etc. */
#include "stty_info.h"

static int raw_settings_debug = 0;
static int raw_settings_debug_speed = 0;

/***********************************************************************
 *** Raw serial line (/dev/tty*) settings from stty:                 ***
 *** - COLUMNS=1 stty -F /dev/ttyUSB0 -a | LC_ALL=C sort             ***
 *** - Empirically determined using [stty] from coreutils            ***
 **********************************************************************/
static char const raw_settings[] =
{ "\
-brkint\n\
-cmspar\n\
-echo\n\
-echoctl\n\
-echoe\n\
-echok\n\
-echoke\n\
-echonl\n\
-echoprt\n\
-extproc\n\
-flusho\n\
-hupcl\n\
-icanon\n\
-icrnl\n\
-iexten\n\
-ignbrk\n\
-igncr\n\
-ignpar\n\
-imaxbel\n\
-inlcr\n\
-isig\n\
-istrip\n\
-iuclc\n\
-iutf8\n\
-ixany\n\
-ixoff\n\
-ixon\n\
-noflsh\n\
-ocrnl\n\
-ofdel\n\
-ofill\n\
-olcuc\n\
-onlcr\n\
-onlret\n\
-onocr\n\
-opost\n\
-parmrk\n\
-parodd\n\
-tostop\n\
-xcase\n\
bs0\n\
clocal\n\
cr0\n\
cread\n\
crtscts\n\
cs8\n\
cstopb\n\
discard = <undef>;\n\
eof = <undef>;\n\
eol = <undef>;\n\
eol2 = <undef>;\n\
erase = <undef>;\n\
ff0\n\
inpck\n\
intr = <undef>;\n\
kill = <undef>;\n\
line = 0;\n\
lnext = <undef>;\n\
min = 1; time = 0;\n\
nl0\n\
parenb\n\
quit = <undef>;\n\
rows 0; columns 0;\n\
rprnt = <undef>;\n\
speed 9600 baud;\n\
start = <undef>;\n\
stop = <undef>;\n\
susp = <undef>;\n\
swtch = <undef>;\n\
tab0\n\
vt0\n\
werase = <undef>;\n\
" };

/* Output the string above to a stream */
static void
dump_raw_settings(FILE* f)
{
    if (f) { fprintf(f,"%s", raw_settings); }
}

/* Find element in static struct control_info array that matches token,
 * where token may have a '-' prefix for inverted flags
 */
static struct control_info*
find_name_in_control(char* token)
{
    struct control_info* prtn;
    if (!token) { return NULL; }
    if ('-'==*token) { ++token; }
    if (!*token) { return NULL; }
    for (prtn = (struct control_info*) control_info; prtn->name; )
    {
        if (!strcmp(token,prtn->name)) { return prtn; }
        ++prtn;
    }
    return NULL;
}

/* Find element in static struct mode_info array that matches token,
 * where token may have a '-' prefix for inverted flags
 */
static struct mode_info*
find_name_in_mode(char* token)
{
    struct mode_info* prtn;
    if (!token) { return NULL; }
    if ('-'==*token) { ++token; }
    if (!*token) { return NULL; }
    for (prtn = (struct mode_info*) mode_info; prtn->name; )
    {
        if (!strcmp(token,prtn->name)) { return prtn; }
        ++prtn;
    }
    return NULL;
}

/* Find element in static struct speed_map array that matches token,
 * where token may have a '-' prefix for inverted flags
 */
static struct speed_map*
find_name_in_speeds(char* token)
{
    struct speed_map* prtn;
    if (!token) { return NULL; }
    if ('-'==*token) { ++token; }
    if (!*token) { return NULL; }
    for (prtn = (struct speed_map*) speeds; prtn->string; )
    {
        if (!strcmp(token,prtn->string)) { return prtn; }
        ++prtn;
    }
    return NULL;
}

/* Return pointer to corresponding flag in struct termios,
 * based on matching .type element of [struct mode_info] instance
 */
static tcflag_t*
mode_tcflag_pointer(enum mode_type type, struct termios* ptermios)
{
    if (control==type) { return &ptermios->c_cflag; }
    if (input  ==type) { return &ptermios->c_iflag; }
    if (output ==type) { return &ptermios->c_oflag; }
    if (local  ==type) { return &ptermios->c_lflag; }
    return NULL;
}

/* Process first token of raw_settings string above */
static int
stty_process_token(char* tok, char* parser, struct termios* new_termios)
{
    int rtn = 0;
    struct mode_info* pmode;
    struct control_info* pcontrol;

    if (!tok) return 0;
    rtn = strlen(tok);
    if (!rtn) return 0;

    /* Process mode name:  set, or clear bit if token has - prefix */
    if ((pmode=find_name_in_mode(tok)))
    {
        tcflag_t* ptcflag = mode_tcflag_pointer(pmode->type, new_termios);
        if (!ptcflag) { return rtn; }  /* do nothing if no flag */

        if ('-'!=*tok)                 /* Set bit and return */
        {
            *ptcflag &= ~pmode->mask;
            *ptcflag |=  pmode->bits;
            if (raw_settings_debug)
            {
                fprintf(stderr,"- Set bit [%s]\n", tok);
            }
            return rtn;
        }

        /* Don't clear non-reversible bit, and return */
        if (0==(pmode->flags & REV))
        {
            if (raw_settings_debug)
            {
                fprintf(stderr,"- ERROR:  cannot clear bit [%s]\n", tok);
            }
            return 0;
        }

        /* Clear reversible bit, and return */
        *ptcflag &= ~pmode->mask & ~pmode->bits;
        if (raw_settings_debug)
        {
            fprintf(stderr,"- Cleared bit [%s]\n", tok);
        }

        return rtn;
    }

    /* Process control character name:  assign value _POSIX_VDISABLE */
    if ((pcontrol=find_name_in_control(tok)))
    {
    char equals[3] = { "" };
    char undef[20] = { "" };
        /* If token matched name, do nothing return unless name is
         * followed by " = <undef>"
         */
        if (2!=sscanf(parser+strlen(tok),"%1s %9s",equals,undef)
           || strcmp("=",equals)
           || strcmp("<undef>;",undef)
           )
        {
            if (raw_settings_debug)
            {
                fprintf(stderr,"- Failed to find [= <undef> after [%s]\n", tok);
            }
            return 0;
        }

        /* Assign _POSIX_VDISABLE */
        new_termios->c_cc[pcontrol->offset] = _POSIX_VDISABLE;
        if (raw_settings_debug)
        {
            fprintf(stderr,"- Disabled control character [%s]\n", tok);
        }
        return rtn;
    }
    if (raw_settings_debug)
    {
        fprintf(stderr,"- WARNING:  ignored [%s]\n", tok);
    }
    return 0;
}

/* Configure serial port (/dev/tty*) for raw data transmission */
static int
stty_raw_config(char* tty_name, char* alt_settings)
{
    /* The struct termios' of the TTY */
    struct termios old_termios;       /* On entry to this routine */
    struct termios new_termios;        /* After applying raw_settings */

    /* Data to parse raw_settings string, one "line: at a time */
    char* parser = alt_settings ? alt_settings : (char*) raw_settings;
    enum WANTS { want_nonws, want_tok0, want_nl };
    enum WANTS istate = want_nonws;
    char tok0[21];

    /* Open the TTY */
    int fd = tty_name ? open(tty_name, O_RDONLY | O_NONBLOCK) : -1;

    /* Ensure [struct termios] state is initially known */
    memset(&old_termios, 0, sizeof old_termios);

    if (raw_settings_debug)
    {
        fprintf(stderr, "stty_raw_config:  tty_name=[%s]; fd=%d\n"
                      , tty_name?tty_name: "<null>", fd);
    }

    if (tty_name)
    {
        /* Error handling */
        if (fd < 0)
        {
            fprintf(stderr, "ERROR:  opening device; ");
            perror(tty_name);
            errno = 0;
            return -1;
        }

        /* Get initial struct termios data */
        if (tty_name && ioctl(fd, TCGETS, &old_termios))
        /*if (tty_name && tcgetattr(fd, &old_termios))*/
        {
            fprintf(stderr, "ERROR:  getting device attributes; ");
            perror(tty_name);
            close(fd);
            errno = 0;
            return -2;
        }
    }

    /* Copy initial struct termios data to new struct */
    memcpy(&new_termios, &old_termios, sizeof old_termios);

    /* Parse settings 1 char at a time, until null terminator */
    while (*parser) {
        switch(istate)
        {

        case want_nl:
            /* Move parser pointer to first char after a newline */
            if ('\n'!=*(parser++)) { break;}
            istate = want_nonws;
            /* N.B. drop through on char after newline */

        case want_nonws:
            /* Move parser pointer to first char after a whitespace */
            if (isspace(*parser)) { ++parser; break;}
            istate = want_tok0;
            /* N.B. drop through on non-ws char */

        case want_tok0:
            /* Read, and process token after newline */
            *tok0 = '\0';
            if (1==sscanf(parser, "%20s", tok0))
            {
                /* Advance pointer by length of token */
                parser += stty_process_token(tok0, parser, &new_termios);
            }
            /* Start looking for newline again */
            istate = want_nl;
            break;

        default:
            fprintf(stderr, "***ERROR:  Unknown state\n");
            parser = 0;
            break;
        } /* switch (*parser) */
    } /* while (*parser) */

    if (tty_name && fd > -1)
    {
        /* Compare old and possibly updated struct termios' */
        char* ptrold = (char*) &old_termios;
        char* ptrold_end = (char*) ((&old_termios)+1);
        char* ptrnew = (char*) &new_termios;
        while (ptrold < ptrold_end)
        {
            if (*ptrold != *ptrnew) { break; }
            ++ptrold;
            ++ptrnew;
        }

        /* If any difference was found, configure new attibutes */
        if (ptrold < ptrold_end)
        {
            if (0 > ioctl(fd, TCSETSF, &new_termios))
            /*if (0 > tcsetattr(fd, TCSAFLUSH, &new_termios))*/
            {
                fprintf(stderr, "ERROR:  setting device attributes; ");
                perror(tty_name);
                close(fd);
                errno = 0;
                return -3;
            }
        }
        close(fd);
    }

    return 0;
}

/* Configure serial port (/dev/tty*) speed (baud rate) */
static int
stty_set_speed(char* tty_name, char* speed_token)
{
    /* The struct termios' of the TTY */
    struct termios old_termios;       /* On entry to this routine */
    struct termios new_termios;        /* After applying raw_settings */
#   ifdef BOTHER
    struct termios2 old_termios2;     /* On entry to this routine */
    struct termios2 new_termios2;      /* After applying raw_settings */
#   endif/*BOTHER*/

    int fd;   /* File descriptor of opened TTY */

    struct speed_map* pspeed;
    int not_bother = 1;

    if (!tty_name) { return -1; }
    if (!*tty_name) { return -1; }
    if (!speed_token) { return -1; }
    if (!*speed_token) { return -1; }

    if (!(pspeed = find_name_in_speeds(speed_token))) { return -1; }

#   ifdef BOTHER
    not_bother = (BOTHER != pspeed->speed);
#   endif/*BOTHER*/

    /* Open the TTY */
    fd = open(tty_name, O_RDONLY | O_NONBLOCK);

    if (raw_settings_debug || raw_settings_debug_speed)
    {
        fprintf(stderr, "stty_set_speed[%sBOTHER]"
                        ":  tty_name=[%s]; fd=%d"
                        "; target-speed=%s(%ldbaud)\n"
                      , not_bother ? "not-" : ""
                      , tty_name, fd
                      , pspeed->string, pspeed->value
                      );
    }

    /* Error handling */
    if (fd < 0)
    {
        fprintf(stderr, "ERROR:  opening device; ");
        perror(tty_name);
        errno = 0;
        return -1;
    }

    /* Ensure [struct termios<2>] states are initially known */
    if (not_bother)
    {
        memset(&old_termios, 0, sizeof old_termios);

        /* Get initial struct termios data */
        if (ioctl(fd, TCGETS, &old_termios))
        /*if (tcgetattr(fd, &old_termios))*/
        {
            fprintf(stderr, "ERROR:  getting termios attributes; ");
            perror(tty_name);
            close(fd);
            errno = 0;
            return -2;
        }

        /* Copy initial struct termios data to new struct */
        memcpy(&new_termios, &old_termios, sizeof old_termios);

        /* Update baudrate bits in new termios struct */
        new_termios.c_cflag &= ~CBAUD;
        new_termios.c_cflag |= pspeed->speed;

        /* Write new termios struct to device */
        if (0 > ioctl(fd, TCSETSF, &new_termios))
        /*if (0 > tcsetattr(fd, TCSAFLUSH, &new_termios))*/
        {
            fprintf(stderr, "ERROR:  setting device attributes; ");
            perror(tty_name);
            close(fd);
            errno = 0;
            return -3;
        }

        if (raw_settings_debug || raw_settings_debug_speed)
        {
            fprintf(stderr, "stty_set_speed[not-BOTHER] success"
                            ": speed=%s(%ldbaud)\n"
                          , pspeed->string, pspeed->value);
        }
    }
#   ifdef BOTHER
    else
    {
        memset(&old_termios2, 0, sizeof old_termios2);

        /* Get initial struct termios data */
        if (ioctl(fd, TCGETS2, &old_termios2))
        {
            fprintf(stderr, "ERROR:  getting termios2 attributes; ");
            perror(tty_name);
            close(fd);
            errno = 0;
            return -2;
        }

        if (raw_settings_debug || raw_settings_debug_speed)
        {
            fprintf(stderr, "stty_set_speed[BOTHER], old_termios2"
                            ":  .c_cflag&CBAUD=0o%06o"
                            "; .c_ispeed=%u"
                            "; .c_ospeed=%u\n"
                          , old_termios2.c_cflag & CBAUD
                          , old_termios2.c_ispeed
                          , old_termios2.c_ospeed
                          );
        }

        /* Copy initial struct termios data to new struct */
        memcpy(&new_termios2, &old_termios2, sizeof old_termios2);

        /* Update baudrate bits in new termios struct */
        new_termios2.c_cflag &= ~CBAUD;
        new_termios2.c_cflag |= BOTHER;

        /* Update baudrates in new termios struct */
        new_termios2.c_ispeed = new_termios2.c_ospeed = pspeed->value;

        if (raw_settings_debug || raw_settings_debug_speed)
        {
            fprintf(stderr, "stty_set_speed[BOTHER], new_termios2"
                            ":  .c_cflag&CBAUD=0o%06o"
                            "; .c_ispeed=%u"
                            "; .c_ospeed=%u\n"
                          , new_termios2.c_cflag & CBAUD
                          , new_termios2.c_ispeed
                          , new_termios2.c_ospeed
                          );
        }

        /* Write new termios struct to device */
        if (0 > ioctl(fd, TCSETSF2, &new_termios2))
        {
            fprintf(stderr, "ERROR:  setting termios2 attributes; ");
            perror(tty_name);
            close(fd);
            errno = 0;
            return -3;
        }

        if (raw_settings_debug || raw_settings_debug_speed)
        {
            struct termios2 chk_termios2;
            memset(&old_termios2, 0, sizeof chk_termios2);
            if (ioctl(fd, TCGETS2, &chk_termios2))
            {
                fprintf(stderr, "ERROR:  checking termios2 attribs; ");
                perror(tty_name);
                close(fd);
                errno = 0;
                return -2;
            }
            fprintf(stderr, "stty_set_speed[BOTHER], chk_termios2"
                            ":  .c_cflag&CBAUD=0o%06o"
                            "; .c_ispeed=%u"
                            "; .c_ospeed=%u\n"
                          , chk_termios2.c_cflag & CBAUD
                          , chk_termios2.c_ispeed
                          , chk_termios2.c_ospeed
                          );
        }

        if (raw_settings_debug || raw_settings_debug_speed)
        {
            fprintf(stderr, "stty_set_speed[BOTHER]"
                            ": speed=%s(%ldbaud)\n"
                          , pspeed->string, pspeed->value);
        }
    }
#   endif/*BOTHER*/

    close(fd);

    return 0;
}
#endif/*__RAW_SETTINGS_H__*/
