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
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../env.h"
#include "../macros.h"
#include "check.h"


/*
 * Data types
 */

/* Mapping of a string to a return value. */
typedef struct {
    const bool set;
    const char *const key;
    const char *const value;
    const Error retval;
    int signo;
} Args;


/*
 * Module variables
 */

#if !defined(NDEBUG)
/* A variable name that exceeds MAX_VARNAME_LEN. */
static char hugename[MAX_VARNAME_LEN + 1U] = {'\0'};
#endif

/* A variable name just within MAX_VARNAME_LEN. */
static char longname[MAX_VARNAME_LEN] = {'\0'};

/* A value that exceeds MAX_VAR_LEN. */
static char hugevar[MAX_VAR_LEN + 1U] = {'\0'};

/* A value just within MAX_VAR_LEN. */
static char longvar[MAX_VAR_LEN] = {'\0'};

/* Static test cases. */
static const Args cases[] = {
    /* Invalid arguments. */
#if !defined(NDEBUG)
    {false, "", "n/a", OK, SIGABRT},
    {true, hugename, "n/a", OK, SIGABRT},
#endif

    /* Long name, but okay. */
    {true, longname, "foo", OK, 0},

    /* Simple tests. */
    {true, "foo", "bar", OK, 0},
    {false, "bar", "n/a", ERR_SEARCH, 0},

    /* Empty string shenanigans. */
    {true, "empty", "", OK, 0},

    /* Long values. */
    {true, "long", longvar, OK, 0},
    {true, "huge", hugevar, ERR_LEN, 0},

    /* UTF-8. */
    {true, "ḟ𝐨ò", "🄑ǠƦ", OK, 0},
    {false, "🄑ǠƦ", "n/a", ERR_SEARCH, 0}
};


/*
 * Main
 */

int
main (void)
{
    volatile int result = TEST_PASSED;

    checkinit();

#if !defined(NDEBUG)
    (void) memset(hugename, 'x', sizeof(hugename) - 1U);
#endif

    (void) memset(longname, 'x', sizeof(longname) - 1U);
    (void) memset(hugevar, 'x', sizeof(hugevar) - 1U);
    (void) memset(longvar, 'x', sizeof(longvar) - 1U);

    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];
        char value[sizeof(hugevar)];
        int jumpval;
        volatile Error retval;

        errno = 0;
        if (args.set && setenv(args.key, args.value, true) != 0) {
            err(TEST_ERROR, "setenv %s=%s", args.key, args.value);
        }

        jumpval = sigsetjmp(checkenv, true);

        if (jumpval == 0) {
            checking = 1;
            retval = envcopyvar(args.key, value);
            checking = 0;

            if (retval != args.retval) {
                warnx("(%s → %s) → %u [!]", args.key, value, retval);
                result = TEST_FAILED;
            }

            if (retval == OK &&
                strncmp(value, args.value, sizeof(value)) != 0)
            {
                warnx("(%s → %s [!]) → %u", args.key, value, retval);
                result = TEST_FAILED;
            }

            (void) memset(value, '\0', sizeof(value));

            if (retval == OK &&
                *args.value != '\0' &&
                strncmp(getenv(args.key), "", sizeof("")) == 0)
            {
                warnx("changing the copied value changed the environment.");
                result = TEST_FAILED;
            }
        }

        if (jumpval != args.signo) {
            warnx("(%s → %s) ↑ %s [!]",
                  args.key, value, strsignal(jumpval));
            result = TEST_FAILED;
        }
    }

    return result;
}

