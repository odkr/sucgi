/*
 * Test handler_lookup.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../config.h"
#include "../macros.h"
#include "../max.h"
#include "../pair.h"
#include "result.h"


/*
 * Data types
 */

/* Mapping of arguments to return values. */
typedef struct {
    const char *const key;
    const char *const value;
    const Error ret;
} Args;


/*
 * Module variables
 */

/* A string just within MAX_LEN. */
static char longstr[MAX_STR_LEN] = {'\0'};

/* Test cases. */
static const Args cases[] = {
    /* Simple tests. */
    {"foo", "bar", OK},
    {"bar", "n/a", ERR_SEARCH},

    /* Empty strings. */
    {"", "empty string", OK},
    {"empty string", "", OK},

    /* Maximum string length. */
    {longstr, "long string", OK},

    /* Unicode shenanigans. */
    {"ⓕȱȱ", "Ḅḁᴿ", OK},
    {"Ḅḁᴿ", "n/a", ERR_SEARCH},
    {"", "empty string", OK},
    {"èṃṗťÿ ŝțȓịñḡ", "", OK}
};

/* Pairs for testing. */
static const Pair pairs[] = {
    {"", "empty string"},
    {"foo", "bar"},
    {"empty string", ""},
    {longstr, "long string"},
    {"ⓕȱȱ", "Ḅḁᴿ"},
    {"èṃṗťÿ ŝțȓịñḡ", ""}
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

    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];
        const char *value;
        Error ret;

        ret = pair_lookup(NELEMS(pairs), pairs, args.key, &value);

        if (args.ret != ret) {
            warnx("(<pairs>, %s, -> %s) -> %u [!]",
                  args.key, args.value, ret);
            result = TEST_FAILED;
        }

        if (ret == OK && strncmp(value, args.value, MAX_STR_LEN) != 0) {
            warnx("(<pairs>, %s, -> %s [!]) -> %u",
                  args.key, value, args.ret);
            result = TEST_FAILED;
        }
    }

    return result;
}
