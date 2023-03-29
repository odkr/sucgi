/*
 * Test str_split.
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
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../macros.h"
#include "../max.h"
#include "../str.h"
#include "result.h"


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
    const Error ret;
} Args;


/*
 * Module variables
 */

/* A string just within MAX_LEN. */
static char longstr[MAX_LEN] = {'\0'};

/* A string w/o a delimiter that exceeds MAX_LEN. */
static char hugestr[MAX_LEN + 1U] = {'\0'};

/* A pair w/ a head that exceeds MAX_LEN. */
static char hugehead[MAX_LEN + 32U] = {'\0'};

/* Test cases. */
static const Args cases[] = {
    /* Overly long string. */
    {hugestr, ",", longstr, NULL, ERR_LEN},

    /* Overly long head. */
    {hugehead, ",", NULL, ",foo", ERR_LEN},

    /* Barely fitting string. */
    {longstr, ",", longstr, NULL, OK},

    /* Simple test. */
    {"a,b", ",", "a", "b", OK},

    /* Empty strings. */
    {",b", ",", "", "b", OK},
    {"a,", ",", "a", "", OK},
    {"a,b", "", "a,b", NULL, OK},

    /* Environment-like tests. */
    {"foo=bar", "=", "foo", "bar", OK},
    {"foo=", "=", "foo", "", OK},
    {"foo==bar", "=", "foo", "=bar", OK},
    {"=bar", "=", "", "bar", OK},
    {"foo", "=", "foo", NULL, OK}
};


/*
 * Main
 */

int
main(void)
{
    int result = TEST_PASSED;

    (void) memset(longstr, 'x', sizeof(longstr));
    longstr[sizeof(longstr) - 1U] = '\0';

    (void) memset(hugestr, 'x', sizeof(hugestr));
    hugestr[sizeof(hugestr) - 1U] = '\0';

    (void) memset(hugehead, 'x', sizeof(hugehead));
    (void) strncpy(&hugehead[MAX_LEN], ",foo", 5U);

    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];
        char head[MAX_LEN];
        const char *tail;
        Error ret;

        (void) memset(head, '\0', MAX_LEN);

        ret = str_split(args.str, args.sep, MAX_LEN, head, &tail);

        if (ret != args.ret)
        {
            warnx("(%s, %s, %u, -> %s, -> %s) -> %u [!]",
                  args.str, args.sep, MAX_LEN,
                  args.head, args.tail, ret);
            result = TEST_FAILED;
        }

        if (!(args.head == NULL ||
              strncmp(args.head, head, MAX_STR_LEN) == 0))
        {
            warnx("(%s, %s, %u, -> %s [!], -> %s) -> %u",
                  args.str, args.sep, MAX_LEN,
                  head, args.tail, args.ret);
            result = TEST_FAILED;
        }

        if (!(args.tail == tail ||
              strncmp(args.tail, tail, MAX_STR_LEN) == 0))
        {
            warnx("(%s, %s, %u, -> %s, -> %s [!]) -> %u",
                  args.str, args.sep, MAX_LEN,
                  args.head, tail, args.ret);
            result = TEST_FAILED;
        }
    }

    return result;
}
