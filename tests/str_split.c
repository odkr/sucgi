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

#include <assert.h>
#include <err.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../macros.h"
#include "../max.h"
#include "../str.h"
#include "lib.h"


/*
 * Constants
 */

/* Maximum length of dynamically created pre- and suffixes. */
#define SUB_STR_LEN 2U

/* Maximum length of dynamically created filenames. */
#define STR_LEN (SUB_STR_LEN * 3U + 1U)


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

/* A string just within limits. */
static char long_str[MAX_FNAME_LEN] = {'\0'};

/* A string w/o a delimiter that exceeds MAX_STR_LEN. */
static char huge_str[MAX_FNAME_LEN + 1U] = {'\0'};

/* A pair w/ a head that exceeds MAX_STR_LEN. */
static char huge_head[MAX_FNAME_LEN + 32U] = {'\0'};

/* Test cases. */
static const Args cases[] = {
    /* Overly long string. */
    {huge_str, ",", long_str, NULL, ERR_LEN},

    /* Overly long head. */
    {huge_head, ",", NULL, ",foo", ERR_LEN},

    /* Barely fitting string. */
    {long_str, ",", long_str, NULL, OK},

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
    fillstr('x', sizeof(long_str), long_str);
    fillstr('x', sizeof(huge_str), huge_str);
    fillstr('x', sizeof(huge_head), huge_head);

    (void) strncpy(&huge_head[MAX_FNAME_LEN], ",foo", 5);

    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];
        char head[MAX_FNAME_LEN];
        const char *tail;
        Error ret;

        (void) memset(head, '\0', MAX_FNAME_LEN);

        warnx("checking (%s, %s, %d, -> %s, -> %s) -> %u ...",
              args.str, args.sep, MAX_FNAME_LEN,
              args.head, args.tail, args.ret);

        ret = str_split(args.str, args.sep, MAX_FNAME_LEN, head, &tail);

        if (ret != args.ret) {
            errx(TEST_FAILED, "returned error %u", ret);
        }

        if (!(args.head == NULL ||
              strncmp(args.head, head, MAX_STR_LEN) == 0))
        {
            errx(TEST_FAILED, "returned head '%s'", head);
        }

        if (!(args.tail == tail ||
              strncmp(args.tail, tail, MAX_STR_LEN) == 0))
        {
            errx(TEST_FAILED, "returned tail '%s'", tail);
        }
    }

    warnx("all tests passed");
    return EXIT_SUCCESS;
}
