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

#if !defined(_FORTIFY_SOURCE)
#define _FORTIFY_SOURCE 3
#endif

#include <assert.h>
#include <string.h>

#include "handler.h"
#include "max.h"
#include "path.h"
#include "types.h"

Error
handler_lookup(const size_t nelems, const Pair *const handlerdb,
               const char *const script, const char **const handler)
{
    const char *suffix;

    assert(handlerdb);
    assert(script);
    assert(strnlen(script, MAX_FNAME_LEN) < (size_t) MAX_FNAME_LEN);
    assert(*script != '\0');
    assert(handler);

    if (path_suffix(script, &suffix) == OK) {
        size_t len;

        len = strnlen(suffix, MAX_SUFFIX_LEN);
        if (len >= MAX_SUFFIX_LEN) {
            return ERR_LEN;
        }

        for (size_t i = 0; i < nelems; ++i) {
            const Pair *ent = &handlerdb[i];

            if (strncmp(suffix, ent->key, len + 1U) == 0) {
                if (ent->value == NULL || *ent->value == '\0') {
                    return ERR_BAD;
                }

                *handler = ent->value;
                return OK;
            }
        }
    }

    return ERR_SEARCH;
}
