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

#if !defined(SRC_ENV_H)
#define SRC_ENV_H

#include <sys/stat.h>

#include "attr.h"
#include "err.h"
#include "str.h"


/*
 * Constants
 */

/* Maximum number of environment variables. */
#define VAR_MAX 256U


/*
 * Globals
 */

/* The environment. */
extern char **environ;

/*
 * Environment variables to keep.
 *
 * Array of shell wildcard patterns; must be NULL-terminated.
 * Variables are only kept if their name matches one of these patterns.
 * The only exception to that rule is $PATH, which is always kept.
 *
 * Adopted from Apache's suEXEC. There should be no need to adapt this list.
 */

/* Flawfinder: ignore; array is constant. */
extern const char *const env_keep[49];

/*
 * Environment variables to toss even if they match an ENV_KEEP pattern.
 *
 * Array of shell wildcard patterns; must be NULL-terminated.
 * Variables are tossed if their name matches one of these patterns.
 * The only exception to this rule is $PATH, which is never tossed.
 *
 * Adopted from Apache's suEXEC. There should be no need to adapt this list.
 */

/* Flawfinder: ignore; array is constant. */
extern const char *const env_toss[2];


/*
 * Functions
 */

/*
 * Clear the environment and save a copy of the old environment to vars.
 * If vars is NULL, the old environment is not saved. vars must have
 * enough space to hold VAR_MAX variables.
 *
 * Return code:
 *      OK           Success.
 *      ERR_VAR_MAX  Encountered more than VAR_MAX environment variables.
 */
error env_clear(char *vars[]);

/*
 * Safely read a filename from the environement variable name,
 * check if its of type ftype and, if so, store a pointer to the
 * filename in fname and a pointer to the file's status in fstatus.
 * If fstatus is NULL, the status is dicarded.
 *
 * Errors:
 *      - The given environment variable is undefined.
 *      - The value of the variable is the empty string.
 *	- The file is not of the given type.
 *
 *      And any error that path_check_len or file_safe_stat may raise.
 *
 * Return code:
 *      OK             Success.
 *      ERR_FILE_NAME  A filename in the path is too long.
 *      ERR_STR_MAX    The whole path is too long.
 *      ERR_SYS        System failure. errno(2) should be set.
 *      ERR_VAR_UNDEF  The variable is undefined.
 *      ERR_VAR_EMPTY  The variable is empty.
 */
error env_get_fname(const char *name, const mode_t ftype,
                    /* Flawfinder: ignore; realpath should respect PATH_MAX. */
                    char (*fname)[STR_MAX], struct stat *fstatus);

/*
 * Unset every environment variable the name of which (a) does not match a
 * pattern in keep or (b) does match a pattern in toss. Patterns are shell
 * wildcard patterns. See fnmatch(3) for the syntax.
 *
 * Errors:
 *      - A variable is the empty string.
 *      - A variable name is longer than STR_MAX - 1.
 *      - A variable does not contain a "=".
 *      - A variable assigns a value to the empty string.
 *
 * Caveats:
 *      An attacker may populate the environment with variables that are not
 *      terminated by a nullbyte. If the memory region after that variable
 *      contains a nullbyte within STR_MAX - 1 bytes from the start of the
 *      variable, setenv will overshoot and dump the contents of that region
 *      into the environment, from where they can be extracted with 'ps -Eef'.
 *      That said, suCGI should not be privy to any information that is not
 *      in the environment already, so such an attack should be pointless.
 *
 *      Moreover, if vars contains multiple assignments to the same name, it
 *      depends on the system's libc how many and which of those assignments
 *      are honoured. This cannot be helped because some implementations of
 *      setenv(3), e.g., Apple's Libc, may store multiple assignments to the
 *      same variable name in environ(7), and there is no way of knowing which
 *      of those assignments is the definitive one. However, most programmes
 *      run via CGI are scripts, and scripting languages usually provide an
 *      API to access the environment, and such APIs usually use getenv(3),
 *      and getenv is usually implemented so that it always returns the
 *      definitive value of an environment variable. So this should not
 *      be an issue, usually.
 *
 * Return code:
 * 	OK               Success.
 *      ERR_VAR_MAX      Encountered more than VAR_MAX environment variables.
 * 	ERR_STR_MAX      A variable name is longer than STR_MAX - 1 bytes.
 *	ERR_VAR_INVALID  A variable is not of the form key=value.
 * 	ERR_SYS          System error. errno(2) should be set.
 */
error env_sanitise (const char *const keep[], const char *const toss[]);


#endif /* !defined(SRC_ENV_H) */
