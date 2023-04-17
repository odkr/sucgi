/*
 * Utility functions for tests.
 *
 * Copyright 2023 Odin Kroeger.
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

#include <err.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "../../macros.h"


/*
 * Global variables
 */

/* Is a test running? */
volatile sig_atomic_t checking = 0;

/* Environment to run tests in. */
sigjmp_buf checkenv;

/* Test result. */
int result = TEST_PASSED;


/*
 * Prototypes
 */

/*
 * If CHECKING > 0, jump to CHECKENV and return SIGNO.
 * Otherwise, restore the default handler and re-raise SIGNO.
 */
static void catch(int signo);


/*
 * Functions
 */

static void
catch(const int signo)
{
    static const struct sigaction defhdl = {
        .sa_handler = SIG_DFL
    };

    if (checking > 0) {
        checking = 0;
        siglongjmp(checkenv, signo);
    }

    psignal((unsigned int) signo, NULL);

    (void) sigaction(signo, &defhdl, NULL);
    (void) raise(signo);
}

void
checkinit(void)
{
    const int signos[] = {
        SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGQUIT,
        SIGSEGV, SIGSYS, SIGXCPU, SIGXFSZ
    };

    for (size_t i = 0; i < NELEMS(signos); ++i) {
        const struct sigaction action = {.sa_handler = catch};

        errno = 0;
        if (sigaction(signos[i], &action, NULL) != 0) {
            err(EXIT_FAILURE, "sigaction");
        }
    }
}

void
check_err(int eval, const char *fmt, ...)
{
    va_list argp;

    va_start(argp, fmt);
    vwarn(fmt, argp);
    va_end(argp);

    _exit(eval);
}

void
check_errx(int eval, const char *fmt, ...)
{
    va_list argp;

    va_start(argp, fmt);
    vwarnx(fmt, argp);
    va_end(argp);

    _exit(eval);
}
