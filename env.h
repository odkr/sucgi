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

#include "config.h"
#include "macros.h"
#include "error.h"
#include "str.h"


/*
 * Constants
 */

/* Characters allowed in environment variable names. */
#define ENV_NAME_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789" \
                       "abcdefghijklmnopqrstuvwxyz"

/*
 * Globals
 */

/* The environment. */
extern char **environ;

/*
 * Environment variables to keep.
 * NULL-terminated array of shell wildcard patterns.
 */
extern const char *const env_vars_safe[];


/*
 * Functions
 */

/*
 * Clear the environment and store a copy of the old environment in VARS.
 * If VARS is NULL, the old environment is discarded. VARS must point to
 * a memory area that is large enough to hold MAX_ENV variables.
 *
 * Return code:
 *      OK        Success.
 *      ERR_LEN  There are more than MAX_ENV environment variables.
 */
__attribute__((warn_unused_result))
enum error env_clear(/* RATS: ignore; vars is bound-checked. */
	             const char *(*const vars)[MAX_ENV]);

/*
 * Read a filename from the environment variable VARNAME, canonicalise it,
 * check whether it resides within the directory JAIL, and, if so, open the
 * file, and store a pointer to the canonicalised filename in FNAME and
 * its file descriptor in FD.
 *
 * JAIL must exist and be to canonical path; if JAIL does not exist and suCGI
 * has not been compiled with NDEBUG defined, then suCGI segfaults with a
 * cryptic error message. If the filename contains symbolic links at the time
 * the file is opened, an EMLINK error is raised.
 *
 * FNAME is allocated enough memory to store the canonicalised filename and
 * should be freed by the caller. It also contains the last filename passed
 * to file-related system calls and can be used in error messages.
 *
 * Return code:
 *      OK            Success.
 *      ERR_CNV*      File descriptor is too large (Linux only).
 *      ERR_ILL       The file is not within the jail.
 *	ERR_NIL       VARNAME is undefined or empty.
 *      ERR_LEN       The filename is too long.
 *      ERR_CALLOC    calloc(2) failed.
 *      ERR_GETENV    getenv(3) failed.
 *      ERR_OPEN      open(2) or openat2(2) failed.
 *      ERR_REALPATH  realpath(3) failed.
 *
 *      Errors marked with an asterisk should be impossible.
 */
__attribute__((nonnull(1, 2, 4, 5), warn_unused_result))
enum error env_file_open(const char *const jail, const char *const varname,
                         const int flags, const char **const fname,
			 int *const fd);


/*
 * Check if the string NAME is a valid environment variable name.
 */
__attribute__((nonnull(1), pure, warn_unused_result))
bool env_is_name(const char *const name);

/*
 * Restore every environment variable in the list of environment variables
 * VARS that matches one of the shell wildcard patterns in PATTERNS.
 * See fnmatch(3) for the syntax.
 *
 * Note, an attacker may populate the environment with variables that are
 * not NUL-termianted. If the memory region after such a variable contains
 * a NUL within MAX_STR - 1 bytes from the start of the variable, setenv(3)
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
 *      OK           Success.
 *      ERR_LEN      An environment variable is too long.
 *      ERR_ILL      An environment variable is ill-formed.
 *      ERR_SETENV   setenv(3) failed.
 */
__attribute__((nonnull(1, 2), warn_unused_result))
enum error env_restore (const char *vars[], const char *const patterns[]);


#endif /* !defined(ENV_H) */
