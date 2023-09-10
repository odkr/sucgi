/*
 * Array search and comparison.
 *
 * Copyright 2023 Odin Kroeger.
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

/* macOS doesn't export mkdtemp unless _DARWIN_C_SOURCE is set. */
#define _BSD_SOURCE
#define _DARWIN_C_SOURCE
#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../params.h"
#include "path.h"
#include "tmpdir.h"
#include "types.h"

int
tmpdir_make(const char *const fname, char **const dir, ErrorFn errh)
{
    /* cppcheck-suppress unreadVariable; RAII. */
    int fatalerr = 0;

    errno = 0;
    /* RATS: ignore; length of TMPDIR is checked below. */
    const char *tmpdir = getenv("TMPDIR");
    if (tmpdir == NULL || *tmpdir == '\0') {
        /* cppcheck-suppress misra-c2012-22.10; getenv sets errno. */
        if (errno == 0) {
            tmpdir = "/tmp";
        } else  {
            fatalerr = errno;
            goto error;
        }
    }

    /* RATS: ignore; path_join is bounded by the size of the template. */
    char template[MAX_FNAME_LEN];
    errno = 0;
    const int len = path_join(sizeof(template), template, tmpdir, fname, NULL);
    if (len < 0) {
        fatalerr = errno;
        goto error;
    }

    errno = 0;
    /* RATS: ignore; no race condition. */
    const long size = pathconf(tmpdir, _PC_PATH_MAX);
    if (size < 0) {
        fatalerr = errno;
        goto error;
    }

    if (len > size) {
        fatalerr = ENAMETOOLONG;
        goto error;
    }

    errno = 0;
    char *const symdir = mkdtemp(template);
    if (symdir == NULL) {
        fatalerr = errno;
        goto error;
    }

    /* RATS: ignore; symdir cannot be longer than MAX_FNAME_LEN. */
    *dir = realpath(symdir, NULL);
    if (*dir == NULL) {
        fatalerr = errno;
        goto error;
    }

    return 0;

    error:
        errno = fatalerr;
        if (errh != NULL) {
            errh(EXIT_FAILURE, "tmpdir_make");
        }

        return -1;
}
