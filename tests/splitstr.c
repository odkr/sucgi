/*
 * Test splitstr.
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
#include <limits.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../macros.h"
#include "../params.h"
#include "../str.h"
#include "util/check.h"


/*
 * Constants
 */

/* Arbitrary maximum string length for testing. */
#define MAX_LEN 32U


/*
 * Data types
 */

/* Mapping of arguments to return values. */
typedef struct {
    const char *const str;
    const char *const sep;
    const char *const head;
    const char *const tail;
    const Error retval;
    int signo;
} Args;


/*
 * Module variables
 */

/* A string w/o a delimiter that exceeds MAX_STR_LEN. */
#if !defined(NDEBUG)
static char xhugestr[MAX_STR_LEN + 1U] = {'\0'};
#endif

/* A string just within MAX_STR_LEN. */
static char xlongstr[MAX_STR_LEN] = {'\0'};

/* A string w/o a delimiter that exceeds MAX_LEN. */
static char hugestr[MAX_LEN + 1U] = {'\0'};

/* A string just within MAX_LEN. */
static char longstr[MAX_LEN] = {'\0'};

/* A head that exceeds MAX_LEN. */
static char hugehead[MAX_LEN + 32U] = {'\0'};

/* Test cases. */
static const Args cases[] = {
#if !defined(NDEBUG)
    /* Illegal arguments. */
    {xhugestr, ",", "<n/a>", NULL, ERR_LEN, SIGABRT},
#endif

    /* Overly long string. */
    {xlongstr, ",", "<n/a>", NULL, ERR_LEN, 0},
    {hugestr, ",", "<n/a>", NULL, ERR_LEN, 0},

    /* Overly long head. */
    {hugehead, ",", "<n/a>", ",foo", ERR_LEN, 0},

    /* Barely fitting string. */
    {longstr, ",", longstr, NULL, OK, 0},

    /* Simple test. */
    {"a,b", ",", "a", "b", OK, 0},

    /* Empty strings. */
    {",b", ",", "", "b", OK, 0},
    {"a,", ",", "a", "", OK, 0},
    {"a,b", "", "a,b", NULL, OK, 0},

    /* Environment-like tests. */
    {"foo=bar", "=", "foo", "bar", OK, 0},
    {"foo=", "=", "foo", "", OK, 0},
    {"foo==bar", "=", "foo", "=bar", OK, 0},
    {"=bar", "=", "", "bar", OK, 0},
    {"foo", "=", "foo", NULL, OK, 0}
};


/*
 * Main
 */

int
main(void)
{
    int volatile result = TEST_PASSED;

    if (checkinit() != 0) {
        err(TEST_ERROR, "sigaction");
    }

#if !defined(NDEBUG)
    (void) memset(xhugestr, 'x', sizeof(xhugestr) - 1U);
#endif
    (void) memset(xlongstr, 'x', sizeof(xlongstr) - 1U);
    (void) memset(hugestr, 'x', sizeof(hugestr) - 1U);
    (void) memset(longstr, 'x', sizeof(longstr) - 1U);
    (void) memset(hugehead, 'x', sizeof(hugehead) - 1U);
    (void) strncpy(&hugehead[MAX_LEN], ",foo", 5U);

    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];
        char head[MAX_LEN];
        const char *tail;
        int jumpval;
        Error retval;

        (void) memset(head, '\0', MAX_LEN);

        jumpval = sigsetjmp(checkenv, true);

        if (jumpval == 0) {
            checking = 1;
            retval = splitstr(args.str, args.sep, MAX_LEN, head, &tail);
            checking = 0;

            if (retval != args.retval) {
                result = TEST_FAILED;
                warnx("(%s, %s, %u, → %s, → %s) → %u [!]",
                      args.str, args.sep, MAX_LEN, head, tail, retval);
            }

            if (retval == OK && strncmp(args.head, head, MAX_STR_LEN) != 0) {
                result = TEST_FAILED;
                warnx("(%s, %s, %u, → %s [!], → %s) → %u",
                      args.str, args.sep, MAX_LEN, head, tail, retval);
            }

            if (args.tail != NULL &&
                strncmp(args.tail, tail, MAX_STR_LEN) != 0)
            {
                result = TEST_FAILED;
                warnx("(%s, %s, %u, → %s, → %s [!]) → %u",
                      args.str, args.sep, MAX_LEN, head, tail, retval);
            }
        }

        if (jumpval != args.signo) {
            result = TEST_FAILED;
            warnx("(%s, %s, %u, → %s, → %s) ↑ %s [!]",
                  args.str, args.sep, MAX_LEN, head, tail, strsignal(jumpval));
        }
    }

    return result;
}
