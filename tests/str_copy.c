/*
 * Test str_copy.
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

#define _XOPEN_SOURCE 700

#include <err.h>
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../macros.h"
#include "../params.h"
#include "../str.h"
#include "util/abort.h"
#include "util/types.h"


/*
 * Data types
 */

/* Mapping of arguments to return values. */
typedef struct {
    const size_t n;
    const char *const src;
    const char *const dest;
    const Error retval;
    const int signal;
} StrCopyArgs;


/*
 * Main
 */

int
main(void)
{
#if !defined(NDEBUG)

    /* RATS: ignore; used safely. */
    char hugestr[MAX_STR_LEN + 1U] = {0};
#endif

    /* RATS: ignore; used safely. */
    char longstr[MAX_STR_LEN] = {0};

    const StrCopyArgs cases[] = {
#if !defined(NDEBUG)
        /* Invalid argument. */
        {1, hugestr, "n/a", OK, SIGABRT},
#endif

        /* Long string. */
        {MAX_STR_LEN - 1U, longstr, longstr, OK, 0},

        /* Simple test. */
        {MAX_STR_LEN - 1U, "foo", "foo", OK, 0},


        /* Almost truncated. */
        {1, "x", "x", OK, 0},

        /* Truncation. */
        {3, "abcd", "abc", ERR_LEN, 0},

        /* Truncate to 0. */
        {0, "foo", "", ERR_LEN, 0},

        /* Empty strings. */
        {MAX_STR_LEN - 1U, "", "", OK, 0},
        {1, "", "", OK, 0},
        {0, "", "", OK, 0}
    };

    volatile int result = PASS;

#if !defined(NDEBUG)
    (void) memset(hugestr, 'x', sizeof(hugestr) - 1U);
#endif
    (void) memset(longstr, 'x', sizeof(longstr) - 1U);

    for (volatile size_t i = 0; i < NELEMS(cases); ++i) {
        const StrCopyArgs args = cases[i];

        if (sigsetjmp(abort_env, 1) == 0) {
            /* RATS: ignore; used safely. */
            char dest[MAX_STR_LEN];
            size_t destlen;
            Error retval;

            (void) memset(dest, '\0', sizeof(dest));

            if (args.signal != 0) {
                warnx("the next test should fail an assertion.");
            }

            (void) abort_catch(err);
            retval = str_copy(args.n, args.src, &destlen, dest);
            (void) abort_reset(err);

            if (retval != args.retval) {
                result = FAIL;
                warnx("%zu: (%zu, %s, → %zu, → %s) → %u [!]",
                      i, args.n, args.src, destlen, dest, retval);
            }

            if (retval == OK) {
                if (strnlen(dest, MAX_STR_LEN) != destlen) {
                    result = FAIL;
                    warnx("%zu: (%zu, %s, → %zu [!], → %s) → %u",
                          i, args.n, args.src, destlen, dest, retval);
                }

                if (strncmp(args.dest, dest, MAX_STR_LEN) != 0) {
                    result = FAIL;
                    warnx("%zu: (%zu, %s, → %zu, → %s [!]) → %u",
                          i, args.n, args.src, destlen, dest, retval);
                }
            }
        }

        if (abort_signal != args.signal) {
            result = FAIL;
            warnx("%zu: (%zu, %s, → <destlen>, → <dest>) ↑ %s [!]",
                  i, args.n, args.src, strsignal(abort_signal));
        }
    }

    return result;
}
