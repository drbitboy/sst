#ifndef __SST_H__
#define __SST_H__

/**********************************************************************/
/*** Routines to send a pre-defined stream of binary                ***/
/*** 8-bit characters to a TTY (or other file)                      ***/
/**********************************************************************/

/* Contents
 * ========
 * fill_to_send()              - fill source data array, return pointer
 * dump_to_send(...)           - Dump source data array to output stream
 * typedef ... *pSEQUENCE8BIT  - Struct to use source data array
 * send_chars(...)             - Automate large writy of source data
 * typedef ... *pRECVSTATUS    - Struct with forked reader status
 * recv_chars(...)             - Read data from TTY
 * tohere(...)                 - High-frequency debug logging
 * - #ifdef DOTOHERE           - Control compile-time usage of tohere()
 * - #define TOHEREI(I)        - Control compile-time usage of tohere()
 * - #define TOHERE(I)         - Control compile-time usage of tohere()
 */

#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>

/***********************************************************************
 * Stream of 8-bit characters, alternating high bit 8, finish with CRLF:
 *   255, 127, 254, 126, ..., 160, 32, 128, 0, CR, NL
 *   - so no control characters except 128, 0, CR, NL
 *   - so total count is (256chars - (64-4)control) = 196chars
 *
 * Sequence will be subsets of that array, ending with NL (newline)
 * i) send last three characters (0, CR, NL)
 * ii) send last four characters (128, 0, CR, NL)
 * ...) ...
 * cxcv) send last 195 characters (127, 254, ..., CR, NL)
 * cxcvi) send all 196 characters (255, 127, ..., CR, NL)
 * cxcvii) return to (i)
 */

#define LSEND 196              /* Length of array of characters */
static char to_send[LSEND];    /* Array of characters */
static char* p_to_send_end;    /* Pointer to one past last character */


/**********************************************************************/
/* Routine to fill array and initialize p_to_send_end pointer */
static void
fill_to_send()
{
    unsigned char* p = (unsigned char*) to_send;
    unsigned char v = 128;
    if ((to_send + LSEND) == p_to_send_end) { return; }
    while ( v-- > 32)
    {
        *(p++) = v | 128;   /* chars with high bit 8 = 1 */
        *(p++) = v;         /* chars with high bit 8 = 0 */
    }
    *(p++) = 128;
    *(p++) = 0;
    *(p++) = '\r';
    *(p++) = '\n';
    p_to_send_end = (char*) p;
}


/**********************************************************************/
/* Dump entire array */
static void
dump_to_send(FILE* fout)
{
   fill_to_send();
   fwrite(to_send, 1, LSEND, fout);
}


/**********************************************************************/
/* Structure to keep track of sequence of array subsets */
typedef struct sequence8bitstr
{
    char* p_line;
    char* p_next;
    char* p;
    char last;
} SEQUENCE8BIT, *pSEQUENCE8BIT;


/**********************************************************************/
#ifdef DOTOHERE
/* High-frequency debug logging (e.g. gcc -DDOTOHERE)
 * - These routines cause system crashes and reboots in NVIDIA Jetson
 *   platforms, so this tohere(...) routine is a compile-time-enabled
 *   way to know, approximately, what is the last line of code that
 *   executes before a crash occurs
 */
void tohere(const char* fyle, const char* func, const int lyne, const int val)
{
    fprintf(stderr,"TOHERE:  file=[%s]; function=[%s]; line=%d; val=%d\n"
                  , fyle, func, lyne, val
           );
    fflush(stderr);
}

/* The macro TOHEREI(I) is a standard way to control the calling of
 * tohere(...)
 * - There will be another macro, TOHERE(I) that will be used where
 *   this high-freequency debug logging is desired.
 *   - Macro TOHERE(I) will
 *     - either be #defined empty to disable the logging,
 *     - or be #defined equivalent to TOHEREI(I) to enable the logging
 * - The parameter I is an integer that is used when there is a need to
 *   log more information than the file, function, and line number
 *   - The parameter I will normally be 0
 */
#define TOHEREI(I) tohere(__FILE__,__FUNCTION__,__LINE__,(int)(I));

#else
/* If the macro DOTOHERE is not #defined, then the TOHEREI(I) macro is
 * empty, which disables any calls to the non-existent tohere(...)
 * routine, regardless how TOHERE(I) is #defined elsewhere
 */
#define TOHEREI(I)
#endif

/* Initialize no debug logging */
#define TOHERE(I)


/**********************************************************************/
/* Routine to send sequence of array subsets to open file descriptor
 *
 * Return value:  how many characters were sent:  sum of write()'s
 *
 * Input arguments:
 *            fd - open file descriptor
 *     remaining - How many total characters to send
 *         pseq8 - pSEQUENCE8BIT struct (see above)
 *
 * Output arguments (pointers):
 *        ptries - Count of how many writes
 *       pagains - Count of EAGAIN/EWOULDBLOCK write errors
 */
static ssize_t
send_chars(int fd, size_t remaining, pSEQUENCE8BIT pseq8
          , size_t* ptries, size_t* peagains)
{
    /* Initialize counters */
    size_t lsent = 0;
TOHERE(0)
    *ptries = *peagains = 0;

    /* Initialize array to send */
TOHERE(0)
    fill_to_send();

    /* Initialize pointers into array of chars
     * - ->p is set past end of array to trigger new line
     */
TOHERE(0)
    pseq8->p = p_to_send_end;       /* next char to send */
    pseq8->p_line = pseq8->p - 3;   /* start of next line to send */

    /* Loop over writes until target character count has been sent */
TOHERE(0)
    while (remaining > 0)
    {
    size_t count_this_pass;
    ssize_t iwrite;

TOHERE(0)
        ++*ptries;

        /* When start of chars to send is too early or late,
         * reset to send last three chars (NUL, CR, NL)
         */
TOHERE(0)
        if (pseq8->p_line<to_send || pseq8->p_line>(p_to_send_end-3))
        {
TOHERE(0)
            pseq8->p_line  = p_to_send_end-3;
        }

        /* When next character to send is past end of array,
         * it is likely because the entire line has been sent, so
         * - move to start of next line to send, and
         * - decrement the start of the line for next time
         */
TOHERE(0)
        if (pseq8->p >= p_to_send_end || pseq8->p < to_send)
        {
TOHERE(0)
            pseq8->p = pseq8->p_line--;
        }

        /* Get count of chars to send on this pass (line), ensure result
         * does not exceed total chars to send for all lines
         */
TOHERE(0)
        count_this_pass = p_to_send_end - pseq8->p;
#undef TOHERE
#define TOHERE(I) TOHEREI(I)
TOHERE(remaining)
        if (count_this_pass>remaining) { count_this_pass = remaining; }

        /* Write up to that many characters */
TOHERE(count_this_pass)
        iwrite = write(fd, pseq8->p, count_this_pass);

        /* Handle errors */
TOHERE(iwrite)
        if (iwrite < 0)
        {
            /* Ignore, but keep track of, blocked writes */
            /* - should only occur if fd was opened O_NONBLOCK */
TOHERE(errno)
            if (EAGAIN==errno || EWOULDBLOCK==errno)
            {
TOHERE(*peagains)
                ++*peagains;
                errno = 0;
                continue;
            }
            /* Fail on all other errors */
TOHERE(-99)
            perror("send_chars");
            return -1;
        }
        /* Update counters and next-character pointer */
TOHERE(0)
        remaining -= iwrite;
TOHERE(remaining)
#undef TOHERE
#define TOHERE(I)
        lsent += iwrite;
TOHERE(0)
        pseq8->p += iwrite;
    }
TOHERE(0)
    return lsent;
} /* send_chars(...) */


/**********************************************************************/
/* Struct to return status from forked reader (cf. recv_char(...)) */
typedef struct RECVSTATUSstr
{
    int status;
    int m_errno;
    size_t count;
    size_t reads;
} RECVSTATUS, *pRECVSTATUS;

#undef TOHERE
#define TOHERE(I)


/**********************************************************************/
/* Fork process to read loopback data sent by send_char(...) above */
/* Return value:  -1 or file descriptor of read pipe from data reader */
static int
recv_chars(char* tty_name, size_t count)
{
    int fdtty;
    int fdpipes[2];
    pid_t gcpid;     /* Grandchild PID */
    pid_t ppid;      /* Parent (child of grandparent) PID */
    RECVSTATUS buf;
    int timeouts_remaining = 4;
    int iwrite;

TOHERE(0)
    memset(&buf, 0, sizeof buf);

    /* Open bi-directional pipe pair to get grandchild results */
TOHERE(0)
    if (0 > pipe(fdpipes))
    {
TOHERE(0)
        perror("recv_chars=>pipe");
TOHERE(0)
        return -1;
    }

    /* Fork child of grandparent (parent), which will fork grandchild */
TOHERE(0)
    if (0 > (ppid = fork()))
    {
TOHERE(0)
        perror("recv_chars=>fork");
TOHERE(0)
        return -1;
    };

    /* Grandparent (main process) waits
     * - first for 1st child (parent) to exit after forking grandchild,
     * - then for read ack via pipe from grandchild
     */
TOHERE(0)
    if (ppid)
    {
        fd_set rfds;
        struct timeval tv;
        int retval;

TOHERE(0)
        close(fdpipes[1]);   /* Grandparent closes write pipe */

        /* Wait for 1st child (parent) exit after forking grandchild */
TOHERE(0)
        if ((-1==waitpid(ppid,0,0)))
        {
TOHERE(0)
            perror("recv_char=>waitpid");
TOHERE(0)
            close(fdpipes[0]);
TOHERE(0)
            return -1;
        }

        /* Set up to read from read pipe from grandchild */
TOHERE(0)
        FD_ZERO(&rfds);
TOHERE(0)
        FD_SET(fdpipes[0], &rfds);
TOHERE(0)
        tv.tv_sec = 5;
TOHERE(0)
        tv.tv_usec = 0;

        /* Wait for data to be available on pipe */
TOHERE(0)
        retval = select(fdpipes[0]+1, &rfds, 0, 0, &tv);
TOHERE(0)
        if (0 > retval)
        {
TOHERE(0)
            perror("recv_char=>select(pipe)");
TOHERE(0)
            close(fdpipes[0]);
TOHERE(0)
            return -1;
        }
TOHERE(0)
        if (!retval)
        {
TOHERE(0)
            perror("recv_char=>select(pipe)=>timeout");
TOHERE(0)
            close(fdpipes[0]);
TOHERE(0)
            return -1;
        }

        /* Read data from pipe */
TOHERE(0)
        if ((sizeof buf) != read(fdpipes[0],&buf,sizeof buf))
        {
TOHERE(0)
            perror("recv_char=>read-pipe");
TOHERE(0)
            close(fdpipes[0]);
TOHERE(0)
            return -1;
        }

TOHERE(0)
        if (buf.status)
        {
TOHERE(0)
            close(fdpipes[0]);
TOHERE(0)
            errno = buf.m_errno;
TOHERE(0)
            perror("recv_char=>grandchild-init-failed");
TOHERE(0)
            return -1;
        }
TOHERE(0)
        return fdpipes[0];
    } /* End of grandparent (main) process in this routine */

    /* To here, this is child-of grandparent, parent-of-grandchild
     * - Its only task is to fork the grandchild process, then exit
     * - This is done to ensure data reader is not a zombie process
     */

    /* Fork grandchild process to read the data sent by send_char */
TOHERE(0)
    if ((gcpid = fork()))
    {
TOHERE(0)
        if (0 > gcpid)
        {
TOHERE(0)
            perror("recv_chars=>fork-of-grandchild");
TOHERE(0)
            exit(-1);
        }
        /* Successful fork of grandchild; close read pipe and exit */
TOHERE(0)
        close(fdpipes[0]);
        exit(0);
    }

    /* To here, this is grandchild process, with several tasks
     * 1) Open TTY for read
     * 2) Send initial success status to pipe
     * 3) Read data from TTY
     * 4) Send final success status to pipe
     * 5) Exit
     */

    /* 1) Open TTY for read */
TOHERE(0)
    if (0 > (fdtty=open(tty_name,O_RDONLY | O_NONBLOCK)))
    {
TOHERE(0)
        buf.status = fdtty;
TOHERE(0)
        buf.m_errno = errno;
TOHERE(0)
        write(fdpipes[1],&buf,sizeof buf);
TOHERE(0)
        perror("recv_chars=>open(tty)");
TOHERE(0)
        exit(-1);
    }
TOHERE(0)
    if (!isatty(fdtty))
    {
TOHERE(0)
        buf.status = -1;
TOHERE(0)
        buf.m_errno = errno;
TOHERE(0)
        write(fdpipes[1],&buf,sizeof buf);
TOHERE(0)
        close(fdtty);
TOHERE(0)
        exit(-1);
    }

    /* 2) Send initial success status to pipe */
TOHERE(0)
    write(fdpipes[1],&buf,sizeof buf);

    /* 3) Read data from TTY */
TOHERE(0)
    while (buf.count < count)
    {
        char databuf[1024];
        int retval;
        struct timeval tv;
        fd_set rfds;

#undef TOHERE
#define TOHERE(I) TOHEREI(I)

        /* Setup for select with 3s timeout*/
TOHERE(0)
        tv.tv_sec = 3;
TOHERE(0)
        tv.tv_usec = 0;
TOHERE(0)
        FD_ZERO(&rfds);
TOHERE(0)
        FD_SET(fdtty, &rfds);

        /* Setup for select */
TOHERE(0)
        if (0 > (retval = select(fdtty+1,&rfds,0,0,&tv)))
        {
TOHERE(0)
            perror("recv_chars=>select(tty)");
TOHERE(0)
            buf.m_errno = errno;
TOHERE(0)
            buf.status = retval;
TOHERE(0)
            break;
        }

        /* Select timed out */
TOHERE(retval)
        if (!retval)
        {
TOHERE(errno)
            perror("recv_chars=>select(tty)=>timeout");
TOHERE(timeouts_remaining)
            if (--timeouts_remaining < 1) { break; }
TOHERE(timeouts_remaining)
            continue;
        }

        /* Read data */
TOHERE(buf.reads)
        ++buf.reads;
TOHERE(buf.reads)
        if (0 > (retval = read(fdtty,databuf, 1024)))
        {
TOHERE(retval)
            perror("recv_chars=>read(tty)");
TOHERE(errno)
            buf.m_errno = errno;
TOHERE(0)
            buf.status = retval;
TOHERE(0)
            break;
        }
TOHERE(retval)
        buf.count += retval;
TOHERE(buf.count)
    }

    /* 4) Send status to pipe */
#undef TOHERE
#define TOHERE(I) TOHEREI(I)
TOHERE(buf.count)
    iwrite = write(fdpipes[1],&buf,sizeof buf);

    /* 5) Clean up and exit */
TOHERE(iwrite)
    close(fdtty);
#undef TOHERE
#define TOHERE(I)
TOHERE(0)
    close(fdpipes[1]);

TOHERE(0)
    exit(0*iwrite);
} /* static int recv_chars(char* tty_name, size_t count) */

#endif/*__SST_H__*/
