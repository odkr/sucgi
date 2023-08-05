/*
 * Utility string functions for testing.
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
#include <string.h>

#include "str.h"


int
str_cmp_ptrs(const char *const *const str1, const char *const *const str2)
{
    assert(str1 != NULL);
    assert(str2 != NULL);

    if (*str1 == *str2) {
        return 0;
    }

    if (*str1 == NULL || *str2 == NULL) {
        return -1;
    }

    return strcmp(*str1, *str2);
}

