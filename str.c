/*
 * Manipulate strings safely.
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

#if !defined(_FORTIFY_SOURCE)
#define _FORTIFY_SOURCE 3
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "max.h"
#include "str.h"
#include "types.h"


Error
str_cp(const size_t n, const char *const src, char dest[n + 1U])
{
    char *end;      /* Position of last byte of src. */
    size_t len;     /* Bytes copied. */

    assert(src);
    assert(strnlen(src, MAX_STR_LEN) < MAX_STR_LEN);
    assert(dest);

    end = stpncpy(dest, src, n);
    /* cppcheck-suppress [misra-c2012-10.8, misra-c2012-18.4];
       cast is safe and portable, end must be greater than dest */
    len = (size_t) (end - dest);
    dest[len] = '\0';

    /* dest[len] could be '\0' only because dest was zeroed out. */
    if (src[len] != '\0') {
        return ERR_LEN;
    }

    return OK;
}

Error
str_split(const char *const str, const char *const sep,
          const size_t n, char head[n], const char **const tail)
{
    size_t len;     /* Length of head. */

    assert(str);
    assert(strnlen(str, MAX_STR_LEN) < MAX_STR_LEN);
    assert(sep);
    assert(strnlen(sep, MAX_STR_LEN) < MAX_STR_LEN);
    assert(n > 0U);
    assert(head);
    assert(tail);

    *tail = strpbrk(str, sep);
    if (*tail == NULL) {
        return str_cp(n - 1U, str, head);
    }

    /* cppcheck-suppress [misra-c2012-10.8, misra-c2012-18.4];
       cast is safe and portable, *tail must be greater than str. */
    len = (size_t) (*tail - str);
    if (len >= n) {
        return ERR_LEN;
    }

    (void) str_cp(len, str, head);
    ++(*tail);

    return OK;
}
