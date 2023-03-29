/*
 * Test file_is_wexcl.
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

#include <sys/types.h>
#include <err.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>

#include "../file.h"
#include "../macros.h"
#include "result.h"



/*
 * Macros
 */

/* Shorthand to create a struct stat object with UID and MODE. */
#define STAT(uid, mode) \
    (const struct stat) {.st_uid = (uid), .st_mode = (mode)}


/*
 * Data types
 */

/* Mapping of arguments to a return value. */
typedef struct {
    const uid_t uid;
    const struct stat fstatus;
    const bool ret;
} Args;


/*
 * Module variables
 */

/* Static test cases. */
static const Args cases[] = {
    /* Owner matches. */
    {0, {.st_uid = 0, .st_mode = 0000}, true},
    {0, {.st_uid = 0, .st_mode = 0100}, true},
    {0, {.st_uid = 0, .st_mode = 0200}, true},
    {0, {.st_uid = 0, .st_mode = 0300}, true},
    {0, {.st_uid = 0, .st_mode = 0400}, true},
    {0, {.st_uid = 0, .st_mode = 0500}, true},
    {0, {.st_uid = 0, .st_mode = 0600}, true},
    {0, {.st_uid = 0, .st_mode = 0700}, true},
    {0, {.st_uid = 0, .st_mode = 0010}, true},
    {0, {.st_uid = 0, .st_mode = 0020}, false},
    {0, {.st_uid = 0, .st_mode = 0030}, false},
    {0, {.st_uid = 0, .st_mode = 0040}, true},
    {0, {.st_uid = 0, .st_mode = 0050}, true},
    {0, {.st_uid = 0, .st_mode = 0060}, false},
    {0, {.st_uid = 0, .st_mode = 0070}, false},
    {0, {.st_uid = 0, .st_mode = 0001}, true},
    {0, {.st_uid = 0, .st_mode = 0002}, false},
    {0, {.st_uid = 0, .st_mode = 0003}, false},
    {0, {.st_uid = 0, .st_mode = 0004}, true},
    {0, {.st_uid = 0, .st_mode = 0005}, true},
    {0, {.st_uid = 0, .st_mode = 0006}, false},
    {0, {.st_uid = 0, .st_mode = 0007}, false},

    /* Owner does not match. */
    {0, {.st_uid = 1, .st_mode = 0000}, false},
    {0, {.st_uid = 1, .st_mode = 0100}, false},
    {0, {.st_uid = 1, .st_mode = 0200}, false},
    {0, {.st_uid = 1, .st_mode = 0300}, false},
    {0, {.st_uid = 1, .st_mode = 0400}, false},
    {0, {.st_uid = 1, .st_mode = 0500}, false},
    {0, {.st_uid = 1, .st_mode = 0600}, false},
    {0, {.st_uid = 1, .st_mode = 0700}, false},
    {0, {.st_uid = 1, .st_mode = 0010}, false},
    {0, {.st_uid = 1, .st_mode = 0020}, false},
    {0, {.st_uid = 1, .st_mode = 0030}, false},
    {0, {.st_uid = 1, .st_mode = 0040}, false},
    {0, {.st_uid = 1, .st_mode = 0050}, false},
    {0, {.st_uid = 1, .st_mode = 0060}, false},
    {0, {.st_uid = 1, .st_mode = 0070}, false},
    {0, {.st_uid = 1, .st_mode = 0001}, false},
    {0, {.st_uid = 1, .st_mode = 0002}, false},
    {0, {.st_uid = 1, .st_mode = 0003}, false},
    {0, {.st_uid = 1, .st_mode = 0004}, false},
    {0, {.st_uid = 1, .st_mode = 0005}, false},
    {0, {.st_uid = 1, .st_mode = 0006}, false},
    {0, {.st_uid = 1, .st_mode = 0007}, false},
};


/*
 * Main
 */

int
main(void)
{
    int result = TEST_PASSED;

    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];
        bool ret;

        ret = file_is_wexcl(args.uid, args.fstatus);
        if (ret != args.ret) {
            warnx("(%d, {%d, 0%03o})) -> %d [!]",
                  (int) args.uid, (int) args.fstatus.st_gid,
                  args.fstatus.st_mode, ret);
            result = TEST_FAILED;
        }
    }

    return result;
}
