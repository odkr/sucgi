/*
 * User directory handling for suCGI.
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
#include <errno.h>
#include <pwd.h>
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
userdir_expand(const char *const str, const struct passwd *const user,
            const size_t size, size_t *const dirlen, char *const dir)
{
    assert(str != NULL);
    assert(*str != '\0');
    assert(strnlen(str, MAX_FNAME_LEN) < (size_t) MAX_FNAME_LEN);
    assert(size <= (size_t) MAX_FNAME_LEN);
    assert(user != NULL);
    assert(dir != NULL);

    /* Some versions of snprintf fail to null-terminate strings. */
    (void) memset(dir, '\0', size);

    int nchars = -1;

    errno = 0;
    if (*str != '/') {
        /* RATS: ignore; format is short and a literal. */
        nchars = snprintf(dir, size, "%s/%s", user->pw_dir, str);
    } else {
        const char *spec = NULL;
        size_t nspecs = 0U;

        if (str_fmtspecs(str, 1, &nspecs, &spec) != OK) {
            return ERR_BAD;
        }

        if (nspecs == 0U) {
            /* RATS: ignore; format is short and a literal. */
            nchars = snprintf(dir, size, "%s/%s", str, user->pw_name);
        } else if (*spec == 's') {
            /* RATS: ignore; str is verified above. */
            nchars = snprintf(dir, size, str, user->pw_name);
        } else {
            return ERR_BAD;
        }
    }

    if (nchars < 0) {
        return ERR_SYS;
    }

    if ((size_t) nchars >= (size_t) MAX_FNAME_LEN) {
        return ERR_LEN;
    }

    assert((size_t) nchars == strnlen(dir, MAX_FNAME_LEN));
    assert(nchars > 0);
    assert(*dir != '\0');

    if (dirlen != NULL) {
        *dirlen = (size_t) nchars;
    }

    return OK;
}

#if defined(__GNUC__) && __GNUC__ >= 3
#pragma GCC diagnostic pop
#endif
