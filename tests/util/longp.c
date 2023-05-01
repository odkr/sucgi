/*
 * Handling of paths that are longer than PATH_MAX.
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

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#include "../../attr.h"
#include "../../params.h"
#include "longp.h"


/*
 * Constants
 */

/* Maximum depth of nested directories. */
#define MAX_DEPTH 256


/*
 * Prototypes
 */

/* Wrapper around mkdir(2) for longpmake. */
__attribute__((nonnull(2), warn_unused_result))
static int mkdir_w(LongPRole role, const char *fname, va_list argp);


/*
 * Functions
 */

static int
mkdir_w(LongPRole role, const char *fname, va_list argp)
{
    assert(fname != NULL);
    assert(*fname != '\0');

    if (role == LONGP_DIR) {
        return mkdir(fname, (mode_t) va_arg(argp, int));
    }

    return 0;
}

int
longpdo(LongPFunc func, const char *path, ...)
{
    DIR *oldwdp;
    va_list argp;
    const char *ptr;
    int retval, olderr, idx;

    assert(func != NULL);
    assert(path != NULL);
    assert(*path != '\0');

    retval = 0;
    olderr = 0;

    oldwdp = opendir(".");
    if (oldwdp == NULL) {
        return -1;
    }

    ptr = path;
    for (idx = 0; idx < MAX_DEPTH; ++idx) {
        char dir[MAX_FNAME_LEN] = {0};
        const char *sep;
        size_t size;

        sep = strchr(ptr, '/');
        if (sep == NULL) {
            va_start(argp, path);
            retval = func(LONGP_FILE, ptr, argp);
            va_end(argp);

            break;
        }

        size = (size_t) (sep - ptr);
        if (size > sizeof(dir)) {
            errno = ENAMETOOLONG;
            retval = -1;
            break;
        }

        if (size == 0) {
            (void) strncpy(dir, "/", sizeof(dir));
        } else {
            (void) strncpy(dir, ptr, size);
        }
        ptr = &sep[strspn(sep, "/")];

        va_start(argp, path);
        retval = func(LONGP_DIR, dir, argp);
        va_end(argp);

        if (retval != 0) {
            break;
        }

        do {
            retval = chdir(dir);
        } while (retval != 0 && errno == EINTR);

        if (retval != 0) {
            break;
        }
    }

    va_end(argp);

    if (idx == MAX_DEPTH) {
        errno = ENAMETOOLONG;
        retval = -1;
    }

    olderr = errno;
    do {
        if (fchdir(dirfd(oldwdp)) != 0) {
            if (errno == EINTR) {
                continue;
            } else if (retval == 0) {
                retval = -1;
            } else {
                errno = olderr;
            }
        }
        break;
    } while (true);

    return retval;
}

int
longpmake(const char *path, const mode_t perms)
{
    assert(path != NULL);
    assert(*path != '\0');

    return longpdo(mkdir_w, path, perms);
}
