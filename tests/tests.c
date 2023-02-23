/*
 * Utiltiy functions for tests.
 *
 * Copyright 2022 and 2023 Odin Kroeger
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

#include <assert.h>
#include <math.h>
#include <string.h>

#include "tests.h"


int
cat_strs(char *dest, const char *const src, const char *const lim, char **end)
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
join_strs(size_t n, const char *const strs[n], const char *sep,
          size_t size, char dest[size])
{
    char *ptr;		/* Pointer to current position in dest. */
    char *lim;		/* Limit past which must not be written. */

    assert(strs != NULL);
    assert(sep != NULL);
    assert(dest != NULL);

    ptr = dest;
    lim = dest + size - 1U;

    (void) memset(dest, '\0', size);
    for (size_t i = 0U; i < n && strs[i]; ++i) {
        if (i > 0U) {
            if (cat_strs(ptr, sep, lim, &ptr) != 0) {
                return 1;
            }
        }

        if (cat_strs(ptr, strs[i], lim, &ptr) != 0) {
            return 1;
        }
    }

    return 0;
}

void
fill_str(const char ch, const size_t n, char dest[n])
{
    assert(n > 0);
    assert(dest != NULL);

    size_t len;

    len = n - 1U;
    (void) memset(dest, ch, len);
    dest[len] = '\0';
}

#include <err.h>

int
to_str(unsigned int num, unsigned int base, const char digits[base],
       size_t size, char dest[size])
{
    size_t len;         /* Number of digits needed. */

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
        unsigned int rem;	/* Remainder. */

        rem = num % base;
        num = num / base;

        *ptr = digits[rem];
    }

    return 0;
}
