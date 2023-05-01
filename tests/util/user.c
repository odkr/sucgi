/*
 * Utility functions for searching users.
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

#define _XOPEN_SOURCE 700

#include <sys/types.h>
#include <assert.h>
#include <err.h>
#include <errno.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "check.h"
#include "user.h"


/*
 * Functions
 */

UserError
usergetregular(uid_t *const uid) {
    struct passwd *pwd;
    int olderr;
    UserError retval;

    assert(uid != NULL);

    retval = USER_NOTFOUND;
    setpwent();
    while ((errno = 0, pwd = getpwent()) != NULL) {
        if (pwd->pw_uid > 0) {
            *uid = pwd->pw_uid;
            retval = USER_SUCCESS;
            break;
        }
    }
    olderr = errno;
    endpwent();

    if (pwd == NULL && olderr != 0) {
        errno = olderr;
        return USER_ERROR;
    }

    return retval;
}

UserError
userget(uid_t uid, struct passwd *pwd)
{
    long bufsize;
    char *buffer;
    struct passwd *result;

    errno = 0;
    bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    assert(bufsize > 0L);
    assert((uint64_t) bufsize < (uint64_t) SIZE_MAX);

    /* buffer will leak, of course. */
    errno = 0;
    buffer = malloc((size_t) bufsize);
    if (buffer == NULL) {
        return USER_ERROR;
    }

    result = NULL;
    errno = getpwuid_r(uid, pwd, buffer, (size_t) bufsize, &result);
    if (result == NULL) {
        if (errno == 0) {
            return USER_NOTFOUND;
        }
        return USER_ERROR;
    }

    return USER_SUCCESS;
}
