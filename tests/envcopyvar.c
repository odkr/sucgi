/*
 * Test envcopyvar.
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
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../env.h"
#include "../macros.h"
#include "result.h"


/*
 * Data types
 */

/* Mapping of a string to a return value. */
typedef struct {
    const bool set;
    const char *const key;
    const char *const value;
    const Error ret;
} Args;


/*
 * Module variables
 */

/* A value just within MAX_VAR_LEN. */
static char longstr[MAX_VAR_LEN] = {'\0'};

/* A value that exceeds MAX_VAR_LEN. */
static char hugestr[MAX_VAR_LEN + 1U] = {'\0'};

/* Static test cases. */
static const Args cases[] = {
    /* Simple tests. */
    {true, "foo", "bar", OK},
    {false, "bar", "n/a", ERR_SEARCH},

    /* Empty string shenanigans. */
    {true, "empty", "", OK},

    /* Long values. */
    {true, "long", longstr, OK},
    {true, "huge", hugestr, ERR_LEN},

    /* UTF-8. */
    {true, "ḟ𝐨ò", "🄑ǠƦ", OK},
    {false, "🄑ǠƦ", "n/a", ERR_SEARCH}
};


/*
 * Main
 */

int
main (void)
{
    int result = TEST_PASSED;

    (void) memset(longstr, 'x', sizeof(longstr));
    longstr[sizeof(longstr) - 1U] = '\0';

    (void) memset(hugestr, 'x', sizeof(hugestr));
    hugestr[sizeof(hugestr) - 1U] = '\0';

    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];
        char value[sizeof(hugestr)];
        Error ret;

        errno = 0;
        if (args.set && setenv(args.key, args.value, true) != 0) {
            err(TEST_ERROR, "setenv %s=%s", args.key, args.value);
        }

        ret = envcopyvar(args.key, value);

        if (ret != args.ret) {
            warnx("(%s -> %s) -> %u [!]", args.key, args.value, ret);
            result = TEST_FAILED;
        }

        if (ret == OK && strncmp(value, args.value, sizeof(value)) != 0) {
            warnx("(%s -> %s [!]) -> %u", args.key, value, args.ret);
            result = TEST_FAILED;
        }

        (void) memset(value, '\0', sizeof(value));

        if (ret == OK &&
            *args.value != '\0' &&
            strncmp(getenv(args.key), "", sizeof("")) == 0)
        {
            warnx("changing the copied value changed the environment.");
            result = TEST_FAILED;
        }
    }

    return result;
}

