/*
 * Test copystr.
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
#include <stdlib.h>
#include <string.h>

#include "../macros.h"
#include "../max.h"
#include "../str.h"
#include "check.h"


/*
 * Data types
 */

/* Mapping of arguments to return values. */
typedef struct {
    const size_t n;
    const char *const src;
    const char *const dest;
    const Error retval;
    const int signo;
} Args;


/*
 * Module variables
 */

#if !defined(NDEBUG)
/* A string that that exceeds MAX_STR_LEN. */
static char hugestr[MAX_STR_LEN + 1U] = {0};
#endif

/* A string just within MAX_STR_LEN */
static char longstr[MAX_STR_LEN] = {0};

/* Tests. */
static const Args cases[] = {
#if !defined(NDEBUG)
    /* Invalid argument. */
    {1, hugestr, "n/a", OK, SIGABRT},
#endif

    /* Long string. */
    {MAX_STR_LEN - 1U, longstr, longstr, OK, 0},

    /* Simple test. */
    {MAX_STR_LEN - 1U, "foo", "foo", OK, 0},


    /* Almost truncated. */
    {1, "x", "x", OK, 0},

    /* Truncation. */
    {3, "abcd", "abc", ERR_LEN, 0},

    /* Truncate to 0. */
    {0, "foo", "", ERR_LEN, 0},

    /* Empty strings. */
    {MAX_STR_LEN - 1U, "", "", OK, 0},
    {1, "", "", OK, 0},
    {0, "", "", OK, 0}
};


/*
 * Main
 */

int
main (void) {
    volatile int result = TEST_PASSED;

    checkinit();

#if !defined(NDEBUG)
    (void) memset(hugestr, 'x', sizeof(hugestr) - 1U);
#endif
    (void) memset(longstr, 'x', sizeof(longstr) - 1U);

    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];
        char dest[MAX_STR_LEN];
        int jumpval;
        volatile Error retval;

        (void) memset(dest, '\0', sizeof(dest));

        jumpval = sigsetjmp(checkenv, true);

        if (jumpval == 0) {
            checking = 1;
            retval = copystr(args.n, args.src, dest);
            checking = 0;

            if (retval != args.retval) {
                result = TEST_FAILED;
                warnx("(%zu, %s, → %s) → %u [!]",
                      args.n, args.src, dest, retval);
            }

            if (retval == OK && strncmp(args.dest, dest, MAX_STR_LEN) != 0) {
                result = TEST_FAILED;
                warnx("(%zu, %s, → %s [!]) → %u",
                      args.n, args.src, dest, retval);
            }
        }

        if (jumpval != args.signo) {
            result = TEST_FAILED;
            warnx("(%zu, %s, → %s) ↑ %s [!]",
                  args.n, args.src, dest, strsignal(jumpval));
        }
    }

    return result;
}
