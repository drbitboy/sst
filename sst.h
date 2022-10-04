#ifndef __SST_H__
#define __SST_H__

/**********************************************************************/
/*** Routines to send a pre-defined stream of binary                ***/
/*** 8-bit characters to a TTY (or other file)                      ***/
/**********************************************************************/

#include <errno.h>
#include <stdio.h>

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
#endif/*__SST_H__*/
