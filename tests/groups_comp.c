/*
 * Test env_is_name.
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

#include <err.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../env.h"
#include "../groups.h"
#include "../macros.h"
#include "util/abort.h"
#include "util/types.h"


/*
 * Data types
 */

/* Mapping of a string to a return value. */
typedef struct {
    const gid_t gid1;
    const gid_t gid2;
    int retval;
} GroupsCompArgs;


/*
 * Main
 */

int
main(void)
{
    const GroupsCompArgs cases[] = {
        {0, 0, 0},
        {MAX_GRP_VAL, MAX_GRP_VAL, 0},
        {MAX_GID_VAL, MAX_GID_VAL, 0},
        {0, MAX_GRP_VAL, -1},
        {MAX_GRP_VAL, 0, 1},
        {0, MAX_GID_VAL, -1},
        {MAX_GID_VAL, 0, 1},
    };

    volatile int result = PASS;

    for (volatile size_t i = 0; i < NELEMS(cases); ++i) {
        const GroupsCompArgs args = cases[i];

        if (sigsetjmp(abort_env, 1) == 0) {
            int retval;

            (void) abort_catch(err);
            retval = groups_comp(&args.gid1, &args.gid2);
            (void) abort_reset(err);

            if (retval != args.retval) {
                result = FAIL;
                if (ISSIGNED(gid_t)) {
                    warnx("%zu: (%lld, %lld) → %d [!]", i,
                          (long long) args.gid1,
                          (long long) args.gid2,
                          retval);
                } else {
                    warnx("%zu: (%llu, %llu) → %d [!]", i,
                          (unsigned long long) args.gid1,
                          (unsigned long long) args.gid2,
                          retval);
                }
            }
        }

        if (abort_signal != 0) {
            result = FAIL;
            if (ISSIGNED(gid_t)) {
                warnx("%zu: (%lld, %lld) ↑ %s [!]", i,
                      (long long) args.gid1,
                      (long long) args.gid2,
                      strsignal(abort_signal));
            } else {
                warnx("%zu: (%llu, %llu) ↑ %s [!]", i,
                      (unsigned long long) args.gid1,
                      (unsigned long long) args.gid2,
                      strsignal(abort_signal));
            }
        }
    }

    return result;
}
