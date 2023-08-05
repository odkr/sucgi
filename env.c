/*
 * Access the environment.
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
#include <ctype.h>
#include <errno.h>
#include <fnmatch.h>
#include <regex.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include "env.h"
#include "params.h"
#include "str.h"
#include "types.h"


Error
env_copy_var(const char *const name, const size_t maxlen,
             size_t *const len, char *const value)
{
    const char *var;
    Error ret;

    assert(name != NULL);
    assert(*name != '\0');
    assert(strchr(name, '=') == NULL);
    assert(strnlen(name, MAX_VARNAME_LEN) < MAX_VARNAME_LEN);
    assert(value != NULL);

    /* Return an empty string if an error occurs. */
    (void) memset(value, '\0', maxlen);

    errno = 0;
    /* RATS: ignore; CGI requires reading environment variables. */
    var = getenv(name);
    if (var == NULL) {
        /* cppcheck-suppress misra-c2012-22.10; getenv sets errno. */
        if (errno == 0) {
            return ERR_SEARCH;
        }

        return ERR_SYS;
    }

    ret = str_copy(maxlen, var, len, value);
    if (ret != OK) {
        return ret;
    }

    assert(value != NULL);
    assert(strnlen(value, MAX_VAR_LEN) == *len);
    assert(*len < MAX_VAR_LEN);
    assert(value[*len] == '\0');

    return ret;
}





/* FIXME: better take a string as arg! makes it testable */
Error
env_init(const char *const vars)
{
    for (const char *ptr = vars; ptr != NULL; ) {
        char var[MAX_VAR_LEN];
        char name[MAX_VARNAME_LEN];
        const char *value;

        if (str_split(ptr, " ", sizeof(var), var, &ptr) != OK) {
            /* RATS: ignore; format is short and a literal. */
            syslog(LOG_INFO, "discarding variable that is too long.");
        } else if (str_split(var, "=", sizeof(name), name, &value) != OK) {
            /* RATS: ignore; format is short and a literal. */
            syslog(LOG_INFO, "discarding variable with too long a name.");
        } else if (!env_is_name(name)) {
            /* RATS: ignore; format is short and a literal. */
            syslog(LOG_INFO, "$%s: bad name.", name);
        } else if (value == NULL) {
            /* RATS: ignore; format is short and a literal. */
            syslog(LOG_INFO, "$%s: no value.", name);
        } else {
            errno = 0;
            if (setenv(name, value, true) != 0) {
                return ERR_SYS;
            }
        }
    }

    return OK;
}

bool
env_is_name(const char *const str)
{
    assert(str != NULL);
    assert(strnlen(str, MAX_VARNAME_LEN) < MAX_VARNAME_LEN);

    if (isdigit(*str) || *str == '\0') {
        return false;
    }

    for (const char *chr = str; *chr != '\0'; ++chr) {
        if (!isalpha(*chr) && !isdigit(*chr) && *chr != '_') {
            return false;
        }
    }

    return true;
}

Error
env_restore(const char *const *const vars, const size_t npregs,
            const regex_t *const pregs)
{
    size_t varidx;

    assert(vars != NULL);
    assert(pregs != NULL);

    for (varidx = 0; vars[varidx] != NULL; ++varidx) {
        /* RATS: ignore; str_split respects MAX_VARNAME_LEN. */
        char name[MAX_VARNAME_LEN];
        const char *value;
        const char *var;

        var = vars[varidx];

        if (strnlen(var, MAX_VAR_LEN) >= (size_t) MAX_VAR_LEN) {
            /* RATS: ignore; format is short and a literal. */
            syslog(LOG_INFO, "discarding variable that is too long.");
        } else if (str_split(var, "=", sizeof(name), name, &value) != OK) {
            /* RATS: ignore; format is short and a literal. */
            syslog(LOG_INFO, "discarding variable with too long a name.");
        } else if (!env_is_name(name)) {
            /* RATS: ignore; format is short and a literal. */
            syslog(LOG_INFO, "$%s: bad name.", name);
        } else if (value == NULL) {
            /* RATS: ignore; format is short and a literal. */
            syslog(LOG_INFO, "$%s: no value.", name);
        } else {
            size_t pregidx;

            for (pregidx = 0; pregidx < npregs; ++pregidx) {
                if (regexec(&pregs[pregidx], name, 0, NULL, 0) == 0) {
                    errno = 0;
                    if (setenv(name, value, true) != 0) {
                        return ERR_SYS;
                    }

                    /* RATS: ignore; format is short and a literal. */
                    syslog(LOG_INFO, "keeping $%s.", name);
                    break;
                }
            }

            if (pregidx >= npregs) {
                /* RATS: ignore; format is short and a literal. */
                syslog(LOG_INFO, "discarding $%s.", name);
            }
        }
    }

    if (varidx >= (size_t) MAX_NVARS) {
        return ERR_LEN;
    }

    return OK;
}
