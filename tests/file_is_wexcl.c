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

#include <err.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>

#include "../file.h"
#include "../macros.h"
#include "lib.h"



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
 * Prototypes
 */

/* Test file_is_wexcl. */
static void test(uid_t uid, struct stat fstatus, bool ret);


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
 * Functions
 */

static void
test(const uid_t uid, const struct stat fstatus, const bool ret)
{
    bool retret;

    warnx(
        "checking (%llu, {.st_uid = %llu, .st_mode = 0%o})) -> %d ...",
        (unsigned long long) uid, (unsigned long long) fstatus.st_gid,
        fstatus.st_mode, ret
    );

    retret = file_is_wexcl(uid, fstatus);

    if (retret != ret) {
        errx(TEST_FAILED, "returned %d", retret);
    }
}


/*
 * Main
 */

int
main(void)
{
    const uid_t uids[] = {
        0, 1, 99, 100, 101, 499, 500, 501, 999,
        1000, 1001, 59999, 60000, 60001, INT_MAX
    };

    /* Static test cases. */
    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];
        test(args.uid, args.fstatus, args.ret);
    }

    /* Dynamic tests. */
    for (size_t i = 0; i < NELEMS(uids); ++i) {
        uid_t a = uids[i];
        uid_t b = uids[(i + 1) % NELEMS(uids)];

        for (mode_t perms = 0; perms <= 0777; ++perms) {
            test(a, STAT(a, perms), !(perms & (S_IWGRP | S_IWOTH)));
            test(a, STAT(b, perms), false);
        }
    }

    warnx("all tests passed");
    return EXIT_SUCCESS;
}
