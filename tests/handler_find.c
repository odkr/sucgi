/*
 * Test handler_find.
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
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../handler.h"
#include "../macros.h"
#include "../params.h"
#include "libutil/abort.h"
#include "libutil/str.h"
#include "libutil/types.h"


/*
 * Data types
 */

/* Mapping of arguments to return values. */
typedef struct {
    const char *const script;
    const char *const handler;
    const Error retval;
    int signal;
} HandlerFindArgs;


/*
 * Main
 */

int
main(void)
{
    /* RATS: ignore; used safely. */
    char hugefname[(size_t) MAX_FNAME_LEN + 1U] _unused;
    str_fill(sizeof(hugefname), hugefname, 'x');

    /* RATS: ignore; used safely. */
    char longfname[MAX_FNAME_LEN];
    str_fill(sizeof(longfname), longfname, 'x');

    const HandlerFindArgs cases[] = {
#if !defined(NDEBUG)
        /* Illegal arguments. */
        {hugefname, "<n/a>", OK, SIGABRT},
        {"", "<n/a>", OK, SIGABRT},
#endif

        /* Simple errors. */
        {"file", "<n/a>", ERR_SUFFIX, 0},
        {".", "<n/a>", ERR_SUFFIX, 0},
        {".sh", "<n/a>", ERR_SUFFIX, 0},
        {".py", "<n/a>", ERR_SUFFIX, 0},
        {"file.null", "<n/a>", ERR_BAD, 0},
        {"file.empty", "<n/a>", ERR_BAD, 0},
        {"file.py", "<n/a>", ERR_SEARCH, 0},
        {"file.post", "<n/a>", ERR_SEARCH, 0},
        {"long.suffix-0123456789abcdef", "<n/a>", ERR_LEN, 0},

        /* Empty string shenanigans. */
        {" ", "<n/a>", ERR_SUFFIX, 0},
        {". ", "<n/a>", ERR_SUFFIX, 0},
        {".sh ", "<n/a>", ERR_SUFFIX, 0},
        {".py ", "<n/a>", ERR_SUFFIX, 0},
        {" .null", "<n/a>", ERR_BAD, 0},
        {" .empty", "<n/a>", ERR_BAD, 0},
        {" .py", "<n/a>", ERR_SEARCH, 0},
        {" .post", "<n/a>", ERR_SEARCH, 0},
        {" . ", "<n/a>", ERR_SEARCH, 0},

        /* Unicode shenanigans. */
        {"𝕗ïḻę", "<n/a>", ERR_SUFFIX, 0},
        {".", "<n/a>", ERR_SUFFIX, 0},
        {".sh", "<n/a>", ERR_SUFFIX, 0},
        {".py", "<n/a>", ERR_SUFFIX, 0},
        {"𝕗ïḻę.null", "<n/a>", ERR_BAD, 0},
        {"𝕗ïḻę.empty", "<n/a>", ERR_BAD, 0},
        {"𝕗ïḻę.py", "<n/a>", ERR_SEARCH, 0},
        {"𝕗ïḻę.post", "<n/a>", ERR_SEARCH, 0},
        {"𝕗ïḻę.suffix-0123456789abcdef", "<n/a>", ERR_LEN, 0},

        /* Simple tests. */
        {longfname, "<n/a>", ERR_SUFFIX, 0},
        {"file.sh", "sh", OK, 0},
        {"file.", "dot", OK, 0}
    };

    const Pair handlers[] = {
        {"", "unreachable"},
        {".", "dot"},
        {".sh", "sh"},
        {".null", NULL},
        {".empty", ""},
        {".pre", "pre"},
        {".suffix-0123456789abcdef", "<unreachable>"}
    };

    volatile int result = PASS;

    for (volatile size_t i = 0; i < NELEMS(cases); ++i) {
        const HandlerFindArgs args = cases[i];

        if (sigsetjmp(abort_env, 1) == 0) {
            size_t scriptlen = strnlen(args.script,
                                       (size_t) MAX_FNAME_LEN + 1U);
            assert(scriptlen <= (size_t) MAX_FNAME_LEN);

            if (args.signal != 0) {
                warnx("the next test should fail an assertion.");
            }

            const char *handler = NULL;

            (void) abort_catch(err);
            const Error retval = handler_find(NELEMS(handlers), handlers,
                                              scriptlen, args.script, &handler);
            (void) abort_reset(err);

            if (args.retval != retval) {
                warnx("(<handlers>, %s, → %s) → %u [!]",
                      args.script, handler, retval);
                result = FAIL;
            }

            if (retval == OK) {
                if (handler == NULL) {
                    result = FAIL;
                    warnx("(<handlers>, %s, → %p [!]) → %u",
                          args.script, (const void *) handler, retval);
                } else if (strncmp(handler, args.handler, MAX_STR_LEN) != 0) {
                    result = FAIL;
                    warnx("(<handlers>, %s, → %s [!]) → %u",
                          args.script, handler, retval);
                } /* cppcheck-suppress misra-c2012-15.7; no else needed. */
            }
        }

        if (abort_signal != args.signal) {
            result = FAIL;
            warnx("(<handlers>, %s, → <handler>) ↑ %s [!]",
                  args.script, strsignal(abort_signal));
        }
    }

    return result;
}
