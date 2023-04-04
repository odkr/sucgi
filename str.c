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

#if defined(__OPTIMIZE__) && !defined(_FORTIFY_SOURCE)
#define _FORTIFY_SOURCE 3
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "max.h"
#include "str.h"
#include "types.h"


Error
str_cp(const size_t nbytes, const char *const src, char *const dest)
{
    const char *end;
    size_t len;

    assert(src != NULL);
    assert(strnlen(src, MAX_STR_LEN) < MAX_STR_LEN);
    assert(dest != NULL);

    end = stpncpy(dest, src, nbytes);
    /* cppcheck-suppress [misra-c2012-10.8, misra-c2012-18.4];
       cast is safe and portable, end must be greater than dest */
    len = (size_t) (end - dest);
    dest[len] = '\0';

    /* dest[len] could only be '\0' only dest was zeroed out. */
    if (src[len] != '\0') {
        return ERR_LEN;
    }

    return OK;
}

Error
str_split(const char *const str, const char *const sep,
          const size_t size, char *const head, const char **const tail)
{
    size_t len;

    assert(str != NULL);
    assert(strnlen(str, MAX_STR_LEN) < MAX_STR_LEN);
    assert(sep != NULL);
    assert(strnlen(sep, MAX_STR_LEN) < MAX_STR_LEN);
    assert(size > 0U);
    assert(head != NULL);
    assert(tail != NULL);

    *tail = strpbrk(str, sep);
    if (*tail == NULL) {
        return str_cp(size - 1U, str, head);
    }

    /* cppcheck-suppress [misra-c2012-10.8, misra-c2012-18.4];
       cast is safe and portable, *tail must be greater than str. */
    len = (size_t) (*tail - str);
    if (len >= size) {
        return ERR_LEN;
    }

    (void) str_cp(len, str, head);
    ++(*tail);

    return OK;
}
