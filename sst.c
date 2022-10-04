/* Program to write some number or characters as raw stream to
 * tty file
 */
#include <errno.h>
#include <stdlib.h>

/* Most logic is in one of these header files as static routines */
#include "raw_settings.h"
#include "sst.h"

int
main(int argc, char** argv)
{
    int iarg;
    char* arg;
    char* tty_name = NULL;
    int do_raw_config = 0;
    size_t send_count;
    int debug = 0;
    int o_nonblock = 0;
    size_t tries;
    size_t eagains;
    int fork_reader = 0;

    /* Handle input arguments */
    for (iarg=1; iarg<argc; ++iarg)
    {
        arg = argv[iarg];

        /* Dump raw settings string from raw_settings.h
         * --dump-raw-settings          -> write to STDOUT
         * --dump-raw-settings=xyz.txt  -> write to file
         */
        if (!strcmp(arg,"--dump-raw-settings")
           || !strncmp(arg,"--dump-raw-settings=", 20)
           )
        {
            FILE* f = arg[19] ? fopen(arg+20,"w") : stdout;
            if (!f) { perror(arg); continue; }
            dump_raw_settings(f);
            fclose(f);
        }

        /* Dump [to_send] array from sst.h
         * --dump-to-send               -> write to STDOUT
         * --dump-to-send=xyz.txt       -> write to file
         */
        else if (!strcmp(arg,"--dump-to-send")
           || !strncmp(arg,"--dump-to-send=", 15)
           )
        {
            FILE* f = arg[14] ? fopen(arg+15,"wb") : stdout;
            if (!f) { perror(arg); continue; }
            dump_to_send(f);
            fclose(f);
        }

        /* Name of typical TTY device in filesystem to which to write
         * --tty=/dev/tty*
         * E.g. --tty=/dev/ttyTHS0 or --tty=/dev/ttyUSB0
         * N.B. default tty_name is a null pointer, which will inhibit
         *      any configuration or output
         */
        else if (!strncmp(arg,"--tty=/dev/tty", 14))
        {
            tty_name = arg + 6;
        }

        /* Name of non-typical TTY device (or file) to which to write
         * E.g. --non-standard-tty=test_data.txt
         */
        else if (!strncmp(arg,"--non-standard-tty=", 19))
        {
            tty_name = arg + 19;
        }

        /* How many characters to send
         * --send-count=921600
         */
        else if (!strncmp(arg,"--send-count=", 13))
        {
            unsigned long ct;
            if (1 != sscanf(arg+13,"%lu",&ct))
            {
                fprintf(stderr,"ERROR:  bad count [%s]\n", arg);
                continue;
            }
            send_count = ct;
        }

        /* Debugging (logging) for this routine
         * --debug
         */
        else if (!strcmp(arg,"--debug"))
        {
            debug = -1;
        }

        /* Debugging (logging) for raw_settings.h
         * --raw-config-debug
         */
        else if (!strcmp(arg,"--raw-config-debug"))
        {
            raw_settings_debug = -1;
        }

        /* Configure the tty to pass raw data
         * --do-raw-config
         * N.B. Default is to not do raw configuration
         */
        else if (!strcmp(arg,"--do-raw-config"))
        {
            do_raw_config = 1;
        }

        /* Open tty non-blocking
         * --open-non-blocking
         * N.B. Default is to open for blocking
         */
        else if (!strcmp(arg,"--open-non-blocking"))
        {
            o_nonblock = O_NONBLOCK;
        }

        /* Fork a reader for the data
         * --fork-reader
         * N.B. Default is to not fork a reader
         */
        else if (!strcmp(arg,"--fork-reader"))
        {
            fork_reader = 1;
        }

        else
        {
           fprintf(stderr, "Unknown option:  [%s]\n", arg);
           return 3;
        }
    }

    if (tty_name && do_raw_config)
    {
        /* Configure TTY for raw data */
        if (!stty_raw_config(tty_name, (char*)NULL) && debug)
        {
            fprintf(stderr, "SUCCESS:  raw config of [%s]\n", tty_name);
        }
    };

    if (tty_name && send_count > 0)
    {
    SEQUENCE8BIT s8;   /* used by send_chars below (cf. stty.h) */
    int fdrdr = 0;

        /* Write test array data (see sst.h) to TTY (or file) */
        ssize_t sc;

        /* Open for write and non-block if that option was supplied */
        int fd = open(tty_name, O_WRONLY | o_nonblock);

        /* Try again if file not in filesystem i.e. create new file */
        if (0>fd && ENOENT==errno)
        {
            errno = 0;
            fd = open(tty_name, O_WRONLY  | o_nonblock | O_CREAT);
        }
        if (debug) { fprintf(stderr,"Opened [%s]; fd=%d\n", tty_name, fd); }
        if (0>fd) { perror(tty_name); }

        /* Fork reader of these data, if requested (--fork-reader) */
        fdrdr = fork_reader ? recv_chars(tty_name, send_count) : 0;
        if (0 > fdrdr) { return -1; }

        /* Write test data */
        sc = send_chars(fd, send_count, &s8, &tries, &eagains);

        if (debug) {
            fprintf(stderr,"Wrote %ld chars to [%s]; fd=%d"
                           "; tries=%lu; EAGAINs=%lu\n"
                          , (long)sc, tty_name, fd, tries, eagains
                          );
        }

        if (fork_reader)
        {
            RECVSTATUS buf;
            if ((sizeof buf) != read(fdrdr,&buf,sizeof buf))
            {
                perror("Retrieving reader result from pipe"); 
                close(fdrdr);
                return -1;
            }
            close(fdrdr);

            if (debug) {
                fprintf(stderr,"Read %lu chars from [%s]; fd=%d"
                               "; status=%d; errno=%d\n"
                              , buf.count, tty_name, fdrdr
                              , (int)buf.status, buf.m_errno
                              );
            }
        }

        close(fd);
    }

    return 0;
}
