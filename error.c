/*
 * Error handling.
 *
 * Copyright 2022 and 2023 Odin Kroeger.
 *
 * This file is part of suCGI.
 *
 * SuCGI is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * SuCGI is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General
 * Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with suCGI. If not, see <https://www.gnu.org/licenses/>.
 */

#define _BSD_SOURCE
#define _DARWIN_C_SOURCE
#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#if defined(__OPTIMIZE__) && !defined(_FORTIFY_SOURCE)
#define _FORTIFY_SOURCE 3
#endif

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "error.h"
#include "params.h"


/* message cannot be a literal. */
#if defined(__GNUC__) && __GNUC__ >= 3
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif

void
error(const char *const message, ...)
{
    va_list argp;

    assert(message != NULL);
    assert(*message != '\0');
    assert(strnlen(message, MAX_ERRMSG_LEN) < MAX_ERRMSG_LEN);

    va_start(argp, message);
    vsyslog(LOG_ERR, message, argp);
    va_end(argp);

    /* cppcheck-suppress misra-c2012-21.8; `error` must terminate. */
    exit(EXIT_FAILURE);
}

#if defined(__GNUC__) && __GNUC__ >= 3
#pragma GCC diagnostic pop
#endif
