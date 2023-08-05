/*
 * Path handling.
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

#include "params.h"
#include "path.h"
#include "str.h"
#include "types.h"

Error
path_real(const size_t fnamelen, const char *const fname,
          size_t *const reallen, char **const real)
{
    size_t len;

    assert(fname != NULL);
    assert(*fname != '\0');
    assert(strnlen(fname, MAX_FNAME_LEN) == fnamelen);
    assert(PATH_MAX == -1 || MAX_FNAME_LEN <= PATH_MAX);
    assert(real != NULL);

    *real = NULL;

    if (fnamelen >= (size_t) MAX_FNAME_LEN) {
        return ERR_LEN;
    }

    errno = 0;
    /* RATS: ignore; length of input is checked. */
    *real = realpath(fname, NULL);
    if (*real == NULL) {
        return ERR_SYS;
    }

    len = strnlen(*real, MAX_FNAME_LEN);
    if (reallen != NULL) {
        *reallen = len;
    }

    if (len >= (size_t) MAX_FNAME_LEN) {
        return ERR_LEN;
    }

    assert(len > 0U);

    return OK;
}

Error
path_suffix(const char *const fname, const char **const suffix)
{
    const char *pathsep = NULL;

    assert(fname != NULL);
    assert(*fname != '\0');
    assert(strnlen(fname, MAX_FNAME_LEN) < (size_t) MAX_FNAME_LEN);
    assert(suffix != NULL);

    *suffix = strrchr(fname, '.');
    if (*suffix == NULL) {
        return ERR_SUFFIX;
    }

    if (*suffix <= fname) {
        return ERR_SUFFIX;
    }

    /* cppcheck-suppress misra-c2012-18.4; only reached if *suffix > fname. */
    if (*(*suffix - 1U) == '/') {
        return ERR_SUFFIX;
    }

    pathsep = strchr(*suffix, '/');
    if (pathsep == NULL) {
        return OK;
    }

    if (pathsep[strspn(pathsep, "/")] == '\0') {
        return OK;
    }

    return ERR_SUFFIX;
}

bool
path_within(const size_t fnamelen, const char *const fname,
            const size_t basedirlen, const char *const basedir)
{
    assert(basedir != NULL);
    assert(*basedir != '\0');
    assert(strnlen(basedir, MAX_FNAME_LEN) == basedirlen);
    assert(basedirlen < (size_t) MAX_FNAME_LEN);
    assert(fname != NULL);
    assert(*fname != '\0');
    assert(strnlen(fname, MAX_FNAME_LEN) == fnamelen);
    assert(fnamelen < (size_t) MAX_FNAME_LEN);

    if (strncmp(fname, "/", sizeof("/")) == 0) {
        return false;
    }

    if (strncmp(fname, ".", sizeof(".")) == 0) {
        return false;
    }

    if (*fname == '/') {
        if (strncmp(basedir, "/", sizeof("/")) == 0) {
            return true;
        }
    } else {
        if (strncmp(basedir, ".", sizeof(".")) == 0) {
            return true;
        }
    }

    if (fnamelen <= basedirlen) {
        return false;
    }

    if (fname[basedirlen] != '/') {
        return false;
    }

    if (strncmp(basedir, fname, basedirlen) != 0) {
        return false;
    }

    assert(strstr(fname, basedir) != NULL);

    return true;
}
