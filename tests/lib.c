/*
 * Utiltiy functions for tests.
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

#define _BSD_SOURCE
#define _DARWIN_C_SOURCE
#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <assert.h>
#include <math.h>
#include <string.h>

#include "lib.h"


int
catstrs(char *dest, const char *const src, const char *const lim, char **end)
{
    assert(dest != NULL);
    assert(src != NULL);
    assert(lim != NULL);
    assert(lim > dest);
    assert(end != NULL);

    *end = stpncpy(dest, src, (size_t) (lim - dest));
    if (src[*end - dest] != '\0') {
        return 1;
    }

    return 0;
}

int
joinstrs(size_t n, const char *const *const strs, const char *sep,
         size_t size, char *const dest)
{
    char *ptr;
    char *lim;

    assert(strs != NULL);
    assert(sep != NULL);
    assert(dest != NULL);

    ptr = dest;
    lim = dest + size - 1U;

    (void) memset(dest, '\0', size);
    for (size_t i = 0U; i < n && strs[i]; ++i) {
        if (i > 0U) {
            if (catstrs(ptr, sep, lim, &ptr) != 0) {
                return 1;
            }
        }

        if (catstrs(ptr, strs[i], lim, &ptr) != 0) {
            return 1;
        }
    }

    return 0;
}

void
fillstr(const char ch, const size_t n, char *const dest)
{
    assert(n > 0);
    assert(dest != NULL);

    size_t len;

    len = n - 1U;
    (void) memset(dest, ch, len);
    dest[len] = '\0';
}

int
tostr(unsigned int num, unsigned int base, const char *const digits,
      size_t size, char *const dest)
{
    size_t len;

    assert(base > 0);
    assert(digits != NULL);
    assert(size > 0);
    assert(dest);

    len = (size_t) (log(fmax(num, 1)) / log(base) + 1);
    if (len + 1U > size) {
        return 1;
    }

    (void) memset(dest + len, '\0', size - len);
    for (char *ptr = dest + len - 1U; ptr >= dest; --ptr) {
        unsigned int rem;

        rem = num % base;
        num = num / base;

        *ptr = digits[rem];
    }

    return 0;
}
