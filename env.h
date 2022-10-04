/*
 * Headers for env.c.
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

#if !defined(ENV_H)
#define ENV_H

#include <sys/stat.h>

#include "defs.h"
#include "err.h"
#include "str.h"


/*
 * Constants
 */

/* Maximum number of environment variables. */
#define ENV_MAX 192



/*
 * Globals
 */

/* The environment. */
extern char **environ;

/*
 * Environment variables to keep.
 * NULL-terminated array of shell wildcard patterns.
 */
extern const char *const env_safe_vars[];


/*
 * Functions
 */

/*
 * Clear the environment and store a copy of the old environment in VARS.
 * If VARS is NULL, the old environment is discarded. VARS must point to
 * a memory area that is large enough to hold ENV_MAX variables.
 *
 * Return code:
 *      OK           Success.
 *      ERR_ENV_MAX  There are more than ENV_MAX environment variables.
 */
__attribute__((warn_unused_result))
enum error env_clear(/* RATS: ignore; vars is bound-checked. */
	             const char *(*const vars)[ENV_MAX]);

/*
 * Read a filename from the environment variable VARNAME, canonicalise it,
 * check whether it resides within the directory JAIL, and, if so, open the
 * file, and store a pointer to the canonicalised filename in FNAME and
 * its file descriptor in FD.
 *
 * JAIL must be to canonical path. If the filename contains symbolic links
 * at the time the file is opened, an EMLINK error is raised.
 *
 * FNAME is allocated anough memory to hold the canonicalised filename
 * and should be freed by the caller.
 *
 * Return code:
 *      OK           Success.
 *      ERR_CNV*     File descriptor is too large (Linux only).
 *      ERR_ENV_LEN  The filename is longer than STR_MAX - 1 bytes.
 *      ERR_ENV_MAL  The file resides outside of the given jail.
 *	ERR_ENV_NIL  The environment variable is undefined or empty.
 *      ERR_SYS      System failure. errno(2) should be set.
 *
 *      Errors marked with an asterisk should be impossible.
 */
enum error
env_file_open(const char *const jail, const char *const varname,
              const int flags, const char **const fname, int *const fd);


/*
 * Check if the string NAME is a valid environment variable name.
 */
__attribute__((nonnull(1), pure, warn_unused_result))
bool env_name_valid(const char *const name);

/*
 * Restore every environment variable in the list of environment variables
 * VARS that matches one of the shell wildcard patterns in PATTERNS.
 * See fnmatch(3) for the syntax.
 *
 * Note, an attacker may populate the environment with variables that are
 * not NUL-termianted. If the memory region after such a variable contains
 * a NUL within STR_MAX - 1 bytes from the start of the variable, setenv(3)
 * will overshoot and dump the contents of that memory region into the
 * environment. That said, suCGI should not be privy to any information not
 * in the environment already, so such an attack should be pointless.
 *
 * If VARS contains multiple assignments to the same variable name, it
 * depends on the system's libc how many and which of those assignments
 * are honoured. This cannot be helped because some implementations of
 * setenv(3), e.g., Apple's Libc, may store multiple assignments to the
 * same variable name in environ(7), and there is no way of knowing which
 * of those is the authoritative one. However, most programmes run via
 * CGI are scripts, and scripting languages usually provide an API to
 * access the environment, and such APIs usually use getenv(3), and
 * getenv(3) is usually implemented so that it returns a variable's
 * authoritative value. So this should not be an issue, usually.
 *
 * Return code:
 *      OK            Success.
 *      ERR_ENV_LEN   An environment variable is too long.
 *      ERR_ENV_MAL   An environment variable is malformed.
 *      ERR_STR_LEN*  Some string is too long.
 *      ERR_SYS       setenv(3) failed. errno(2) should be set.
 *
 *      Errors marked with an asterisk should be impossible.
 */
__attribute__((nonnull(1, 2), warn_unused_result))
enum error env_restore (const char *vars[], const char *const patterns[]);


#endif /* !defined(ENV_H) */
