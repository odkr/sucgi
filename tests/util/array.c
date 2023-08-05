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

#define _XOPEN_SOURCE 700

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "array.h"


/*
 * lfind does not accept a pointer to a const-qualified lvalue as "nelp"
 * argument when compiled with -Wcast-qual; array_find does.
 */
const void *
array_find(const void *const needle, const void *const haystack,
           const size_t len, const size_t wd, const CompFn cmp)
{
    /* cppcheck-suppress misra-c2012-11.5; alignment is not an issue here. */
    const char *ptr = (const char *) haystack;

    assert(needle != NULL);
    assert(haystack != NULL);
    assert(wd > 0U);

    for (size_t i = 0U; i < len; ++i) {
        if (cmp(needle, (const void *) ptr) == 0) {
            return ptr;
        }

        /*
         * If len or wd are too large, this addition is undefined behaviour.
         * But as long as they are of the right size, this is correct.
         */
        ptr += wd; /* cppcheck-suppress [misra-c2012-10.3, misra-c2012-18.4] */
    }

    return NULL;
}

bool
array_is_sub(const void *const sub,
             const size_t sublen, const size_t subwd,
             const void *const super,
             const size_t superlen, const size_t superwd,
             const CompFn cmp)
{
    /* cppcheck-suppress misra-c2012-11.5; alignment is not an issue here. */
    const char *ptr = (const char *) sub;

    assert(sub != NULL);
    assert(subwd > 0U);
    assert(super != NULL);
    assert(superwd > 0U);

    for (size_t i = 0U; i < sublen; ++i) {
        const void *elem;

        elem = array_find(ptr, super, superlen, superwd, cmp);
        if (elem == NULL) {
            return false;
        }

        /* cppcheck-suppress [misra-c2012-10.3, misra-c2012-18.4] */
        ptr += superwd;
    }

    return true;
}

bool
array_eq(const void *const one, const size_t onelen, const size_t onewd,
         const void *const two, const size_t twolen, const size_t twowd,
         const CompFn cmp)
{
    assert(one != NULL);
    assert(onewd > 0U);
    assert(two != NULL);
    assert(twowd > 0U);

    return array_is_sub(one, onelen, onewd, two, twolen, twowd, cmp) &&
           array_is_sub(two, twolen, twowd, one, onelen, onewd, cmp);
}
