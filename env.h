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
 * Check whether STR is a valid environment variable name.
 */
__attribute__((nonnull(1), warn_unused_result))
bool env_is_name(const char *str);

/*
 * Set every variable in VARS the name of which matches one of the regular
 * expressions in PREGS as environment variable. VARS is an array of strings,
 * must be NULL-terminated, and may contain at most MAX_NVARS elements,
 * including the terminating NULL; strings must be of the form <name>=<value>
 * and may be at most MAX_VAR_LEN bytes long, including the terminating NUL;
 * variable names must be ASCII-encoded, non-empty, start with a non-numeric
 * character, consist only of alphanumeric characters or the underscore, and
 * may be at most MAX_VARNAME_LEN - 1 bytes long. PREGS must contain at least
 * NPREGS expressions; supernumery expressions are ignored.
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
Error env_restore(char *const *vars, size_t npregs, const regex_t *pregs);


#endif /* !defined(ENV_H) */
