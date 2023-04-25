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

#include "../macros.h"
#include "../params.h"
#include "../pair.h"
#include "check.h"


/*
 * Data types
 */

/* Mapping of arguments to return values. */
typedef struct {
    const char *const key;
    const char *const value;
    const Error retval;
    int signo;
} Args;


/*
 * Module variables
 */

#if !defined(NDEBUG)
/* A key that that exceeds MAX_STR_LEN. */
static char hugekey[MAX_STR_LEN + 1U] = {0};
#endif

/* A key just within MAX_FNAME_LEN */
static char longkey[MAX_STR_LEN] = {0};

/* Test cases. */
static const Args cases[] = {
    /* Illegal argument. */
#if !defined(NDEBUG)
    {hugekey, "huge key", OK, SIGABRT},
#endif

    /* Simple tests. */
    {"foo", "bar", OK, 0},
    {"bar", "n/a", ERR_SEARCH, 0},

    /* Empty strings. */
    {"", "empty key", OK, 0},
    {"empty string", "", OK, 0},

    /* Maximum key length. */
    {longkey, "long key", OK, 0},

    /* Unicode shenanigans. */
    {"ⓕȱȱ", "Ḅḁᴿ", OK, 0},
    {"Ḅḁᴿ", "n/a", ERR_SEARCH, 0},
    {"", "empty key", OK, 0},
    {"èṃṗťÿ ŝțȓịñḡ", "", OK, 0}
};

/* Pairs for testing. */
static const Pair pairs[] = {
#if !defined (NDEBUG)
    {hugekey, "huge key"},
#endif
    {longkey, "long key"},
    {"", "empty key"},
    {"foo", "bar"},
    {"empty string", ""},
    {"ⓕȱȱ", "Ḅḁᴿ"},
    {"èṃṗťÿ ŝțȓịñḡ", ""}
};


/*
 * Main
 */

int
main(void)
{
    int volatile result = TEST_PASSED;

    checkinit();

#if !defined(NDEBUG)
    (void) memset(hugekey, 'x', sizeof(hugekey) - 1U);
#endif
    (void) memset(longkey, 'x', sizeof(longkey) - 1U);

    for (volatile size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];
        const char *value;
        int jumpval;
        volatile Error retval;

        jumpval = sigsetjmp(checkenv, true);

        if (jumpval == 0) {
            checking = 1;
            retval = pairfind(NELEMS(pairs), pairs, args.key, &value);
            checking = 0;

            if (args.retval != retval) {
                warnx("(<pairs>, %s, → %s) → %u [!]",
                      args.key, value, retval);
                result = TEST_FAILED;
            }

            if (retval == OK && strncmp(value, args.value, MAX_STR_LEN) != 0) {
                warnx("(<pairs>, %s, → %s [!]) → %u",
                      args.key, value, retval);
                result = TEST_FAILED;
            }
        }

        if (jumpval != args.signo) {
            warnx("(<pairs>, %s, → %s) ↑ %s [!]",
                  args.key, value, strsignal(jumpval));
            result = TEST_FAILED;
        }
    }

    return result;
}
