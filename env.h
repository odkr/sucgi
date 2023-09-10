/*
 * Header file for env.c.
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
 * Copy the given environment variable to the memory area pointed to by
 * the parameter "buf" and return the number of bytes copied, excluding
 * the terminating null byte, in the variable pointed to by "len".
 *
 * Return value:
 *      OK          Success.
 *      ERR_LEN     The variable is larger than the buffer.
 *      ERR_SEARCH  There is no variable of the given name.
 *      ERR_SYS     getenv failed.
 */
_read_only(1) _write_only(3, 2) _write_only(4) _nonnull(1, 3, 4) _nodiscard
Error env_get(const char *name, size_t bufsize, char *buf, size_t *len);

/*
 * Set up the minimal conforming environment.
 *
 * Return value:
 *      See env_setn.
 */
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
 * Set each given name=value pair the name of which matches one of the
 * given POSIX extended regular expressions as environment variable.
 *
 * "vars" must be terminated by a null pointer. "pregs" must contain at
 * least "npregs" expressions; supernumery expressions are ignored.
 *
 * If a variable name is longer than MAX_VARNAME_LEN, does not pass
 * env_is_name, or does not match any of the given regular expressions,
 * or if the variable as a whole, including the null terminator, is
 * longer than MAX_VAR_LEN, then the variable is ignored.
 *
 * Note, if a variable is NOT null-terminated but there is a null byte
 * within MAX_VAR_LEN - 1 bytes after its starting address, then env_restore
 * will either segfault or overshoot and dump the contents of that memory
 * area up to the address of the null byte into the environment.
 *
 * Return value:
 *      OK          Success.
 *      ERR_LEN     "vars" contains more than MAX_NVARS variables.
 *      ERR_SYS     setenv failed.
 *
 * Side-effects:
 *      Logs which variables are kept and which are discarded.
 */
_read_only(1) _read_only(3, 2) _nonnull(1, 3) _nodiscard
Error env_restore(const char *const *vars,
                  size_t npregs, const regex_t *pregs);

/*
 * Take a space-separated list of name=value pairs and
 * set each of them as environment variable.
 *
 * Return value:
 *      OK          Success.
 *      ERR_BAD     Malformed variables given.
 *      ERR_LEN     More than MAX_NVARS variables were given.
 *      ERR_SYS     setenv failed.
 */
_read_only(1) _nonnull(1) _nodiscard
Error env_setn(const char *vars);

#endif /* !defined(ENV_H) */
