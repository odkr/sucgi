/*
 * Test env_restore.
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

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <float.h>
#include <math.h>
#include <regex.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../env.h"
#include "../macros.h"
#include "../params.h"
#include "../str.h"
#include "libutil/abort.h"
#include "libutil/array.h"
#include "libutil/str.h"
#include "libutil/types.h"


/*
 * Constants
 */

/* Maximum number of environment variables for testing. */
#define MAX_TEST_NVARS (MAX_NVARS + 1U)

/* Maximum string length for testing. */
#define MAX_TEST_STR_LEN (MAX_STR_LEN + 1U)

/* Format for creating environment variables. */
#define VAR_FORMAT "var%0*d= "


/*
 * Data types
 */

/* Mapping of constant inputs to constant outputs. */
typedef struct {
    const char *const vars;         /* Variables to set. */
    const char *const *env;         /* Resulting environmnet. */
    Error retval;                   /* Return value. */
    int signal;                     /* Signal caught, if any. */
} EnvSetNArgs;


/*
 * Main
 */


int
main(void)
{
    assert(MAX_NVARS > 0U);
    assert((uintmax_t) MAX_NVARS < (uintmax_t) INT_MAX);
    assert(MAX_NVARS < DBL_MAX);
    const int maxndigits = (int) (log10(MAX_NVARS) + 1);

    /* RATS: ignore; no string expansion. */
    const int varlen = snprintf(NULL, 0, VAR_FORMAT, maxndigits, (int) MAX_NVARS);
    if (varlen < 1) {
        err(ERROR, "snprintf");
    }

    assert((uintmax_t) varlen < (uintmax_t) SIZE_MAX);
    const size_t strsize = ((size_t) MAX_NVARS + 1U) * (size_t) varlen + 1U;
    /* cppcheck-suppress misra-c2012-11.5; bad advice for malloc. */
    char *const hugenvars = malloc(strsize);
    if (hugenvars == NULL) {
        err(ERROR, "malloc");
    }

    const size_t varsize = (size_t) varlen + 1U;
    for (int i = 0; (unsigned) i <= MAX_NVARS; ++i) {
        assert(i < PTRDIFF_MAX / varlen);
        ptrdiff_t startidx = (ptrdiff_t) i * varlen;
        assert(startidx >= 0);

        errno = 0;
        /* RATS: ignore; format is a literal and expansion is bounded. */
        const int nbytes = snprintf(&hugenvars[startidx], (size_t) varsize,
                                    VAR_FORMAT, maxndigits, i);
        if (nbytes < 1) {
            err(ERROR, "snprintf");
        }
    }

/* strncpy is intended to truncate those strings. */
#if defined(__GNUC__) && __GNUC__ >= 8
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif

    /* RATS: ignore; used safely. */
    char long_var[MAX_VAR_LEN];
    str_fill(sizeof(long_var), long_var, 'x');
    /* RATS: ignore; the buffer is large enough. */
    (void) strncpy(long_var, "var=", 4);

    /* RATS: ignore; used safely. */
    char huge_var[MAX_VAR_LEN + 1];
    str_fill(sizeof(huge_var), huge_var, 'x');
    /* RATS: ignore; the buffer is large enough. */
    (void) strncpy(huge_var, "var=", 4);

#if defined(__GNUC__) && __GNUC__ >= 8
#pragma GCC diagnostic pop
#endif

    /* RATS: ignore; used safely. */
    char long_name[MAX_VAR_LEN];
    str_fill(sizeof(long_name), long_name, 'x');
    long_name[MAX_VARNAME_LEN - 1U] = '=';

    /* RATS: ignore; used safely. */
    char huge_name[MAX_VAR_LEN];
    str_fill(sizeof(huge_name), huge_name, 'x');
    huge_name[MAX_VARNAME_LEN] = '=';

    const EnvSetNArgs cases[] = {
        /* Null tests. */
        {"", (const char *const []) {NULL}, OK, 0},

        /* Too many variables. */
        {hugenvars, (const char *const []) {NULL}, ERR_LEN, 0},

        /* Long but legal variable. */
        {long_var, (const char *const []) {long_var, NULL}, OK, 0},

        /* Long but legal variable name. */
        {long_name, (const char *const []) {long_name, NULL}, OK, 0},

        /* Variable that is too long. */
        {huge_var, (const char *const []) {NULL}, ERR_BAD, 0},

        /* Variable with a name that is too long.  */
        {huge_name, (const char *const []) {NULL}, ERR_BAD, 0},

        /* Simple tests. */
        {"foo=foo", (const char *const []) {"foo=foo", NULL}, OK, 0},
        {"foo=foo bar=bar", (const char *const []) {"foo=foo", "bar=bar", NULL}, OK, 0},
        {"foo", (const char *const []) {NULL}, ERR_BAD, 0},
        {"=foo", (const char *const []) {NULL}, ERR_BAD, 0},

        /* Illegal names. */
        {" foo=foo", (const char *const []) {NULL}, ERR_BAD, 0},
        {"0foo=foo", (const char *const []) {NULL}, ERR_BAD, 0},
        {"*=foo", (const char *const []) {NULL}, ERR_BAD, 0},
        {"foo =foo", (const char *const []) {NULL}, ERR_BAD, 0},
        {"$(foo)=foo", (const char *const []) {NULL}, ERR_BAD, 0},
        {"`foo`=foo", (const char *const []) {NULL}, ERR_BAD, 0},

        /* Odd but legal values. */
        {
            "SSL_CLIENT_S_DN_C_0= SSL_CLIENT_S_DN_C_1==",
            (const char *const []) {
                "SSL_CLIENT_S_DN_C_0=",
                "SSL_CLIENT_S_DN_C_1==",
                NULL
            },
            OK,
            0
        },

        /* Real-world tests. */
        {
            "LANG=en_GB.UTF-8 IFS= SHELL=/bin/sh",
            (const char *const []) {
                "LANG=en_GB.UTF-8",
                "IFS=",
                "SHELL=/bin/sh",
                NULL
            },
            OK,
            0
        },
    };

    volatile int result = PASS;
    char *null = NULL;

    /* Run tests. */
    for (volatile size_t i = 0; i < NELEMS(cases); ++i) {
        const EnvSetNArgs args = cases[i];

        if (sigsetjmp(abort_env, 1) == 0) {
            environ = &null;

            (void) abort_catch(err);
            const Error retval = env_setn(args.vars);
            (void) abort_reset(err);

            if (retval != args.retval) {
                result = FAIL;
                warnx("(%s) → %u [!]", args.vars, retval);
            }

            if (retval == OK) {
                volatile size_t nexpected = 0;
                while (args.env[nexpected] != NULL) {
                    assert(nexpected < SIZE_MAX);
                    ++nexpected;
                }

                volatile size_t nactual = 0;
                while (environ[nactual] != NULL) {
                    assert(nexpected < SIZE_MAX);
                    ++nactual;
                }

                if (
                    !array_equals(
                        args.env, nexpected, sizeof(*args.env),
                        environ, nactual, sizeof(*environ),
                        (CompFn) str_cmp_ptrs
                    )
                ) {
                    result = FAIL;
                    warnx("(%s) ─→ <environ> [!]", args.vars);
                }
            }
        }

        if (abort_signal != args.signal) {
            result = FAIL;
            warnx("(%s) ↑ %s [!]", args.vars, strerror(abort_signal));
        }
    }

    return result;
}
