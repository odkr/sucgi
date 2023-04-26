/*
 * Test handlerfind.
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

#include <err.h>
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../config.h"
#include "../handler.h"
#include "../macros.h"
#include "../params.h"
#include "lib/check.h"


/*
 * Data types
 */

/* Mapping of arguments to return values. */
typedef struct {
    const char *const script;
    const char *const handler;
    const Error retval;
    int signo;
} Args;


/*
 * Module variables
 */

#if !defined(NDEBUG)
/* A filename that that exceeds MAX_FNAME_LEN. */
static char hugefname[MAX_FNAME_LEN + 1U] = {0};
#endif

/* A filename just within MAX_FNAME_LEN */
static char longfname[MAX_FNAME_LEN] = {0};

/* Test cases. */
static const Args cases[] = {
#if !defined(NDEBUG)
    /* Illegal arguments. */
    {hugefname, "<n/a>", OK, SIGABRT},
    {"", "<n/a>", OK, SIGABRT},
#endif

    /* Simple errors. */
    {"file", "<n/a>", ERR_SUFFIX, 0},
    {".", "<n/a>", ERR_SUFFIX, 0},
    {".sh", "<n/a>", ERR_SUFFIX, 0},
    {".py", "<n/a>", ERR_SUFFIX, 0},
    {"file.null", "<n/a>", ERR_BAD, 0},
    {"file.empty", "<n/a>", ERR_BAD, 0},
    {"file.py", "<n/a>", ERR_SEARCH, 0},
    {"file.post", "<n/a>", ERR_SEARCH, 0},
    {"long.suffix-0123456789abcdef", "<n/a>", ERR_LEN, 0},

    /* Empty string shenanigans. */
    {" ", "<n/a>", ERR_SUFFIX, 0},
    {". ", "<n/a>", ERR_SUFFIX, 0},
    {".sh ", "<n/a>", ERR_SUFFIX, 0},
    {".py ", "<n/a>", ERR_SUFFIX, 0},
    {" .null", "<n/a>", ERR_BAD, 0},
    {" .empty", "<n/a>", ERR_BAD, 0},
    {" .py", "<n/a>", ERR_SEARCH, 0},
    {" .post", "<n/a>", ERR_SEARCH, 0},
    {" . ", "<n/a>", ERR_SEARCH, 0},

    /* Unicode shenanigans. */
    {"𝕗ïḻę", "<n/a>", ERR_SUFFIX, 0},
    {".", "<n/a>", ERR_SUFFIX, 0},
    {".sh", "<n/a>", ERR_SUFFIX, 0},
    {".py", "<n/a>", ERR_SUFFIX, 0},
    {"𝕗ïḻę.null", "<n/a>", ERR_BAD, 0},
    {"𝕗ïḻę.empty", "<n/a>", ERR_BAD, 0},
    {"𝕗ïḻę.py", "<n/a>", ERR_SEARCH, 0},
    {"𝕗ïḻę.post", "<n/a>", ERR_SEARCH, 0},
    {"𝕗ïḻę.suffix-0123456789abcdef", "<n/a>", ERR_LEN, 0},

    /* Simple tests. */
    {longfname, "<n/a>", ERR_SUFFIX, 0},
    {"file.sh", "sh", OK, 0},
    {"file.", "dot", OK, 0}
};

/* Filename suffix-handler database for testing. */
static const Pair db[] = {
    {"", "unreachable"},
    {".", "dot"},
    {".sh", "sh"},
    {".null", NULL},
    {".empty", ""},
    {".pre", "pre"},
    {".suffix-0123456789abcdef", "<unreachable>"}
};


/*
 * Main
 */

int
main(void)
{
    volatile int result = TEST_PASSED;

    if (checkinit() != 0) {
        err(TEST_ERROR, "sigaction");
    }

#if !defined(NDEBUG)
    (void) memset(hugefname, 'x', sizeof(hugefname) - 1U);
#endif
    (void) memset(longfname, 'x', sizeof(longfname) - 1U);

    for (volatile size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];
        const char *handler;
        int jumpval;
        volatile Error retval;

        jumpval = sigsetjmp(checkenv, true);

        if (jumpval == 0) {
            checking = 1;
            retval = handlerfind(NELEMS(db), db, args.script, &handler);
            checking = 0;

            if (args.retval != retval) {
                warnx("(<db>, %s, → %s) → %u [!]",
                      args.script, handler, retval);
                result = TEST_FAILED;
            }

            if (retval == OK &&
                strncmp(handler, args.handler, MAX_STR_LEN) != 0)
            {
                result = TEST_FAILED;
                warnx("(<db>, %s, → %s [!]) → %u",
                      args.script, handler, retval);
            }
        }

        if (jumpval != args.signo) {
            result = TEST_FAILED;
            warnx("(<db>, %s, → %s) ↑ %s [!]",
                  args.script, handler, strsignal(jumpval));
        }
    }

    return result;
}
