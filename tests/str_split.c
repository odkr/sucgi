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
    char ascii[127];
    char sep[2];
    char **strs;
    size_t nstrs;

    fill_str('x', sizeof(long_str), long_str);
    fill_str('x', sizeof(huge_str), huge_str);
    fill_str('x', sizeof(huge_head), huge_head);
    fill_str('\0', sizeof(sep), sep);

    (void) strncpy(&huge_head[MAX_FNAME_LEN], ",foo", 5); /* RATS: ignore. */

    nstrs = (unsigned int) pow(sizeof(ascii), SUB_STR_LEN);
    assert(nstrs < UINT_MAX);

    strs = calloc(nstrs, sizeof(*strs));
    if(strs == NULL) {
        err(TEST_ERROR, "calloc");
    }

    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];
        char head[MAX_FNAME_LEN];    /* RATS: ignore */
        const char *tail;
        Error ret;

        (void) memset(head, '\0', MAX_FNAME_LEN);

        warnx("checking (%s, %s, %d, -> %s, -> %s) -> %u ...",
              args.str, args.sep, MAX_FNAME_LEN,
              args.head, args.tail, args.ret);

        ret = str_split(args.str, args.sep, MAX_FNAME_LEN, head, &tail);

        if (ret != args.ret) {
            errx(TEST_FAILED, "returned %u", ret);
        }
        if (!(args.head == NULL ||
              strncmp(args.head, head, MAX_STR_LEN) == 0))
        {
            errx(TEST_FAILED, "got head '%s'", head);
        }
        if (!(args.tail == tail ||
              strncmp(args.tail, tail, MAX_STR_LEN) == 0))
        {
            errx(TEST_FAILED, "got tail '%s'", tail);
        }
    }

    warnx("generating %zu strings ...", nstrs);
    for (size_t i = 0; i < nstrs; ++i) {
        int rc;

        strs[i] = calloc(SUB_STR_LEN + 1, sizeof(*strs[i]));
        if (strs[i] == NULL) {
            err(TEST_ERROR, "calloc");
        }

        rc = to_str((unsigned int) i, sizeof(ascii), ascii,
                    SUB_STR_LEN + 1, strs[i]);
        assert(rc == 0);
    }

    warnx("checking dynamically created strings ...");
    for (size_t i = 0; i < sizeof(ascii); ++i) {
        *sep = ascii[i];

        for (size_t j = 0; j < nstrs; ++j) {
            if (strchr(strs[j], ascii[i]) != NULL) {
                continue;
            }

            for (size_t k = 0; k < nstrs; ++k) {
                char joined[STR_LEN];
                char head[STR_LEN];
                const char *tail;
                int n;
                Error ret;

                n = snprintf(joined, sizeof(joined),
                             "%s%c%s", strs[j], ascii[i], strs[k]);
                if (n < 0) {
                    err(TEST_ERROR, "snprintf");
                }
                assert((size_t) n <= STR_LEN);

                ret = str_split(joined, sep, sizeof(head), head, &tail);

                if (ret != OK) {
                    errx(TEST_FAILED, "(%s, %c, %zu, -> %s, -> %s) -> %u!",
                         joined, ascii[i], sizeof(head), head, tail, ret);
                }
                if (strncmp(strs[j], head, STR_LEN) != 0) {
                    errx(TEST_FAILED, "(%s, %c, %zu, -> %s!, -> %s) -> %u",
                         joined, ascii[i], sizeof(head), head, tail, ret);
                }
                if (strncmp(strs[k], tail, STR_LEN) != 0) {
                    errx(TEST_FAILED, "(%s, %c, %zu, -> %s, -> %s!) -> %u",
                         joined, ascii[i], sizeof(head), head, tail, ret);
                }
            }
        }
    }

    warnx("all tests passed");
    return EXIT_SUCCESS;
}
