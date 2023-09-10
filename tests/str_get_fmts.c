/*
 * Test str_get_fmts.
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

#include <assert.h>
#include <err.h>
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../macros.h"
#include "../params.h"
#include "../str.h"
#include "../types.h"
#include "libutil/abort.h"
#include "libutil/array.h"
#include "libutil/str.h"
#include "libutil/types.h"


/*
 * Constants
 */

/* Maximum array length. */
#define MAX_ARRAY_LEN 32U


/*
 * Data types
 */

/* Mapping of arguments to return values. */
typedef struct {
    const char *const str;
    const size_t maxnspecs;
    const size_t nspecs;
    /* RATS: ignore; used safely. */
    const char *const specs[100];
    const Error retval;
    const int signal;
} StrFmtSpecsArgs;


/*
 * Main
 */

int
main(void)
{
#if !defined(NDEBUG)

    /* RATS: ignore; used safely. */
    char hugestr[MAX_STR_LEN + 1U];
    str_fill(sizeof(hugestr), hugestr, 'x');
#endif

    /* RATS: ignore; used safely. */
    char longstr[MAX_STR_LEN];
    str_fill(sizeof(longstr), longstr, 'x');

    /* cppcheck-suppress [misra-c2012-9.3, misra-c2012-9.4] */
    const StrFmtSpecsArgs cases[] = {
#if !defined(NDEBUG)
        /* Invalid argument. */
        {hugestr, MAX_ARRAY_LEN, 0, {NULL}, OK, SIGABRT},
#endif

        /* Long string. */
        {longstr, MAX_ARRAY_LEN, 0, {NULL}, OK, 0},

        /* Empty string. */
        {"", MAX_ARRAY_LEN, 0, {NULL}, OK, 0},

        /* Simple tests. */
        {"foo", MAX_ARRAY_LEN, 0, {NULL}, OK, 0},
        {"%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
        {"foo%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
        {"%sfoo", MAX_ARRAY_LEN, 1, {"sfoo"}, OK, 0},
        {"f%so", MAX_ARRAY_LEN, 1, {"so"}, OK, 0},
        {"%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
        {"foo%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
        {"%sfoo", MAX_ARRAY_LEN, 1, {"sfoo"}, OK, 0},
        {"f%so", MAX_ARRAY_LEN, 1, {"so"}, OK, 0},

        /* Multiple specifiers. */
        {"%s%s", MAX_ARRAY_LEN, 2, {"s%s", "s"}, OK, 0},
        {"foo%s%s", MAX_ARRAY_LEN, 2, {"s%s", "s"}, OK, 0},
        {"%s%sfoo", MAX_ARRAY_LEN, 2, {"s%sfoo", "sfoo"}, OK, 0},
        {"f%s%so", MAX_ARRAY_LEN, 2, {"s%so", "so"}, OK, 0},
        {"%sfoo%s", MAX_ARRAY_LEN, 2, {"sfoo%s", "s"}, OK, 0},
        {"%s%s", 1, 1, {"s%s"}, ERR_LEN, 0},
        {"%s%s%s", 2, 2, {"s%s%s", "s%s"}, ERR_LEN, 0},
        {"%s%s%s%s", 3, 3, {"s%s%s%s", "s%s%s", "s%s"}, ERR_LEN, 0},

        /* One escaped specifier. */
        {"%%s", MAX_ARRAY_LEN, 0, {NULL}, OK, 0},
        {"foo%%s", MAX_ARRAY_LEN, 0, {NULL}, OK, 0},
        {"%%sfoo", MAX_ARRAY_LEN, 0, {NULL}, OK, 0},
        {"f%%so", MAX_ARRAY_LEN, 0, {NULL}, OK, 0},

        /* Specifiers after escaped '%'. */
        {"%%%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
        {"%%%s%d", MAX_ARRAY_LEN, 2, {"s%d", "d"}, OK, 0},
        {"foo%%%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
        {"%%sfoo%%%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
        {"%%sfoo%%%s%d", MAX_ARRAY_LEN, 2, {"s%d", "d"}, OK, 0},
        {"f%%so%%%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
        {"%%%%%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
        {"%%%%%s%%%a", MAX_ARRAY_LEN, 2, {"s%%%a", "a"}, OK, 0},
        {"foo%%%%%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
        {"%%sfoo%%%%%s%%%%%e", MAX_ARRAY_LEN, 2, {"s%%%%%e", "e"}, OK, 0},
        {"f%%so%%%%%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
        {"%%s%s%%%e%A", MAX_ARRAY_LEN, 3, {"s%%%e%A", "e%A", "A"}, OK, 0},
        {"foo%%s%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
        {"%%%%sfoo%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
        {"f%%%%so%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},

        /* Unicode shenanigans. */
        {"𝕗ⓞⓞ", MAX_ARRAY_LEN, 0, {NULL}, OK, 0},
        {"%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
        {"𝕗ⓞⓞ%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
        {"%s𝕗ⓞⓞ", MAX_ARRAY_LEN, 1, {"s𝕗ⓞⓞ"}, OK, 0},
        {"𝕗%sⓞ", MAX_ARRAY_LEN, 1, {"sⓞ"}, OK, 0},
        {"%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
        {"𝕗ⓞⓞ%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
        {"%s𝕗ⓞⓞ", MAX_ARRAY_LEN, 1, {"s𝕗ⓞⓞ"}, OK, 0},
        {"𝕗%sⓞ", MAX_ARRAY_LEN, 1, {"sⓞ"}, OK, 0},
        {"%s%s", MAX_ARRAY_LEN, 2, {"s%s", "s"}, OK, 0},
        {"𝕗ⓞⓞ%s%s", MAX_ARRAY_LEN, 2, {"s%s", "s"}, OK, 0},
        {"%s%s𝕗ⓞⓞ", MAX_ARRAY_LEN, 2, {"s%s𝕗ⓞⓞ", "s𝕗ⓞⓞ"}, OK, 0},
        {"𝕗%s%sⓞ", MAX_ARRAY_LEN, 2, {"s%sⓞ", "sⓞ"}, OK, 0},
        {"%s𝕗ⓞⓞ%s", MAX_ARRAY_LEN, 2, {"s𝕗ⓞⓞ%s", "s"}, OK, 0},
        {"%s%s", 1, 1, {"s%s"}, ERR_LEN, 0},
        {"%s%s%s", 2, 2, {"s%s%s", "s%s"}, ERR_LEN, 0},
        {"%s%s%s%s", 3, 3, {"s%s%s%s", "s%s%s", "s%s"}, ERR_LEN, 0},
        {"%%s", MAX_ARRAY_LEN, 0, {NULL}, OK, 0},
        {"𝕗ⓞⓞ%%s", MAX_ARRAY_LEN, 0, {NULL}, OK, 0},
        {"%%s𝕗ⓞⓞ", MAX_ARRAY_LEN, 0, {NULL}, OK, 0},
        {"𝕗%%sⓞ", MAX_ARRAY_LEN, 0, {NULL}, OK, 0},
        {"%%%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
        {"%%%s%d", MAX_ARRAY_LEN, 2, {"s%d", "d"}, OK, 0},
        {"𝕗ⓞⓞ%%%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
        {"%%s𝕗ⓞⓞ%%%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
        {"%%s𝕗ⓞⓞ%%%s%d", MAX_ARRAY_LEN, 2, {"s%d", "d"}, OK, 0},
        {"𝕗%%sⓞ%%%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
        {"%%%%%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
        {"%%%%%s%%%a", MAX_ARRAY_LEN, 2, {"s%%%a", "a"}, OK, 0},
        {"𝕗ⓞⓞ%%%%%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
        {"%%s𝕗ⓞⓞ%%%%%s%%%%%e", MAX_ARRAY_LEN, 2, {"s%%%%%e", "e"}, OK, 0},
        {"𝕗%%sⓞ%%%%%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
        {"%%s%s%%%e%A", MAX_ARRAY_LEN, 3, {"s%%%e%A", "e%A", "A"}, OK, 0},
        {"𝕗ⓞⓞ%%s%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
        {"%%%%s𝕗ⓞⓞ%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
        {"𝕗%%%%sⓞ%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0}
    };

    volatile int result = PASS;

    for (volatile size_t i = 0; i < NELEMS(cases); ++i) {
        const StrFmtSpecsArgs args = cases[i];

        if (sigsetjmp(abort_env, 1) == 0) {
            if (args.signal != 0) {
                warnx("the next test should fail an assertion.");
            }

            /* RATS: ignore; used safely. */
            const char *specs[MAX_ARRAY_LEN];
            size_t nspecs = 0;

            (void) abort_catch(err);
            const Error retval = str_get_fmts(args.str, args.maxnspecs,
                                              &nspecs, specs);
            (void) abort_reset(err);

            if (retval != args.retval) {
                result = FAIL;
                warnx("%zu: (%s, %zu, → %zu, → <specs>) → %u [!]",
                      i, args.str, args.maxnspecs, nspecs, retval);
            }

            if (retval == OK) {
                if (nspecs != args.nspecs) {
                    result = FAIL;
                    warnx("%zu: (%s, %zu, → %zu [!], → <specs>) → %u",
                          i, args.str, args.maxnspecs, nspecs, retval);
                }

                if (!array_equals(args.specs, nspecs, sizeof(*args.specs),
                              specs, nspecs, sizeof(*specs),
                              (CompFn) str_cmp_ptrs))
                {
                    result = FAIL;
                    warnx("%zu: (%s, %zu, → %zu, → <specs> [!]) → %u",
                          i, args.str, args.maxnspecs, nspecs, retval);
                }
            }
        }

        if (abort_signal != args.signal) {
            result = FAIL;
            warnx("%zu: (%s, %zu, → <nspecs>, → <specs> [!]) ↑ %s",
                  i, args.str, args.maxnspecs, strerror(abort_signal));
        }
    }

    return result;
}
