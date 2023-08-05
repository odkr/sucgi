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

#include "handler.h"
#include "params.h"
#include "pair.h"
#include "path.h"
#include "types.h"

#include <err.h>
Error
handler_find(const size_t nhandlers, const Pair *const handlerdb,
             const size_t fnamelen, const char *const fname,
             const char **const handler)
{
    const char *suffix;
    size_t basenamelen;
    size_t suffixlen;
    Error ret;

    assert(handlerdb != NULL);
    assert(fname != NULL);
    assert(*fname != '\0');
    assert(strnlen(fname, MAX_FNAME_LEN) < (size_t) MAX_FNAME_LEN);
    assert(handler != NULL);

    *handler = NULL;

    ret = path_suffix(fname, &suffix);
    if (ret != OK) {
        return ret;
    }

    assert(suffix != NULL);
    assert(suffix >= fname);

    /*
     * The cast is safe and portable.
     * And MISRA C permits pointer subtraction.
     */
    basenamelen = (size_t)          /* cppcheck-suppress misra-c2012-10.8 */
                  (suffix - fname); /* cppcheck-suppress misra-c2012-18.4 */

    assert(fnamelen > basenamelen);

    suffixlen = fnamelen - basenamelen;

    assert(strnlen(suffix, MAX_STR_LEN) == suffixlen);

    if (suffixlen >= MAX_SUFFIX_LEN) {
        return ERR_LEN;
    }

    ret = pair_find(nhandlers, handlerdb, suffixlen, suffix, handler);
    if (ret != OK) {
        return ret;
    }

    if (*handler == NULL || **handler == '\0') {
        return ERR_BAD;
    }

    return OK;
}
