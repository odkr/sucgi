/*
 * Test str_split.
 *
 * Copyright 2022 and 2023 Odin Kroeger.
 *
 * This file is part of suCGI.
 *
 * suCGI is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * suCGI is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General
 * Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with suCGI. If not, see <https://www.gnu.org/licenses/>.
 */

#define _BSD_SOURCE
#define _DARWIN_C_SOURCE
#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <err.h>
#include <limits.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../macros.h"
#include "../params.h"
#include "../str.h"
#include "libutil/abort.h"
#include "libutil/str.h"
#include "libutil/types.h"


/*
 * Constants
 */

/* Arbitrary maximum string length for testing. */
#define MAX_LEN 32U

/* Number of characters to add to MAX_LEN to test overflow protection. */
#define EXTRA_LEN 8U


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
    int signal;
} StrSplitArgs;


/*
 * Main
 */

int
main(void)
{
#if !defined(NDEBUG)

    /* RATS: ignore; used safely. */
    char xhugestr[MAX_STR_LEN + 1U];
    str_fill(sizeof(xhugestr), xhugestr, 'x');
#endif

    /* RATS: ignore; used safely. */
    char xlongstr[MAX_STR_LEN];
    str_fill(sizeof(xlongstr), xlongstr, 'x');

    /* RATS: ignore; used safely. */
    char hugestr[MAX_LEN + 1U];
    str_fill(sizeof(hugestr), hugestr, 'x');

    /* RATS: ignore; used safely. */
    char longstr[MAX_LEN];
    str_fill(sizeof(longstr), longstr, 'x');

    /* RATS: ignore; used safely. */
    char hugehead[MAX_LEN + EXTRA_LEN];
    str_fill(sizeof(hugehead), hugehead, 'x');
    /* RATS: ignore; hugehead is null-terminated. */
    (void) strncpy(&hugehead[MAX_LEN], ",foo", 5U);

    const StrSplitArgs cases[] = {
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

    int volatile result = PASS;

    for (volatile size_t i = 0; i < NELEMS(cases); ++i) {
        const StrSplitArgs args = cases[i];

        if (sigsetjmp(abort_env, 1) == 0) {
            if (args.signal != 0) {
                warnx("the next test should fail an assertion.");
            }

            /* RATS: ignore; used safely. */
            char head[MAX_LEN];
            const char *tail = NULL;

            (void) abort_catch(err);
            const Error retval = str_split(args.str, args.sep, MAX_LEN,
                                           head, &tail);
            (void) abort_reset(err);

            if (retval != args.retval) {
                result = FAIL;
                warnx("(%s, %s, %u, → %s, → %s) → %u [!]",
                      args.str, args.sep, MAX_LEN, head, tail, retval);
            }

            if (retval == OK && strncmp(args.head, head, MAX_STR_LEN) != 0) {
                result = FAIL;
                warnx("(%s, %s, %u, → %s [!], → %s) → %u",
                      args.str, args.sep, MAX_LEN, head, tail, retval);
            }

            if (args.tail != NULL &&
                strncmp(args.tail, tail, MAX_STR_LEN) != 0)
            {
                result = FAIL;
                warnx("(%s, %s, %u, → %s, → %s [!]) → %u",
                      args.str, args.sep, MAX_LEN, head, tail, retval);
            }
        }

        if (abort_signal != args.signal) {
            result = FAIL;
            warnx("(%s, %s, %u, → <head>, → <tail>) ↑ %s [!]",
                  args.str, args.sep, MAX_LEN, strsignal(abort_signal));
        }
    }

    return result;
}
