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

#if !defined(_FORTIFY_SOURCE)
#define _FORTIFY_SOURCE 3
#endif

#include <assert.h>
#include <pwd.h>
/* cppcheck-suppress misra-c2012-21.6; needed for snprintf. */
#include <stdio.h>
#include <string.h>

#include "max.h"
#include "userdir.h"
#include "types.h"


Error
userdir_resolve(const char *const s, const struct passwd *const user,
                char userdir[MAX_FNAME_LEN])
{
    int n;        /* Length of expanded user directory. */

    assert(s);
    assert(*s != '\0');
    assert(strnlen(s, MAX_FNAME_LEN) < (size_t) MAX_FNAME_LEN);
    assert(user);
    assert(userdir);

    /* Some versions of snprintf fail to NUL-terminate strings. */
    (void) memset(userdir, '\0', MAX_FNAME_LEN);

    if (*s != '/') {
        n = snprintf(userdir, MAX_FNAME_LEN, "%s/%s", user->pw_dir, s);
    } else if (strstr(s, "%s") == NULL) {
        n = snprintf(userdir, MAX_FNAME_LEN, "%s/%s", s, user->pw_name);
    } else {
/* s is not a literal, but can only set by the system administrator. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
        /* RATS: ignore; format is controlled by the administrator. */
        n = snprintf(userdir, MAX_FNAME_LEN, s, user->pw_name);
#pragma GCC diagnostic pop
    }

    if (n < 0) {
        return ERR_SYS_SNPRINTF;
    }
    if ((size_t) n >= (size_t) MAX_FNAME_LEN) {
        return ERR_LEN;
    }

    return OK;
}
