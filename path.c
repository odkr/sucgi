/*
 * Path handling.
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

#define _XOPEN_SOURCE 700

#if defined(__OPTIMIZE__) && !defined(_FORTIFY_SOURCE)
#define _FORTIFY_SOURCE 3
#endif

#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "file.h"
#include "max.h"
#include "path.h"
#include "str.h"
#include "types.h"


Error
pathchkloc(const char *const basedir, const char *const fname)
{
    assert(basedir);
    assert(*basedir != '\0');
    assert(fname);
    assert(*fname != '\0');

    if (strncmp(fname, "/", sizeof("/")) != 0 &&
        strncmp(fname, ".", sizeof(".")) != 0)
    {
        size_t basedir_len;
        size_t fname_len;

        if (*fname == '/') {
            if (strncmp(basedir, "/", sizeof("/")) == 0) {
                return OK;
            }
        } else {
            if (strncmp(basedir, ".", sizeof(".")) == 0) {
                return OK;
            }
        }

        basedir_len = strnlen(basedir, MAX_FNAME_LEN);
        if (basedir_len >= (size_t) MAX_FNAME_LEN) {
            return ERR_LEN;
        }

        fname_len = strnlen(fname, MAX_FNAME_LEN);
        if (fname_len >= (size_t) MAX_FNAME_LEN) {
            return ERR_LEN;
        }

        if (fname_len > basedir_len                     &&
            fname[basedir_len] == '/'                   &&
            strncmp(basedir, fname, basedir_len) == 0)
        {
            return OK;
        }
    }

    return ERR_BASEDIR;
}

Error
pathchkperm(const uid_t uid, const char *const basedir,
                 const char *const fname)
{
    const char *pos;
    Error ret;

    assert(basedir != NULL);
    assert(*basedir == '/');
    assert(strnlen(basedir, MAX_FNAME_LEN) < (size_t) MAX_FNAME_LEN);
    assert(fname != NULL);
    assert(*fname == '/');
    assert(strnlen(fname, MAX_FNAME_LEN) < (size_t) MAX_FNAME_LEN);

    ret = pathchkloc(basedir, fname);
    if (ret != OK) {
        return ret;
    }

    /* cppcheck-suppress misra-c2012-18.4; basedir is shorter than fname. */
    pos = fname + strnlen(basedir, MAX_FNAME_LEN);
    do {
        /* RATS: ignore; pathchkloc would have errored out if
           basedir or fname were longer than MAX_FNAME_LEN. */
        char cur[MAX_FNAME_LEN];
        struct stat fstatus;

        /* cppcheck-suppress [misra-c2012-10.8, misra-c2012-18.4];
           cast is safe and portable, pos always points to a char in fname. */
        (void) copystr((size_t) (pos - fname), fname, cur);

        if (stat(cur, &fstatus) != 0) {
            return ERR_SYS;
        }

        if (!fileisxusrw(uid, fstatus)) {
            return ERR_PERM;
        }

        /* cppcheck-suppress misra-c2012-18.4; only moves past '/'s. */
        pos += strspn(pos, "/");
        if (*pos == '\0') {
            break;
        }

        /* cppcheck-suppress misra-c2012-18.4; only moves to the next '/'. */
        pos += strcspn(pos, "/");
    } while (true);

    return OK;
}

Error
pathreal(const char *const fname, const char **const real)
{
    assert(fname != NULL);
    assert(*fname != '\0');
    assert(real != NULL);

    if (strnlen(fname, MAX_FNAME_LEN) >= (size_t) MAX_FNAME_LEN) {
        return ERR_LEN;
    }

    errno = 0;
    /* RATS: ignore; length of input is checked. */
    *real = realpath(fname, NULL);
    if (*real == NULL) {
        return ERR_SYS;
    }

    if (strnlen(*real, MAX_FNAME_LEN) >= (size_t) MAX_FNAME_LEN) {
        return ERR_LEN;
    }

    return OK;
}

Error
pathsuffix(const char *const fname, const char **const suffix)
{
    assert(fname != NULL);
    assert(*fname != '\0');
    assert(strnlen(fname, MAX_FNAME_LEN) < (size_t) MAX_FNAME_LEN);
    assert(suffix != NULL);

    *suffix = strrchr(fname, '.');
    /* cppcheck-suppress misra-c2012-18.4;
       the expression *suffix - 1 can only be reached if *suffix > fname. */
    if (*suffix != NULL && *suffix > fname && *(*suffix - 1) != '/') {
        const char *sep;

        sep = strchr(*suffix, '/');
        if (sep == NULL) {
            return OK;
        }

        do {
            ++sep;
        } while (*sep == '/');

        if (*sep == '\0') {
            return OK;
        }
    }

    return ERR_SUFFIX;
}
