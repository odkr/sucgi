/*
 * Utility functions for testing priv.o.
 *
 * Copyright 2023 Odin Kroeger.
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
#include <assert.h>
#include <err.h>
#include <errno.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"
#include "priv.h"


/*
 * Functions
 */

bool
checkgetreguid(uid_t *const uid) {
    int retval = false;
    int olderr = 0;
    struct passwd *pwd;

    setpwent();
    while ((errno = 0, pwd = getpwent()) != NULL) {
        if (pwd->pw_uid > 0) {
            *uid = pwd->pw_uid;
            retval = true;
            break;
        }
    }
    olderr = errno;
    endpwent();

    errno = olderr;
    return retval;
}

void
checkgetuser(uid_t uid, struct passwd *pwd)
{
    long bufsize;
    char *buffer;
    struct passwd *result;

    bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    assert(bufsize > 0L);
    assert((uint64_t) bufsize < (uint64_t) SIZE_MAX);

    errno = 0;
    buffer = malloc((size_t) bufsize);
    if (buffer == NULL) {
        errx(TEST_ERROR, "malloc");
    }

    errno = getpwuid_r(uid, pwd, buffer, (size_t) bufsize, &result);
    if (result == NULL) {
        if (errno == 0) {
            errx(TEST_ERROR, "user ID %llu: unallocated",
                 (unsigned long long) uid);
        }
        err(TEST_ERROR, "getpwuid_r");
    }
}
