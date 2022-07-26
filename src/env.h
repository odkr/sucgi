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


/*
 * Constants
 */

/* Maximum number of environment variables. */
#define ENV_MAX 256


/*
 * Globals
 */

/*
 * Environment variables to keep.
 *
 * Array of shell wildcard patterns; must be NULL-terminated.
 * Variables are only kept if their name matches one of these patterns.
 * The only exception to that rule is $PATH, which is always kept.
 */
extern char *const env_keep[];

/*
 * Environment variables to toss even if they match an ENV_KEEP pattern.
 *
 * Array of shell wildcard patterns; must be NULL-terminated.
 * Variables are tossed if their name matches one of these patterns.
 * The only exception to this rule is $PATH, which is never tossed.
 */
extern char *const env_toss[];


/*
 * Functions
 */

/*
 * Clear the environment and ssave a copy of the old environment to vars.
 * If vars is NULL, the old environment is not saved.
 *
 * Return code:
 *      OK       Success.
 *      ERR_SYS  System failure. errno(2) should be set.
 */
error env_clear(char ***vars);

/* 
 * Safely read a filename from the environement variable name and store a
 * pointer to that name in fname and a pointer to the status of the file it
 * points to in fstatus. If fstatus is NULL, the status is dicarded.
 *
 * Errors:      
 *      - The given environment variable is undefined.
 *      - The value of the variable is the empty string.
 *      
 *      And every error that path_check_len or file_safe_stat may raise.
 *
 * Return code:
 *      OK             Success.
 *      ERR_STR_LEN    PATH is too long.
 *      ERR_SYS        System failure. errno(2) should be set.
 *      ERR_VAR_UNDEF  NAME is undefined.
 *      ERR_VAR_EMPTY  NAME is empty.
 */
__attribute__((RO(1)))
error env_get_fname(const char *name, char **fname, struct stat **fstatus);

/*
 * Repopulate the environment with any variable in vars the name of which
 * (a) matches a pattern in keep and (b) does not match a pattern in toss,
 * where vars is an array of environment variables that follows the same
 * syntax as the global variable environ(7) and keep and toss are arrays
 * of shell wildcard patterns that are matched against variable names; see 
 * fnmatch(3) for the syntax.
 *
 * Errors:
 *      - A variable is empty.
 *      - A variable is longer than STR_MAX_LEN.
 *      - A variable does not contain a "=".
 *      - A variable assigns a value to the empty string.
 *
 * Caveats:
 *      An attacker may populate the environment with variables that are not
 *      terminated by a nullbyte. If the memory region after that variable
 *      contains a nullbyte within STR_MAX_LEN bytes from the start of the
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
 * 	ERR_STR_LEN      A variable is longer than STR_MAX_LEN.
 *	ERR_VAR_INVALID  A variable is not of the form key=value.
 * 	ERR_SYS          System error. errno(2) should be set.
 */
__attribute__((RO(1), RO(2), RO(3)))
error env_restore(const char *const *vars,
                  char *const *const keep,
                  char *const *const toss);


#endif /* !defined(SRC_ENV_H) */
