/*
 * Array search and comparison.
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
    const char *tmpdir = NULL;
    char *template = NULL;
    int len = -1;
    int fatalerr;
    long size;

    errno = 0;
    /* RATS: ignore; length of TMPDIR is checked below. */
    tmpdir = getenv("TMPDIR");
    if (tmpdir == NULL || *tmpdir == '\0') {
        /* cppcheck-suppress misra-c2012-22.10; getenv sets errno. */
        if (errno == 0) {
            tmpdir = "/tmp";
        } else  {
            fatalerr = errno;
            goto error;
        }
    }

    if (strnlen(tmpdir, MAX_FNAME_LEN) >= (size_t) MAX_FNAME_LEN) {
        fatalerr = ENAMETOOLONG;
        goto error;
    }

    errno = 0;
    /* RATS: ignore; no race condition. */
    size = pathconf(tmpdir, _PC_PATH_MAX);
    if (size < 0) {
        fatalerr = errno;
        goto error;
    }
    if ((uintmax_t) size > (uintmax_t) SIZE_MAX) {
        fatalerr = ENAMETOOLONG;
        goto error;
    }

    errno = 0;
    /* cppcheck-suppress misra-c2012-11.5; bad advice for malloc. */
    template = malloc((size_t) size);
    if (template == NULL) {
        fatalerr = errno;
        goto error;
    }

    errno = 0;
    len = path_join((size_t) size, template, tmpdir, fname, NULL);
    if (len < 0) {
        fatalerr = errno;
        goto cleanup;
    }

    errno = 0;
    *dir = mkdtemp(template);
    if (*dir == NULL) {
        fatalerr = errno;
        goto cleanup;
    }

    return 0;

    cleanup:
        free(template);

    error:
        errno = fatalerr;
        if (errh != NULL) {
            errh(EXIT_FAILURE, "tmpdir_make");
        }

        return -1;
}
