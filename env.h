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

#if !defined(INCLUDED_ENV)
#define INCLUDED_ENV

#include <sys/stat.h>

#include "err.h"


/*
 * Functions
 */

/* Clear the environment and store a pointer to the old environment in old. */
void env_clear (char ***old);

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
enum code env_get_fname (const char *const name,
                         char **fname,
                         struct stat **fstatus);

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
 *      Moreover, if vars contains multiple assignments to the same name,
 *      then it depends on the system's libc whether how many and which of
 *      those assignments are kept. This is because some implementations of
 *      setenv(3), e.g., Darwin's, may store multiple assignments to the same
 *      variable name in environ(7), and there is no way of which of those
 *      assignments is the definitive one. Most CGI programmes are scripts,
 *      and script interpreters should provide a secure API to access environ,
 *      so this should not be a huge issue.
 *
 * Return code:
 * 	OK               Success.
 * 	ERR_STR_LEN      A variable is longer than STR_MAX_LEN.
 *	ERR_VAR_INVALID  A variable is not of the form key=value.
 * 	ERR_SYS          System error. errno(2) should be set.
 */
enum code env_restore (const char *const *vars,
                       char *const *const keep,
                       char *const *const toss);


#endif /* Include guard. */
