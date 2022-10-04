#ifndef __SST_H__
#define __SST_H__

/**********************************************************************/
/*** Routines to send a pre-defined stream of binary                ***/
/*** 8-bit characters to a TTY (or other file)                      ***/
/**********************************************************************/

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

/* Dump entire array */
static void dump_to_send(FILE* fout) {
   fill_to_send();
   fwrite(to_send, 1, LSEND, fout);
}

/* Structure to keep track of sequence of array subsets */
typedef struct sequence8bitstr
{
    char* p_line;
    char* p_next;
    char* p;
    char last;
} SEQUENCE8BIT, *pSEQUENCE8BIT;

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
    *ptries = *peagains = 0;

    /* Initialize array to send */
    fill_to_send();

    /* Initialize pointers into array of chars
     * - ->p is set past end of array to trigger new line
     */
    pseq8->p = p_to_send_end;       /* next char to send */
    pseq8->p_line = pseq8->p - 3;   /* start of next line to send */

    /* Loop over writes until target character count has been sent */
    while (remaining > 0)
    {
    size_t count_this_pass;
    ssize_t iwrite;

       ++*ptries;

        /* When start of chars to send is too early or late,
         * reset to send last three chars (NUL, CR, NL)
         */
        if (pseq8->p_line<to_send || pseq8->p_line>(p_to_send_end-3))
        {
            pseq8->p_line  = p_to_send_end-3;
        }

        /* When next character to send is past end of array,
         * it is likely because the entire line has been sent, so
         * - move to start of next line to send, and
         * - decrement the start of the line for next time
         */
        if (pseq8->p >= p_to_send_end || pseq8->p < to_send)
        {
            pseq8->p = pseq8->p_line--;
        }

        /* Get count of chars to send on this pass (line), ensure result
         * does not exceed total chars to send for all lines
         */
        count_this_pass = p_to_send_end - pseq8->p;
        if (count_this_pass>remaining) { count_this_pass = remaining; }

        /* Write up to that many characters */
        iwrite = write(fd, pseq8->p, count_this_pass);

        /* Handle errors */
        if (iwrite < 0)
        {
            /* Ignore, but keep track of, blocked writes */
            /* - should only occur if fd was opened O_NONBLOCK */
            if (EAGAIN==errno || EWOULDBLOCK==errno)
            {
                ++*peagains;
                errno = 0;
                continue;
            }
            /* Fail on all other errors */
            perror("send_chars");
            return -1;
        }
        /* Update counters and next-character pointer */
        remaining -= iwrite;
        lsent += iwrite;
        pseq8->p += iwrite;
    }
    return lsent;
}

typedef struct RECVSTATUSstr
{
    int status;
    int m_errno;
    size_t count;
    size_t reads;
} RECVSTATUS, *pRECVSTATUS;

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

    memset(&buf, 0, sizeof buf);

    /* Open bi-directional pipe pair to get grandchild results */
    if (0 > pipe(fdpipes))
    {
        perror("recv_chars=>pipe");
        return -1;
    }

    /* Fork child of grandparent (parent), which will fork grandchild */
    if (0 > (ppid = fork()))
    {
        perror("recv_chars=>fork");
        return -1;
    };

    /* Grandparent (main process) waits
     * - first for 1st child (parent) to exit after forking grandchild,
     * - then for read ack via pipe from grandchild
     */
    if (ppid)
    {
        fd_set rfds;
        struct timeval tv;
        int retval;

        close(fdpipes[1]);   /* Grandparent closes write pipe */

        /* Wait for 1st child (parent) exit after forking grandchild */
        if ((-1==waitpid(ppid,0,0)))
        {
            perror("recv_char=>waitpid");
            close(fdpipes[0]);
            return -1;
        }

        /* Set up to read from read pipe from grandchild */
        FD_ZERO(&rfds);
        FD_SET(fdpipes[0], &rfds);
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        /* Wait for data to be available on pipe */
        retval = select(fdpipes[0]+1, &rfds, 0, 0, &tv);
        if (0 > retval)
        {
            perror("recv_char=>select(pipe)");
            close(fdpipes[0]);
            return -1;
        }
        if (!retval)
        {
            perror("recv_char=>select(pipe)=>timeout");
            close(fdpipes[0]);
            return -1;
        }

        /* Read data from pipe */
        if ((sizeof buf) != read(fdpipes[0],&buf,sizeof buf))
        {
            perror("recv_char=>read-pipe");
            close(fdpipes[0]);
            return -1;
        }

        if (buf.status)
        {
            close(fdpipes[0]);
            errno = buf.m_errno;
            perror("recv_char=>grandchild-init-failed");
            return -1;
        }
        return fdpipes[0];
    } /* End of grandparent (main) process in this routine */

    /* To here, this is child-of grandparent, parent-of-grandchild
     * - Its only task is to fork the grandchild process, then exit
     * - This is done to ensure data reader is not a zombie process
     */

    /* Fork grandchild process to read the data sent by send_char */
    if ((gcpid = fork()))
    {
        if (0 > gcpid)
        {
            perror("recv_chars=>fork-of-grandchild");
            exit(-1);
        }
        /* Successful fork of grandchild; close read pipe and exit */
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
    if (0 > (fdtty=open(tty_name,O_RDONLY | O_NONBLOCK)))
    {
        buf.status = fdtty;
        buf.m_errno = errno;
        write(fdpipes[1],&buf,sizeof buf);
        perror("recv_chars=>open(tty)");
        exit(-1);
    }
    if (!isatty(fdtty))
    {
        buf.status = -1;
        buf.m_errno = errno;
        write(fdpipes[1],&buf,sizeof buf);
        close(fdtty);
        exit(-1);
    }
    
    /* 2) Send initial success status to pipe */
    write(fdpipes[1],&buf,sizeof buf);

    /* 3) Read data from TTY */
    while (buf.count < count)
    {
        char databuf[1024];
        int retval;
        struct timeval tv;
        fd_set rfds;

        /* Setup for select with 3s timeout*/
        tv.tv_sec = 3;
        tv.tv_usec = 0;
        FD_ZERO(&rfds);
        FD_SET(fdtty, &rfds);

        /* Setup for select */
        if (0 > (retval = select(fdtty+1,&rfds,0,0,&tv)))
        {
            perror("recv_chars=>select(tty)");
            buf.m_errno = errno;
            buf.status = retval;
            break;
        }

        /* Select timed out */
        if (!retval)
	{
            perror("recv_chars=>select(tty)=>timeout");
            break;
        }

        /* Read data */
	++buf.reads;
        if (0 > (retval = read(fdtty,databuf, 1024)))
        {
            perror("recv_chars=>read(tty)");
            buf.m_errno = errno;
            buf.status = retval;
            break;
        }
        buf.count += retval;
    }

    /* 4) Send status to pipe */
    write(fdpipes[1],&buf,sizeof buf);

    /* 5) Clean up and exit */
    close(fdtty);
    close(fdpipes[1]);

    exit(0);
}
#endif/*__SST_H__*/
