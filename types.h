/*
 * Data types.
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

#if !defined(TYPES_H)
#define TYPES_H

/* Return codes. */
typedef enum {
    OK = 0,             /* Success. */
    ERR_SYS,            /* System error. */
    ERR_BAD,            /* Bad input. */
    ERR_BASEDIR,        /* File is outside of base directory. */
    ERR_LEN,            /* An array or a string is too long. */
    ERR_PRIV,           /* Privileges could be resumed. */
    ERR_SEARCH,         /* Something was not found. */
    ERR_SUFFIX          /* Filename has no suffix. */
} Error;

/* Simple key-value store. */
typedef struct {
    const char *const key;      /* cppcheck-suppress unusedStructMember */
    const char *const value;    /* cppcheck-suppress unusedStructMember */
} Pair;

#endif /* !defined(TYPES_H) */
