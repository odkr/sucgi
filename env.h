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

#include "error.h"
#include "str.h"
#include "sysconf.h"
#include "types.h"


/*
 * Constants
 */

/* Maximum variable name length. */
#define ENV_MAX_NAME 128


/*
 * Globals
 */

/* The environment. */
extern char **environ;


/*
 * Functions
 */

/*
 * Clear the environment and store a copy of at most MAX - 1 variables in
 * VARS. If VARS is NULL, the old environment is discarded. VARS must point
 * to a memory area that is large enough to hold MAX pointers to strings.
 *
 * Return value:
 *      OK       Success.
 *      ERR_LEN  There are more than MAX environment variables.
 */
__attribute__((warn_unused_result))
enum retval env_clear(size_t max, const char *vars[max]);

/*
 * Read a filename from the environment variable VAR, canonicalise it, check
 * whether it resides within the directory JAIL; if it does, open the file and
 * store its canonicalised filename in FNAME and its file descriptor in FD.
 *
 * JAIL must exist and be to canonical path. If the filename contains symbolic
 * links at the time the file is opened, an EMLINK error is raised.
 *
 * FNAME is allocated enough memory to store the canonicalised filename and
 * should be freed by the caller. It also contains the last filename passed
 * to file-related system calls and can be used in error messages.
 *
 * FD is closed on exit.
 *
 * Return value:
 *      OK        Success.
 *      ERR_CNV*  File descriptor is too large (Linux only).
 *      ERR_ILL   The file is not within the jail.
 *	ERR_NIL   VARNAME is undefined or empty.
 *      ERR_LEN   The filename is too long.
 *      ERR_MEM   strndup(2) failed.
 *      ERR_ENV   getenv(3) failed.
 *      ERR_OPEN  open(2) or openat2(2) failed.
 *      ERR_RES   realpath(3) failed.
 *
 *      Errors marked with an asterisk should be impossible.
 */
__attribute__((nonnull(1, 2, 4, 5), warn_unused_result))
enum retval env_fopen(const char *const jail, const char *const var,
                      const int flags, const char **const fname,
                      int *const fd);


/*
 * Check if the string NAME is a valid environment variable name.
 */
__attribute__((nonnull(1), pure, warn_unused_result))
bool env_is_name(const char *const name);

/*
 * Restore every environment variable in ENV that matches one the the given
 * PATTERNS. ENV is a NULL-terminated array of variables saved with env_clear.
 * PATTERNS is a NULL-terminated array of shell wildcard patterns.
 *
 * The name of the variable that is currently processed is stored in NAME,
 * unless the variable could not be split up into a name and a value. If
 * the first variable cannot be split up, NAME is not initialised.
 *
 * Note, an attacker may populate the environment with variables that are
 * not NUL-termianted. If the memory area after such a variable contains
 * a NUL within PATH_SIZE - 1 characters starting from the position of the
 * first '=', then setenv(3) will overshoot and dump the contents of that
 * memory area into the environment. That said, suCGI should not be privy
 * to any information that is not in the  environment already, so such an
 * attack should be pointless.
 *
 * If ENV contains multiple assignments to the same variable name, it
 * depends on the system's libc how many and which of those assignments
 * are honoured. This cannot be helped because some implementations of
 * setenv(3), e.g., Apple's Libc, may store multiple assignments to the
 * same variable name in environ(7), and there is no way of knowing which
 * of those is the authoritative one. However, most programmes run via
 * CGI are scripts, scripting languages usually provide an API to access
 * the environment, such APIs usually use getenv(3), and getenv(3) is
 * usually implemented so that it returns a variable's authoritative
 * value. So this should not be an issue, usually.
 *
 * Return value:
 *      OK       Success.
 *      ERR_CNV  Variable could not be split up into a name and a value.
 *      ERR_ILL  Variable name is illegal.
 *      ERR_LEN  Variable value is longer than PATH_SIZE - 1 bytes.
 *      ERR_ENV  setenv(3) failed.
 */
__attribute__((nonnull(1, 2), warn_unused_result))
enum retval env_restore (const char **env, const char *const *patterns,
                         char name[ENV_MAX_NAME]);


#endif /* !defined(ENV_H) */
