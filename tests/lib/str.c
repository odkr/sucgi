/*
 * Utility string functions for testing.
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
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../../macros.h"
#include "str.h"


/*
 * Functions
 */

char *
catstrs(const size_t size, char *const dest, const char *const src)
{
    assert(dest != NULL);
    assert(src != NULL);
    assert(size > 0);

    char *end = stpncpy(dest, src, size);
    if (src[end - dest] != '\0') {
        return NULL;
    }

    return end;
}

char *
idtostr(id_t id)
{
    char *str;
    size_t size;
    int len;

    errno = 0;
    if (ISSIGNED(id_t)) {
        len = snprintf(NULL, 0, PRId64, (int64_t) id);
    } else {
        len = snprintf(NULL, 0, PRIu64, (uint64_t) id);
    }

    if (len < 0) {
        /* Should be unreachable. */
        return NULL;
    }

    size = (size_t) len + 1U;

    errno = 0;
    str = malloc(size);
    if (str == NULL) {
        return NULL;
    }

    (void) memset(str, '\0', size);

    errno = 0;
    if (ISSIGNED(id_t)) {
        len = snprintf(str, size, PRId64, (int64_t) id);
    } else {
        len = snprintf(str, size, PRIu64, (uint64_t) id);
    }

    if (len < 0) {
        /* Should be unreachable. */
        return NULL;
    }

    return str;
}

int
joinstrs(const size_t nstrs, const char *const *const strs,
         const char *const sep, const size_t size, char *const dest)
{
    char *ptr;
    char *lim;

    assert(strs != NULL);
    assert(sep != NULL);
    assert(size > 0);
    assert(dest != NULL);

    ptr = dest;
    lim = dest + size - 1U;

    (void) memset(dest, '\0', size);
    for (size_t i = 0U; i < nstrs && strs[i]; ++i) {
        if (i > 0U) {
            ptr = catstrs((size_t) (lim - ptr), ptr, sep);
            if (ptr == NULL) {
                return -1;
            }
        }

        ptr = catstrs((size_t) (lim - ptr), ptr, strs[i]);
        if (ptr == NULL) {
            return -1;
        }
    }

    return 0;
}

