/*
 * Test privsuspend.
 *
 * Copyright 2022 and 2023 Odin Kroeger.
 *
 * This file is part of suCGI.
 *
 * suCGI is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * suCGI is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with suCGI. If not, see <https://www.gnu.org/licenses>.
 */

#define _BSD_SOURCE
#define _DARWIN_C_SOURCE
#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <sys/types.h>
#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include "../macros.h"
#include "../error.h"
#include "lib/util.h"


/*
 * Data types
 */

/* Combinations of effective and real process UIDs. */
typedef struct {
    const char *const message;
    const char *const arg;
} Args;


/*
 * Module variables
 */

/* Test cases. */
const Args cases[] = {
    {"message", "foo"},
    {"message", ""},
    {"%s", ""},
    {"foo %s", "bar"}
};


/*
 * Prototypes
 */

/*
 * Exit the programme with STATUS. But do not call exithandlers.
 *
 * GNU gcov uses exit handlers to write out coverage data, to the effect
 * that calling exit breaks collection of coverage data. However, error
 * calls exit, so, in order to allow for collection of coverage data,
 * the exit symbols is re-directed to point to _exit.
 */
void exit(int status);


/*
 * Functions
 */

void exit(int status)
{
    _exit(status);
}


/*
 * Main
 */

int
main (void) {
    volatile int result = TEST_PASSED;

    openlog("error", LOG_CONS | LOG_NDELAY | LOG_PERROR, LOG_AUTH);

    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];
        int pipefds[2];
        pid_t pid;

        errno = 0;
        if (pipe(pipefds) != 0) {
            err(TEST_ERROR, "pipe");
        }

        pid = fork();
        if (pid == 0) {
            int retval;

            do {
                errno = 0;
                retval = dup2(pipefds[1], fileno(stderr));
            } while (retval < 0 && errno == EINTR);

            if (retval < 0) {
                check_err(TEST_ERROR, "dup2");
            }

            for (size_t j = 0; j < NELEMS(pipefds); ++j) {
                int fd = pipefds[j];

                do {
                    errno = 0;
                    retval = close(fd);
                } while (retval < 0 && errno == EINTR);

                if (retval < 0) {
                    check_err(TEST_ERROR, "close");
                }
            }

            error(args.message, args.arg);

            /* Should be unreachable. */
            _exit(0);
        } else {
            char *buffer = NULL;
            char *message = NULL;
            ssize_t nchars;
            size_t msgsize;
            int msglen;
            int retval;
            int status;

            do {
                errno = 0;
                retval = close(pipefds[1]);
            } while (retval != 0 && errno == EINTR);

            if (retval != 0) {
                err(TEST_ERROR, "close");
            }

            for (size_t j = 0; j < INT_MAX; ++j) {
                char *tmp;

                errno = 0;
                tmp = realloc(buffer, BUFSIZ * (1UL << i));
                if (tmp == NULL) {
                    if (buffer != NULL) {
                        free(buffer);
                    }
                    err(TEST_ERROR, "realloc");
                }
                buffer = tmp;

                do {
                    errno = 0;
                    nchars = read(pipefds[0], buffer, BUFSIZ);
                } while (nchars < 0 && errno == EINTR);

                if (nchars == -1) {
                    err(TEST_ERROR, "read");
                } else if (nchars == 0) {
                    break;
                }
            }

/* Format string is args.message. */
#if defined(__GNUC__) && __GNUC__ >= 3
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif

            errno = 0;
            msglen = snprintf(NULL, 0U, args.message, args.arg);
            if (msglen < 0) {
                err(TEST_ERROR, "snprintf");
            }

            msgsize = (size_t) msglen + 1U;
            message = malloc((size_t) msgsize);
            if (message == NULL) {
                err(TEST_ERROR, "malloc");
            }

            (void) memset(message, '\0', msgsize);

            errno = 0;
            msglen = snprintf(message, msgsize, args.message, args.arg);
            if (msglen < 0) {
                err(TEST_ERROR, "snprintf");
            }

#if defined(__GNUC__) && __GNUC__ >= 3
#pragma GCC diagnostic pop
#endif

            if (strstr(buffer, message) == NULL) {
                result = TEST_FAILED;
                warnx("(%s, %s) ─→ <stderr> = %s [!]",
                      args.message, args.arg, buffer);
            }

            do {
                errno = 0;
                retval = waitpid(pid, &status, 0);
            } while (retval != 0 && errno == EINTR);

            if (retval < 0) {
                err(EXIT_FAILURE, "waitpid %d", pid);
            }

            if (WIFEXITED(status)) {
                int exitstatus;

                exitstatus = WEXITSTATUS(status);
                switch (exitstatus) {
                case EXIT_FAILURE:
                    break;
                case EXIT_SUCCESS:
                    result = TEST_FAILED;
                    break;
                default:
                    return TEST_ERROR;
                    break;
                }
            } else if (WIFSIGNALED(status)) {
                int signo;
                const char *signame;

                result = TEST_FAILED;
                signo = WTERMSIG(status);
                signame = strsignal(signo);
                warnx("(%s, %s) ↑ %s [!]", args.message, args.arg, signame);
            } else {
                errx(TEST_ERROR, "child %d exited abnormally", pid);
            }
        }
    }

    return result;
}
