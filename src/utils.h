/*
 * Headers for utils.c.
 *
 * Copyright 2022 Odin Kroeger
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

#if !defined(SRC_UTILS_H)
#define SRC_UTILS_H

#include <pwd.h>

#include "attr.h"


/*
 * Data types
 */

/* A simple key-value store. */
struct pair {
        char *key;
        char *value;
};


/*
 * Functions
 */

/*
 * Assume the UID and GID of user.
 *
 * Aborts the programme if an error occurred.
 */
__attribute__((RO(1)))
void drop_privs(struct passwd *user);

/*
 * Run script with the first matching interpreter in pairs.
 *
 * pairs is an array of filename ending-interpreter pairs.
 *
 * Filename endings must be given including the leading dot (".").
 * Interpreters are searched for in $PATH.
 *
 * The first interpreter that is associated with the script's filename ending
 * is executed with execlp(3) and given the script as first and only argument.
 * 
 * run_script never returns.
 */
__attribute__((RO(1), RO(2)))
void run_script(const char *const script, struct pair pairs[]);

#endif /* !defined(SRC_UTILS_H) */
