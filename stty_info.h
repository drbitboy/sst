#ifndef __STTY_INFO_H__
#define __STTY_INFO_H__

/***********************************************************************
 * Parameterize contents of [struct termios*], to simplify configuration
 * of serial ports (TTYs)
 *
 * *** N.B. Heavily borrowed from Linux coreutils/stty.c
 */

#include <stddef.h>
#include <asm/termbits.h>

#if 0
#include <termios.h>
#ifndef BOTHER
#define BOTHER 0100000
#endif/*BOTHER*/
#endif/*0*/

#if 0

/* Typical (expected) [struct termios*] layout */

struct termios2 {
        tcflag_t c_iflag;              /* input mode flags */
        tcflag_t c_oflag;              /* output mode flags */
        tcflag_t c_cflag;              /* control mode flags */
        tcflag_t c_lflag;              /* local mode flags */
        cc_t c_line;                   /* line discipline */
        cc_t c_cc[NCCS];               /* control characters */
        speed_t c_ispeed;              /* input speed */
        speed_t c_ospeed;              /* output speed */
};

#endif/*0*/

/* mode_info and control_info */

/* Enumerate members of [struct termios2] to allow declaring to which
 * member a "parameterized" mode applies.
 */
enum mode_type
  {
    control, input, output, local, combination
  };

/* Flags for 'struct mode_info'. */
#define REV 4                     /* Can be turned off by prepending '-'. */
#define NO_SETATTR 16             /* tcsetattr not used to set mode bits.  */

/* Each mode.  */
struct mode_info
  {
    char const *name;           /* Name given on command line.  */
    enum mode_type type;        /* Enum of structure element to change. */
    char flags;                 /* Setting and display options.  */
    unsigned long bits;         /* Bits to set for this mode.  */
    unsigned long mask;         /* Other bits to turn off for this mode.  */
  };

/* Parameterize [struct termios2] mode flags by name */
static struct mode_info const mode_info[] =
{
  {"parenb", control, REV, PARENB, 0},
  {"parodd", control, REV, PARODD, 0},
#ifdef CMSPAR
  {"cmspar", control, REV, CMSPAR, 0},
#endif
  {"cs5", control, 0, CS5, CSIZE},
  {"cs6", control, 0, CS6, CSIZE},
  {"cs7", control, 0, CS7, CSIZE},
  {"cs8", control, 0, CS8, CSIZE},
  {"hupcl", control, REV, HUPCL, 0},
  {"cstopb", control, REV, CSTOPB, 0},
  {"cread", control, REV, CREAD, 0},
  {"clocal", control, REV, CLOCAL, 0},
#ifdef CRTSCTS
  {"crtscts", control, REV, CRTSCTS, 0},
#endif
#ifdef CDTRDSR
  {"cdtrdsr", control, REV, CDTRDSR, 0},
#endif

  {"ignbrk", input, REV, IGNBRK, 0},
  {"brkint", input, REV, BRKINT, 0},
  {"ignpar", input, REV, IGNPAR, 0},
  {"parmrk", input, REV, PARMRK, 0},
  {"inpck", input, REV, INPCK, 0},
  {"istrip", input, REV, ISTRIP, 0},
  {"inlcr", input, REV, INLCR, 0},
  {"igncr", input, REV, IGNCR, 0},
  {"icrnl", input, REV, ICRNL, 0},
  {"ixon", input, REV, IXON, 0},
  {"ixoff", input, REV, IXOFF, 0},
#ifdef IUCLC
  {"iuclc", input, REV, IUCLC, 0},
#endif
#ifdef IXANY
  {"ixany", input, REV, IXANY, 0},
#endif
#ifdef IMAXBEL
  {"imaxbel", input, REV, IMAXBEL, 0},
#endif
#ifdef IUTF8
  {"iutf8", input, REV, IUTF8, 0},
#endif

  {"opost", output, REV, OPOST, 0},
#ifdef OLCUC
  {"olcuc", output, REV, OLCUC, 0},
#endif
#ifdef OCRNL
  {"ocrnl", output, REV, OCRNL, 0},
#endif
#ifdef ONLCR
  {"onlcr", output, REV, ONLCR, 0},
#endif
#ifdef ONOCR
  {"onocr", output, REV, ONOCR, 0},
#endif
#ifdef ONLRET
  {"onlret", output, REV, ONLRET, 0},
#endif
#ifdef OFILL
  {"ofill", output, REV, OFILL, 0},
#endif
#ifdef OFDEL
  {"ofdel", output, REV, OFDEL, 0},
#endif
#ifdef NLDLY
  {"nl0", output, 0, NL0, NLDLY},
#endif
#ifdef CRDLY
  {"cr0", output, 0, CR0, CRDLY},
#endif
#ifdef TABDLY
# ifdef TAB0
  {"tab0", output, 0, TAB0, TABDLY},
# endif
#endif
#ifdef BSDLY
  {"bs0", output, 0, BS0, BSDLY},
#endif
#ifdef VTDLY
  {"vt0", output, 0, VT0, VTDLY},
#endif
#ifdef FFDLY
  {"ff0", output, 0, FF0, FFDLY},
#endif

  {"isig", local, REV, ISIG, 0},
  {"icanon", local, REV, ICANON, 0},
#ifdef IEXTEN
  {"iexten", local, REV, IEXTEN, 0},
#endif
  {"echo", local, REV, ECHO, 0},
  {"echoe", local, REV, ECHOE, 0},
  {"echok", local, REV, ECHOK, 0},
  {"echonl", local, REV, ECHONL, 0},
  {"noflsh", local, REV, NOFLSH, 0},
#ifdef XCASE
  {"xcase", local, REV, XCASE, 0},
#endif
#ifdef TOSTOP
  {"tostop", local, REV, TOSTOP, 0},
#endif
#ifdef ECHOPRT
  {"echoprt", local, REV, ECHOPRT, 0},
#endif
#ifdef ECHOCTL
  {"echoctl", local, REV, ECHOCTL, 0},
#endif
#ifdef ECHOKE
  {"echoke", local, REV, ECHOKE, 0},
#endif
#ifdef FLUSHO
  {"flusho", local, REV, FLUSHO, 0},
#endif
#if defined TIOCEXT
  {"extproc", local, REV | NO_SETATTR, EXTPROC, 0},
#elif defined EXTPROC
  {"extproc", local, REV, EXTPROC, 0},
#endif

  /* List termination */
  {NULL, control, 0, 0, 0}
}; /* static struct mode_info const mode_info[]
      Parameterize [struct termios2] mode flags */

/* Control character settings.  */
struct control_info
  {
    char const *name;		/* Name given on command line.  */
    cc_t saneval;		/* Value to set for 'stty sane'.  */
    size_t offset;		/* Offset in c_cc.  */
  };

/* Control characters. */
#ifndef _POSIX_VDISABLE
#define _POSIX_VDISABLE 0
#endif
#if defined VEOL2 && !defined CEOL2
#define CEOL2 _POSIX_VDISABLE
#endif

/* Parameterize tty control characters, by name */
static struct control_info const control_info[] =
{
  {"intr", CINTR, VINTR},
  {"quit", CQUIT, VQUIT},
  {"erase", CERASE, VERASE},
  {"kill", CKILL, VKILL},
  {"eof", CEOF, VEOF},
  {"eol", CEOL, VEOL},
#ifdef VEOL2
  {"eol2", CEOL2, VEOL2},
#endif
#ifdef VSWTCH
  {"swtch", CSWTCH, VSWTCH},
#endif
  {"start", CSTART, VSTART},
  {"stop", CSTOP, VSTOP},
  {"susp", CSUSP, VSUSP},
#ifdef VDSUSP
  {"dsusp", CDSUSP, VDSUSP},
#endif
#ifdef VREPRINT
  {"rprnt", CRPRNT, VREPRINT},
#else
# ifdef CREPRINT /* HPUX 10.20 needs this */
  {"rprnt", CRPRNT, CREPRINT},
# endif
#endif
#ifdef VWERASE
  {"werase", CWERASE, VWERASE},
#endif
#ifdef VLNEXT
  {"lnext", CLNEXT, VLNEXT},
#endif
#if defined VDISCARD && !defined VFLUSHO
#   define VFLUSHO VDISCARD
#endif
#if defined CDISCARD && !defined CFLUSHO
#   define CFLUSHO CDISCARD
#endif
#ifdef VFLUSHO
  {"discard", CFLUSHO, VFLUSHO},
#endif
#ifdef VSTATUS
  {"status", CSTATUS, VSTATUS},
#endif

  /* These must be last because of the display routines. */
  //{"min", 1, VMIN},
  //{"time", 0, VTIME},

  /* List termination */
  {NULL, 0, 0}
}; /* static struct control_info const control_info[]
      Parameterize tty control characters */

/* Struct used to populate list of parameterized aud rate settings */
struct speed_map
{
  char const *string;		/* ASCII representation. */
  speed_t speed;		/* Internal form. */
  unsigned long int value;	/* Numeric value. */
};

/* Parameterize tty baud rates, by name */
static struct speed_map const speeds[] =
{
  {"0", B0, 0},
  {"50", B50, 50},
  {"75", B75, 75},
  {"110", B110, 110},
  {"134", B134, 134},
  {"134.5", B134, 134},
  {"150", B150, 150},
  {"200", B200, 200},
  {"300", B300, 300},
  {"600", B600, 600},
  {"1200", B1200, 1200},
  {"1800", B1800, 1800},
  {"2400", B2400, 2400},
  {"4800", B4800, 4800},
  {"9600", B9600, 9600},
  {"19200", B19200, 19200},
  {"38400", B38400, 38400},
  {"exta", B19200, 19200},
  {"extb", B38400, 38400},
#ifdef B57600
  {"57600", B57600, 57600},
#endif
#ifdef B115200
  {"115200", B115200, 115200},
#endif
#ifdef B230400
  {"230400", B230400, 230400},
#endif
#ifdef B460800
  {"460800", B460800, 460800},
#endif
#ifdef B500000
  {"500000", B500000, 500000},
#endif
#ifdef B576000
  {"576000", B576000, 576000},
#endif
#ifdef B921600
  {"921600", B921600, 921600},
#endif
#ifdef B1000000
  {"1000000", B1000000, 1000000},
  {"1M", B1000000, 1000000},
#endif
#ifdef B1152000
  {"1152000", B1152000, 1152000},
  {"1.152M", B1152000, 1152000},
#endif
#ifdef B1500000
  {"1500000", B1500000, 1500000},
  {"1.5M", B1500000, 1500000},
#endif
#ifdef B2000000
  {"2000000", B2000000, 2000000},
  {"2M", B2000000, 2000000},
#endif
#ifdef B2500000
  {"2500000", B2500000, 2500000},
  {"2.5M", B2500000, 2500000},
#endif
#ifdef B3000000
  {"3000000", B3000000, 3000000},
  {"3M", B3000000, 3000000},
#endif
#ifdef B3500000
  {"3500000", B3500000, 3500000},
  {"3.5M", B3500000, 3500000},
#endif
#ifdef B4000000
  {"4000000", B4000000, 4000000},
  {"4M", B4000000, 4000000},
#endif

#ifdef BOTHER
/* If the .speed internal form member is the macro BOTHER, then it does
 * not specify a specific baudrate known to the system; and the actual
 * rate will be in the member .value.  It is assumed the tty driver
 * knows how to handle these baudrates.
 */
  {"8000000", BOTHER, 8390625},
  {"8M", BOTHER, 8390625},
  {"12000000", BOTHER, 12000000},
  {"12M", BOTHER, 12000000},
  {"12500000", BOTHER, 12500000},
  {"12.5M", BOTHER, 12500000},
#endif

  /* List termination */
  {NULL, 0, 0}
}; /* static struct speed_map const speeds[]
      Parameterize tty baud rates, by name */

#endif/*__STTY_INFO_H__*/
