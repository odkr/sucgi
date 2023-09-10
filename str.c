/*
 * Manipulate strings safely.
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

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "params.h"
#include "str.h"
#include "types.h"


Error
str_copy(const size_t nbytes, const char *const src,
         size_t *const destlen, char *const dest)
{
    assert(src != NULL);
    assert(nbytes <= MAX_STR_LEN);
    assert(strnlen(src, MAX_STR_LEN) < MAX_STR_LEN);
    assert(dest != NULL);

    const char *end = stpncpy(dest, src, nbytes);
    assert(end != NULL);
    assert(end >= dest);

    /* cppcheck-suppress misra-c2012-18.4; safe pointer subtraction. */
    ptrdiff_t len = end - dest;
    assert(len >= 0);
    /* nbytes must be smaller than SIZE_MAX. */
    assert((uintmax_t) len <= (uintmax_t) nbytes);

    /* Null-terminate regardless of the return value. */
    dest[len] = '\0';

    if (destlen != NULL) {
        *destlen = (size_t) len;
    }

    assert(strnlen(dest, nbytes) == (size_t) len);
    assert(strnlen(src, MAX_STR_LEN) >= (size_t) len);
    assert(strncmp(src, dest, nbytes) == 0);

    if (src[len] != '\0') {
        return ERR_LEN;
    }

    return OK;
}

Error
str_fmtspecs(const char *const str, const size_t maxnspecs,
            size_t *const nspecs, const char **const fmtchars)
{
    assert(str != NULL);
    assert(strnlen(str, MAX_STR_LEN) < MAX_STR_LEN);
    assert(nspecs != NULL);
    assert(fmtchars != NULL);

    *nspecs = 0;

    const char *fmtchar = NULL;
    for (const char *ptr = strchr(str, '%'); ptr != NULL; ptr = strchr(fmtchar, '%')) {
        size_t npercents;

        npercents = strspn(ptr, "%");
        fmtchar = &ptr[npercents];

        /* If the number of '%' signs is even, they are literal '%' signs. */
        if (npercents % 2U != 0U) {
            if (*nspecs >= maxnspecs) {
                return ERR_LEN;
            }

            fmtchars[*nspecs] = fmtchar;
            ++*nspecs;
        }
    }

    return OK;
}

Error
str_split(const char *const str, const char *const sep,
          const size_t size, char *const head, const char **const tail)
{
    assert(str != NULL);
    assert(size <= MAX_STR_LEN);
    assert(strnlen(str, MAX_STR_LEN) < MAX_STR_LEN);
    assert(sep != NULL);
    assert(strnlen(sep, MAX_STR_LEN) < MAX_STR_LEN);
    assert(size > 0U);
    assert(head != NULL);
    assert(tail != NULL);

    /* Make sure the head is filled with NULs. */
    (void) memset(head, '\0', size);

    *tail = strpbrk(str, sep);
    if (*tail == NULL) {
        return str_copy(size - 1U, str, NULL, head);
    }

    assert(*tail >= str);

    /* cppcheck-suppress misra-c2012-18.4; safe pointer subtraction. */
    ptrdiff_t len = *tail - str;
    assert(len >= 0);
    assert((uintmax_t) len < (uintmax_t) SIZE_MAX);

    if ((size_t) len >= size) {
        return ERR_LEN;
    }

    (void) str_copy((size_t) len, str, NULL, head);
    ++(*tail);

    return OK;
}
