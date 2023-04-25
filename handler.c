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

#include "handler.h"
#include "params.h"
#include "pair.h"
#include "path.h"
#include "types.h"


Error
handlerfind(const size_t nhandlers, const Pair *const handlerdb,
            const char *const script, const char **const handler)
{
    const char *suffix;
    Error ret;

    assert(handlerdb != NULL);
    assert(script != NULL);
    assert(*script != '\0');
    assert(strnlen(script, MAX_FNAME_LEN) < (size_t) MAX_FNAME_LEN);
    assert(handler != NULL);

    ret = pathsuffix(script, &suffix);
    if (ret != OK) {
        return ret;
    }

    if (strnlen(suffix, MAX_SUFFIX_LEN) >= MAX_SUFFIX_LEN) {
        return ERR_LEN;
    }

    ret = pairfind(nhandlers, handlerdb, suffix, handler);
    if (ret != OK) {
        return ret;
    }

    if (*handler == NULL || **handler == '\0') {
        return ERR_BAD;
    }

    return OK;
}
