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
#define SC_ENV_MAX 192



/*
 * Globals
 */

/* The environment. */
extern char **environ;

/*
 * Environment variables to keep.
 * NULL-terminated array of shell wildcard patterns.
 */
extern const char *const env_keep[];

/*
 * Environment variables to toss even if they match a pattern in env_keep.
 * NULL-terminated array of shell wildcard patterns.
 */
extern const char *const env_toss[];


/*
 * Functions
 */

/*
 * Clear the environment and store a copy of the old environment in VARS.
 * If VARS is NULL, the old environment is discarded. VARS must point to
 * a memory area that is large enough to hold SC_ENV_MAX variables.
 *
 * Return code:
 *      SC_OK           Success.
 *      SC_ERR_ENV_MAX  There are more than SC_ENV_MAX environment variables.
 */
__attribute__((warn_unused_result))
enum error env_clear(const char *(*const vars)[SC_ENV_MAX]);

/*
 * FIXME: Documentation is missing.
 */
enum error
env_file_openat(const char *const jail, const char *const varname,
                const int flags, const char **const fname, int *const fd);

/*
 * Read a path from the environement variable NAME, canonicalise it, and check
 * whether none of its segments is a hidden file and wheter the given file is
 * of the type FTYPE; if so, store a pointer to the canonicalised path in
 * FNAME and a pointer to the file's filesystem status in FSTATUS. FNAME is
 * allocated enough memory to hold the path and should be freed by the caller.
 *
 * Return code:
 *      SC_OK             Success.
 *      SC_ERR_CNV*       File descriptor is larger than INT_MAX (Linux only).
 *      SC_ERR_FTYPE      File is not of the given type.
 *      SC_ERR_STR_LEN    Path is longer than STR_MAX - 1 bytes.
 *      SC_ERR_SYS        System error. errno(2) should be set.
 *      SC_ERR_ENV_NIL    The variable NAME is undefined or empty.
 *
 *      Errors marked with an asterisk should be impossible.
 */
__attribute__((nonnull(1, 3, 4), warn_unused_result))
enum error env_get_fname(const char *name, const mode_t ftype,
                         const char **const fname,
			 struct stat *const fstatus);

/*
 * Check if a string S is a valid environment variable name.
 */
__attribute__((nonnull(1), pure, warn_unused_result))
bool env_name_valid(const char *const s);

/*
 * Restore every environment variable in VARS the name of which matches a
 * a pattern in KEEP and does NOT match a pattern in TOSS, where patterns
 * are shell wildcard patterns. See fnmatch(3) for the syntax.
 *
 * ==========================================================================
 * CAVEATS
 * --------------------------------------------------------------------------
 * An attacker may populate the environment with variables that are not
 * termianted by NUL. If the memory region after such a variable contains
 * a NUL within STR_MAX - 1 bytes from the start of the variable, then
 * setenv(3) will overshoot and dump the contents of that memory region
 * into the environment. That said, suCGI should not be privy to any 
 * information that is not in the environment already, so such an
 * attack should be pointless.
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
 * ==========================================================================
 *
 * Return code:
 *      SC_OK            Success.
 *      SC_ERR_ENV_LEN   An environment variable is too long.
 *      SC_ERR_ENV_MAL   An environment variable is malformed.
 *      SC_ERR_STR_LEN*  Some string is too long.
 *      SC_ERR_SYS       setenv(3) failed. errno(2) should be set.
 *
 *      Errors marked with an asterisk should be impossible.
 */
__attribute__((nonnull(1, 2, 3), warn_unused_result))
enum error env_restore (const char *vars[],
                        const char *const keep[],
			const char *const toss[]);


#endif /* !defined(ENV_H) */
