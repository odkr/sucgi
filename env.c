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

#if !defined(_FORTIFY_SOURCE)
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

#include "env.h"
#include "max.h"
#include "str.h"
#include "types.h"

bool
/* cppcheck-suppress misra-c2012-8.7; external linkage needed for testing. */
env_is_name(const char *s)
{
    assert(s);
    assert(strnlen(s, MAX_VARNAME_LEN) < MAX_VARNAME_LEN);

    if (isdigit(*s) || *s == '\0') {
        return false;
    }

    for (const char *ch = s; *ch != '\0'; ++ch) {
        if (!(isalpha(*ch) || isdigit(*ch) || ('_' == *ch))) {
            return false;
        }
    }

    return true;
}

Error
env_restore(char *const *vars, const size_t n, regex_t pregs[n])
{
    size_t i;

    assert(vars);
    assert(pregs);

    for (i = 0; i < MAX_NVARS && vars[i]; ++i) {
        /* RATS: ignore; str_split respects MAX_VARNAME_LEN. */
        char name[MAX_VARNAME_LEN];
        const char *val;
        const char *var;

        var = vars[i];

        if (strnlen(vars[i], MAX_VAR_LEN) >= (size_t) MAX_VAR_LEN) {
            /* RATS: ignore; format is short and a literal. */
            syslog(LOG_INFO, "variable $%s: too long.", var);
        } else if (str_split(var, "=", MAX_VARNAME_LEN, name, &val) != OK) {
            /* RATS: ignore; format is short and a literal. */
            syslog(LOG_INFO, "variable $%s: name is too long.", var);
        } else if (val == NULL) {
            /* RATS: ignore; format is short and a literal. */
            syslog(LOG_INFO, "variable $%s: malformed.", var);
        } else if (!env_is_name(name)) {
            /* RATS: ignore; format is short and a literal. */
            syslog(LOG_INFO, "variable $%s: bad name.", var);
        } else {
            size_t j;

            for (j = 0; j < n; ++j) {
                if (regexec(&pregs[j], name, 0, NULL, 0) == 0) {
                    errno = 0;
                    if (setenv(name, val, true) != 0) {
                        return ERR_SYS_SETENV;
                    }

                    /* RATS: ignore; format is short and a literal. */
                    syslog(LOG_INFO, "keeping $%s.", name);
                    break;
                }
            }

            if (j == n) {
                /* RATS: ignore; format is short and a literal. */
                syslog(LOG_INFO, "discarding $%s.", name);
            }
        }
    }

    if (i == MAX_NVARS) {
        return ERR_LEN;
    }

    return OK;
}
