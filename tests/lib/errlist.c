/*
 * Lists of errors.
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

#include "../../macros.h"
#include "../../types.h"
#include "errlist.h"


bool
errlistcontains(const ErrList *const haystack, const Error needle)
{
    assert(haystack != NULL);

    for (size_t i = 0; i < haystack->nelems; ++i) {
        if (haystack->elems[i] == needle) {
            return true;
        }
    }

    return false;
}
