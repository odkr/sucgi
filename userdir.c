/*
 * User directory handling for suCGI.
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
#include <errno.h>
#include <pwd.h>
/* cppcheck-suppress misra-c2012-21.6; needed for snprintf. */
#include <stdio.h>
#include <string.h>

#include "params.h"
#include "userdir.h"
#include "str.h"
#include "types.h"


/*
 * Only one format string is not a literal and that
 * string can only be set by the system administrator.
 */
#if defined(__GNUC__) && __GNUC__ >= 3
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif

Error
userdirexp(const char *const str, const struct passwd *const user,
           char dir[MAX_FNAME_LEN])
{
    int nchars;

    assert(str != NULL);
    assert(*str != '\0');
    assert(strnlen(str, MAX_FNAME_LEN) < (size_t) MAX_FNAME_LEN);
    assert(user != NULL);
    assert(dir != NULL);

    /* Some versions of snprintf fail to NUL-terminate strings. */
    (void) memset(dir, '\0', MAX_FNAME_LEN);

    errno = 0;
    if (*str != '/') {
        /* RATS: ignore; format is short and a literal. */
        nchars = snprintf(dir, MAX_FNAME_LEN, "%s/%s", user->pw_dir, str);
    } else {
        const char *spec;
        size_t nspecs;

        if (getspecstrs(str, 1, &nspecs, &spec) != OK) {
            return ERR_BAD;
        }

        if (nspecs == 0U) {
            /* RATS: ignore; format is short and a literal. */
            nchars = snprintf(dir, MAX_FNAME_LEN, "%s/%s", str, user->pw_name);
        } else if (*spec == 's') {
            /* RATS: ignore; str is verified above. */
            nchars = snprintf(dir, MAX_FNAME_LEN, str, user->pw_name);
        } else {
            return ERR_BAD;
        }

    }

    if (nchars < 0) {
        /* Should only be reachable if str contains an invalid wchar. */
        return ERR_SYS;
    }
    if ((size_t) nchars >= (size_t) MAX_FNAME_LEN) {
        return ERR_LEN;
    }

    return OK;
}

#if defined(__GNUC__) && __GNUC__ >= 3
#pragma GCC diagnostic pop
#endif
