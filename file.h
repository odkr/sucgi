/*
 * Headers for file.c.
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

#if !defined(INCLUDED_FILE)
#define INCLUDED_FILE

#include <stdbool.h>
#include <sys/stat.h>

#include "err.h"


/* Check if fstatus indicates that the current has execute permissions. */
bool file_is_exec (const struct stat *const fstatus);

/* Check if fstatus indicates that only uid and gid have write permissions. */
bool file_is_wexcl (const uid_t uid,
                    const gid_t gid,
	            const struct stat *const fstatus);

/*
 * Open fname with flags and store its file descriptor in
 * the variable pointed to by fd.
 *
 * Errors:
 *	- fname contains a symlink (macOS and Linux)
 *
 *	Plus every error that may occur when opening a file with open(2).
 *
 * Return code:
 *      OK       Success.
 *      ERR_SYS  System failure. errno(2) should be set.
 */
enum code file_safe_open (const char * const fname, const int flags, int *fd);

/*
 * Store fname's filesystem status in the variable pointed to by fstatus.
 *
 * If fstatus is NULL, the status is not stored.
 * fstatus should be freed by the caller.
 *
 * Errors:
 * 	The same as file_safe_open.
 *
 * Return code: 
 *      OK       Success.
 *      ERR_SYS  System failure. errno(2) should be set.
 */
enum code file_safe_stat (const char * const fname, struct stat **fstatus);


#endif /* Include guard. */
