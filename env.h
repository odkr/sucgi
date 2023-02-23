/*
 * Header file for env.c.
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

#if !defined(ENV_H)
#define ENV_H

#include <sys/types.h>
#include <regex.h>
#include <stdbool.h>

#include "cattr.h"
#include "max.h"
#include "types.h"


/*
 * Globals
 */

/* The environment. */
extern char **environ;


/*
 * Functions
 */

/*
 * Check whether S is a valid environment variable name.
 */
__attribute__((nonnull(1), warn_unused_result))
bool env_is_name(const char *s);

/*
 * Set every variable in VARS the name of which matches any of the N regular
 * expressions in PREGS as environment variable. VARS is a NULL-terminated
 * array of variables and may contain at most MAX_NVARS elements. Variables
 * are strings of the form <name>=<value> and may be at most MAX_VAR_LEN
 * bytes long, including the terminating NUL. Moreover, variable names must
 * be ASCII-encoded, non-empty, start with a non-numeric character, comprise
 * alphanumeric characters and the underscore only, and may be at most
 * MAX_VARNAME_LEN bytes long. PREGS must contain at least N expressions;
 * supernumery expressions are ignored.
 *
 * Note, an attacker may populate the environment with variables that are
 * not NUL-terminated. If the memory area after such a variable contains
 * a NUL within MAX_VAR_LEN - 1 bytes, env_restore will overshoot and dump
 * the contents of that memory region into the environment.
 *
 * Return value:
 *     OK               Success.
 *     ERR_LEN          Too many environment variables.
 *     ERR_SYS_SETENV   setenv failed.
 *
 * Side-effects:
 *     Logs which variables are kept and which are discarded.
 */
__attribute__((nonnull(1, 3), warn_unused_result))
/* cppcheck-suppress misra-c2012-8.2; declaration is in prototype form. */
Error env_restore(char *const *vars, size_t n, regex_t pregs[n]);


#endif /* !defined(ENV_H) */
