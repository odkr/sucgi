/*
 * Test file_is_exe.
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

/* Shorthand to create a struct stat object with UID, GID, and MODE. */
#define STAT(uid, gid, mode) \
    (const struct stat) {.st_uid = (uid), .st_gid = (gid), .st_mode = (mode)}


/*
 * Data types
 */

/* Mapping of arguments to a return value. */
typedef struct {
    const uid_t uid;
    const gid_t gid;
    const struct stat fstatus;
    const bool ret;
} Args;


/*
 * Prototypes
 */

/* Test file_is_exe. */
static void test(uid_t uid, gid_t gid, struct stat fstatus, bool ret);


/*
 * Module variables
 */

/* Static test cases. */
static const Args cases[] = {
    /* Simple tests. */
    {0, 0, {.st_uid = 0, .st_gid = 0, .st_mode =    0}, false},
    {0, 0, {.st_uid = 0, .st_gid = 0, .st_mode = 0700}, true},
    {0, 0, {.st_uid = 0, .st_gid = 0, .st_mode = 0500}, true},
    {0, 0, {.st_uid = 0, .st_gid = 0, .st_mode = 0300}, true},
    {0, 0, {.st_uid = 0, .st_gid = 0, .st_mode = 0100}, true},

    /* Owner matches. */
    {500, 0, {.st_uid = 500, .st_gid = 0, .st_mode =    0}, false},
    {500, 0, {.st_uid = 500, .st_gid = 0, .st_mode = 0700}, true},
    {500, 0, {.st_uid = 500, .st_gid = 0, .st_mode = 0600}, false},
    {500, 0, {.st_uid = 500, .st_gid = 0, .st_mode = 0500}, true},
    {500, 0, {.st_uid = 500, .st_gid = 0, .st_mode = 0400}, false},
    {500, 0, {.st_uid = 500, .st_gid = 0, .st_mode = 0300}, true},
    {500, 0, {.st_uid = 500, .st_gid = 0, .st_mode = 0200}, false},
    {500, 0, {.st_uid = 500, .st_gid = 0, .st_mode = 0100}, true},
    {500, 0, {.st_uid = 500, .st_gid = 0, .st_mode =  070}, false},
    {500, 0, {.st_uid = 500, .st_gid = 0, .st_mode =  050}, false},
    {500, 0, {.st_uid = 500, .st_gid = 0, .st_mode =  030}, false},
    {500, 0, {.st_uid = 500, .st_gid = 0, .st_mode =  010}, false},
    {500, 0, {.st_uid = 500, .st_gid = 0, .st_mode =   07}, false},
    {500, 0, {.st_uid = 500, .st_gid = 0, .st_mode =   05}, false},
    {500, 0, {.st_uid = 500, .st_gid = 0, .st_mode =   03}, false},
    {500, 0, {.st_uid = 500, .st_gid = 0, .st_mode =   01}, false},

    /* Group matches. */
    {1, 500, {.st_uid = 2, .st_gid = 500, .st_mode =    0}, false},
    {1, 500, {.st_uid = 2, .st_gid = 500, .st_mode = 0700}, false},
    {1, 500, {.st_uid = 2, .st_gid = 500, .st_mode = 0500}, false},
    {1, 500, {.st_uid = 2, .st_gid = 500, .st_mode = 0300}, false},
    {1, 500, {.st_uid = 2, .st_gid = 500, .st_mode = 0100}, false},
    {1, 500, {.st_uid = 2, .st_gid = 500, .st_mode =  070}, true},
    {1, 500, {.st_uid = 2, .st_gid = 500, .st_mode =  060}, false},
    {1, 500, {.st_uid = 2, .st_gid = 500, .st_mode =  050}, true},
    {1, 500, {.st_uid = 2, .st_gid = 500, .st_mode =  040}, false},
    {1, 500, {.st_uid = 2, .st_gid = 500, .st_mode =  030}, true},
    {1, 500, {.st_uid = 2, .st_gid = 500, .st_mode =  020}, false},
    {1, 500, {.st_uid = 2, .st_gid = 500, .st_mode =  010}, true},
    {1, 500, {.st_uid = 2, .st_gid = 500, .st_mode =   07}, false},
    {1, 500, {.st_uid = 2, .st_gid = 500, .st_mode =   05}, false},
    {1, 500, {.st_uid = 2, .st_gid = 500, .st_mode =   03}, false},
    {1, 500, {.st_uid = 2, .st_gid = 500, .st_mode =   01}, false},

    /* No match. */
    {1, 1, {.st_uid = 2, .st_gid = 2, .st_mode =    0}, false},
    {1, 1, {.st_uid = 2, .st_gid = 2, .st_mode = 0700}, false},
    {1, 1, {.st_uid = 2, .st_gid = 2, .st_mode = 0500}, false},
    {1, 1, {.st_uid = 2, .st_gid = 2, .st_mode = 0300}, false},
    {1, 1, {.st_uid = 2, .st_gid = 2, .st_mode = 0100}, false},
    {1, 1, {.st_uid = 2, .st_gid = 2, .st_mode =  070}, false},
    {1, 1, {.st_uid = 2, .st_gid = 2, .st_mode =  050}, false},
    {1, 1, {.st_uid = 2, .st_gid = 2, .st_mode =  030}, false},
    {1, 1, {.st_uid = 2, .st_gid = 2, .st_mode =  010}, false},
    {1, 1, {.st_uid = 2, .st_gid = 2, .st_mode =   07}, true},
    {1, 1, {.st_uid = 2, .st_gid = 2, .st_mode =   06}, false},
    {1, 1, {.st_uid = 2, .st_gid = 2, .st_mode =   05}, true},
    {1, 1, {.st_uid = 2, .st_gid = 2, .st_mode =   04}, false},
    {1, 1, {.st_uid = 2, .st_gid = 2, .st_mode =   03}, true},
    {1, 1, {.st_uid = 2, .st_gid = 2, .st_mode =   02}, false},
    {1, 1, {.st_uid = 2, .st_gid = 2, .st_mode =   01}, true},

    /* The superuser is special. */
    {0, 0, {.st_uid = 0, .st_gid = 1, .st_mode = 070}, true},
    {0, 0, {.st_uid = 0, .st_gid = 1, .st_mode = 060}, false},
    {0, 0, {.st_uid = 0, .st_gid = 1, .st_mode = 050}, true},
    {0, 0, {.st_uid = 0, .st_gid = 1, .st_mode = 040}, false},
    {0, 0, {.st_uid = 0, .st_gid = 1, .st_mode = 030}, true},
    {0, 0, {.st_uid = 0, .st_gid = 1, .st_mode = 020}, false},
    {0, 0, {.st_uid = 0, .st_gid = 1, .st_mode = 010}, true},
    {0, 0, {.st_uid = 0, .st_gid = 1, .st_mode =  07}, true},
    {0, 0, {.st_uid = 0, .st_gid = 1, .st_mode =  06}, false},
    {0, 0, {.st_uid = 0, .st_gid = 1, .st_mode =  05}, true},
    {0, 0, {.st_uid = 0, .st_gid = 1, .st_mode =  04}, false},
    {0, 0, {.st_uid = 0, .st_gid = 1, .st_mode =  03}, true},
    {0, 0, {.st_uid = 0, .st_gid = 1, .st_mode =  02}, false},
    {0, 0, {.st_uid = 0, .st_gid = 1, .st_mode =  01}, true}
};


/*
 * Functions
 */

static void
test(const uid_t uid, const gid_t gid,
     const struct stat fstatus, const bool ret)
{
    bool retret;

    warnx(
        "checking "
        "(%llu, %llu, {.st_uid = %llu, .st_gid = %llu, .st_mode = 0%o})"
        " -> %d ...",
        (unsigned long long) uid,
        (unsigned long long) gid,
        (unsigned long long) fstatus.st_uid,
        (unsigned long long) fstatus.st_gid,
        fstatus.st_mode,
        ret
    );

    retret = file_is_exe(uid, gid, fstatus);

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
    const id_t ids[] = {
        0, 1, 99, 100, 101, 499, 500, 501, 999,
        1000, 1001, 59999, 60000, 60001, INT_MAX
    };

    /* Static test cases. */
    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];
        test(args.uid, args.gid, args.fstatus, args.ret);
    }

    /* Dynamic test cases. */
    for (size_t i = 0; i < NELEMS(ids); ++i) {
        id_t a = ids[i];
        id_t b = ids[(i + 1) % NELEMS(ids)];

        for (mode_t perms = 0; perms <= 0777; ++perms) {
            const bool usrx = (perms & 0100);
            const bool grpx = (perms & 0010);
            const bool othx = (perms & 0001);
            const bool supx = (usrx || grpx || othx);

            if (a != 0) {
                /* user ID matches, group ID matches. */
                test(a, a, STAT(a, a, perms), usrx);

                /* user ID does not match, group ID matches. */
                test(a, a, STAT(b, a, perms), grpx);

                /* user ID matches, group ID does not match. */
                test(a, a, STAT(a, b, perms), usrx);

                /* user ID does not match, group ID does not match. */
                test(a, a, STAT(b, b, perms), othx);
            } else {
                /* user ID matches, group ID matches. */
                test(a, a, STAT(a, a, perms), supx);

                /* user ID does not match, group ID matches. */
                test(a, a, STAT(b, a, perms), supx);

                /* user ID matches, group ID does not match. */
                test(a, a, STAT(a, b, perms), supx);

                /* user ID does not match, group ID does not match. */
                test(a, a, STAT(b, b, perms), supx);
            }
        }
    }

    warnx("all tests passed");
    return EXIT_SUCCESS;
}
