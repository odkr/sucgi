/*
 * Test env_copy_var.
 *
 * Copyright 2023 Odin Kroeger.
 *
 * This file is part of suCGI.
 *
 * SuCGI is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * SuCGI is distributed in the hope that it will be useful, but WITHOUT
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
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../env.h"
#include "../macros.h"
#include "util/abort.h"
#include "util/types.h"


/*
 * Data types
 */

/* Mapping of a string to a return value. */
typedef struct {
    const bool set;
    const char *const key;
    const char *const value;
    const Error retval;
    int signal;
} EnvCopyVarArgs;


/*
 * Main
 */

int
main(void)
{
#if !defined(NDEBUG)

    /* RATS: ignore; used safely. */
    char hugename[MAX_VARNAME_LEN + 1U] = {0};
#endif

    /* RATS: ignore; used safely. */
    char longname[MAX_VARNAME_LEN] = {0};

    /* RATS: ignore; used safely. */
    char hugevar[MAX_VAR_LEN + 1] = {0};

    /* RATS: ignore; used safely. */
    char longvar[MAX_VAR_LEN] = {0};

    const EnvCopyVarArgs cases[] = {
        /* Invalid arguments. */
#if !defined(NDEBUG)
        {false, "", "n/a", OK, SIGABRT},
        {false, "=", "n/a", OK, SIGABRT},
        {false, "=foo", "n/a", OK, SIGABRT},
        {false, "foo=", "n/a", OK, SIGABRT},
        {false, "fo=o", "n/a", OK, SIGABRT},
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

    volatile int result = PASS;

#if !defined(NDEBUG)
    (void) memset(hugename, 'x', sizeof(hugename) - 1U);
#endif

    (void) memset(longname, 'x', sizeof(longname) - 1U);
    (void) memset(hugevar, 'x', sizeof(hugevar) - 1U);
    (void) memset(longvar, 'x', sizeof(longvar) - 1U);

    for (volatile size_t i = 0; i < NELEMS(cases); ++i) {
        const EnvCopyVarArgs args = cases[i];

        if (sigsetjmp(abort_env, 1) == 0) {
            /* RATS: ignore; used safely. */
            char value[sizeof(hugevar)];
            const char *var;
            size_t len;
            Error retval;

            errno = 0;
            if (args.set && setenv(args.key, args.value, true) != 0) {
                err(ERROR, "setenv %s=%s", args.key, args.value);
            }

            if (args.signal != 0) {
                warnx("the next test should fail an assertion.");
            }

            (void) abort_catch(err);
            retval = env_copy_var(args.key, (size_t) MAX_VAR_LEN - 1U,
                                  &len, value);
            (void) abort_reset(err);

            if (retval != args.retval) {
                result = FAIL;
                warnx("%zu: (%s, <maxlen>, → %zu, → %s) → %u [!]",
                      i, args.key, len, value, retval);
            }

            if (retval == OK) {
                if (strnlen(value, sizeof(value)) != len) {
                    result = FAIL;
                    warnx("%zu: (%s, <maxlen>, → %zu [!], → %s) → %u",
                          i, args.key, len, value, retval);
                }

                if (strncmp(value, args.value, sizeof(value)) != 0) {
                    result = FAIL;
                    warnx("%zu: (%s, <maxlen>, → %zu, → %s [!]) → %u",
                          i, args.key, len, value, retval);
                }
            }

            (void) memset(value, '\0', sizeof(value));

            /* RATS: ignore; environment is only used for testing. */
            var = getenv(args.key);

            if (var != NULL) {
                if (retval == OK &&
                    *args.value != '\0' &&
                    strncmp(var, "", sizeof("")) == 0)
                {
                    result = FAIL;
                    warnx("%zu: environment variable was changed", i);
                }
            } else {
                if (args.retval != ERR_SEARCH) {
                    result = FAIL;
                    warnx("%zu: $%s was set to NULL", i, args.key);
                }
            }
        }

        if (abort_signal != args.signal) {
            result = FAIL;
            warnx("%zu: (%s → <value>) ↑ %s [!]",
                  i, args.key, strsignal(abort_signal));
        }
    }

    return result;
}

