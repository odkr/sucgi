/*
 * Header file for env.c.
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

#if !defined(ENV_H)
#define ENV_H

#include <sys/types.h>
#include <regex.h>
#include <stdbool.h>

#include "attr.h"
#include "params.h"
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
 * Copy each byte of the given environment variable to the memory area the
 * parameter "value" points to, but stop when a null byte is encountered or
 * the the given maximum length has been reached, then terminate the copied
 * value with a null byte and return the length of the value in the variable
 * pointed to by "len".
 *
 * The given memory area must be large enough to hold the result; that is,
 * it must be at least one byte larger than the given number of bytes.
 *
 * Return value:
 *     OK          Success.
 *     ERR_LEN     The variable is longer than the given number of bytes.
 *     ERR_SEARCH  There is no variable with the given name.
 *     ERR_SYS     getenv failed.
 */
_read_only(1) _write_only(3) _write_only(4, 2) _nonnull(1, 3, 4) _nodiscard
Error env_copy_var(const char *name, size_t maxlen, size_t *len, char *value);

/* FIXME: Untested, undocumented. */
_nodiscard
Error env_init(void);

/*
 * Check whether the given string is ASCII-encoded, non-empty, starts with
 * a non-numeric character, and comprises only alphanumeric characters and
 * the underscore.
 */
_read_only(1) _nonnull(1) _nodiscard
bool env_is_name(const char *str);

/*
 * Set each of the given environment variables that matches one of the given
 * POSIX extended regular expressions. The array of variables must contain at
 * most MAX_NVARS elements and must be terminated by a null pointer. Variables
 * may at most be MAX_VAR_LEN bytes long, including the terminating null byte;
 * variable names must pass env_is_name and be at most MAX_VAR_LEN bytes
 * long, including the null terminator. At least "npregs" regular expressions
 * must be given; supernumery expressions are ignored.
 *
 * Note, an attacker may populate the environment with variables that are
 * not null-terminated. If the memory area after such a variable contains
 * a null byte within MAX_VAR_LEN - 1 bytes, env_restore will overshoot
 * and dump the contents of that memory area into the environment.
 *
 * Return value:
 *     OK        Success.
 *     ERR_LEN   There are more than MAX_NVARS variables.
 *     ERR_SYS   setenv failed.
 *
 * Side-effects:
 *     Logs which variables are kept and which are discarded.
 */
_read_only(1) _read_only(3, 2) _nonnull(1, 3) _nodiscard
Error env_restore(const char *const *vars,
                  size_t npregs, const regex_t *pregs);

/* FIXME: untested, undocumented. */
_read_only(1) _nonnull(1) _nodiscard
Error env_set_vars(const char *vars);

#endif /* !defined(ENV_H) */
