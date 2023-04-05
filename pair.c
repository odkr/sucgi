/*
 * Script handling for suCGI.
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
#include <string.h>

#include "pair.h"
#include "max.h"
#include "types.h"


Error
pairfind(const size_t npairs, const Pair *const pairs,
            const char *const key, const char **const value)
{
    size_t len = strnlen(key, MAX_STR_LEN);

    assert(pairs != NULL);
    assert(key != NULL);
    assert(len < MAX_STR_LEN);
    assert(value != NULL);

    for (size_t i = 0; i < npairs; ++i) {
        const Pair *pair = &pairs[i];

        if (strncmp(key, pair->key, len + 1U) == 0) {
            *value = pair->value;
            return OK;
        }
    }

    return ERR_SEARCH;
}
