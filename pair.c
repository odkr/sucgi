/*
 * Script handling for suCGI.
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
#include <string.h>

#include "pair.h"
#include "params.h"
#include "types.h"


Error
pair_find(const size_t npairs, const Pair *const pairs,
          const size_t keylen, const char *const key,
          const char **const value)
{
    assert(pairs != NULL);
    assert(key != NULL);
    assert(value != NULL);
    assert(strnlen(key, MAX_STR_LEN) == keylen);
    assert(keylen < MAX_STR_LEN);

    const size_t keysize = keylen + 1U;
    assert(keysize <= (size_t) MAX_STR_LEN);

    for (size_t i = 0; i < npairs; ++i) {
        const Pair *pair = &pairs[i];

        if (pair->key != NULL && strncmp(key, pair->key, keysize) == 0) {
            *value = pair->value;
            return OK;
        }
    }

    return ERR_SEARCH;
}
