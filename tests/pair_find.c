/*
 * Test handler_find.
 *
 * Copyright 2022 and 2023 Odin Kroeger.
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

#include <assert.h>
#include <err.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../macros.h"
#include "../params.h"
#include "../pair.h"
#include "util/abort.h"
#include "util/types.h"


/*
 * Data types
 */

/* Mapping of arguments to return values. */
typedef struct {
    const char *const key;
    const char *const value;
    const Error retval;
    int signal;
} PairFindArgs;


/*
 * Main
 */

int
main(void)
{
#if !defined(NDEBUG)

    /* RATS: ignore; used safely. */
    char hugekey[MAX_STR_LEN + 1U] = {0};
#endif

    /* RATS: ignore; used safely. */
    char longkey[MAX_STR_LEN] = {0};

    const PairFindArgs cases[] = {
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

    const Pair pairs[] = {
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

    const size_t npairs = NELEMS(pairs);
    int volatile result = PASS;

#if !defined(NDEBUG)
    (void) memset(hugekey, 'x', sizeof(hugekey) - 1U);
#endif
    (void) memset(longkey, 'x', sizeof(longkey) - 1U);

    for (volatile size_t i = 0; i < NELEMS(cases); ++i) {
        const PairFindArgs args = cases[i];

        if (sigsetjmp(abort_env, 1) == 0) {
            const char *value = NULL;
            size_t keylen;
            Error retval;

            keylen = strnlen(args.key, MAX_STR_LEN);
            assert(keylen < MAX_STR_LEN + 1U);

            if (args.signal != 0) {
                warnx("the next test should fail an assertion.");
            }

            (void) abort_catch(err);
            retval = pair_find(npairs, pairs, keylen, args.key, &value);
            (void) abort_reset(err);

            if (args.retval != retval) {
                warnx("%zu: (%zu, <pairs>, %zu, %s, → %s) → %u [!]",
                      i, npairs, keylen, args.key, value, retval);
                result = FAIL;
            }

            if (retval == OK) {
                if (value == NULL) {
                    warnx("%zu: (%zu, <pairs>, %zu, %s, → %p [!]) → %u",
                          i, npairs, keylen, args.key,
                          (const void *) value, retval);
                    result = FAIL;
                } else if (strncmp(value, args.value, MAX_STR_LEN) != 0) {
                    warnx("%zu: (%zu, <pairs>, %zu, %s, → %s [!]) → %u",
                          i, npairs, keylen, args.key, value, retval);
                    result = FAIL;
                } /* cppcheck-suppress misra-c2012-15.7; no else needed. */
            }
        }

        if (abort_signal != args.signal) {
            warnx("(<pairs>, %s, → <value>) ↑ %s [!]",
                  args.key, strsignal(abort_signal));
            result = FAIL;
        }
    }

    return result;
}
