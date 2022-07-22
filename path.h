/*
 * Headers for path.c.
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

#if !defined(INCLUDED_PATH)
#define INCLUDED_PATH

#include <stdbool.h>
#include <sys/types.h>

#include "err.h"


/*
 * Check if path's length is within:
 * 	- PATH_MAX
 *	- pathconf(dir, _PC_PATH_MAX) for each directory in path.
 *	- STR_MAX_LEN
 *
 * Return code:
 *      OK           PATH is within limits.
 *      ERR_STR_LEN  PATH is too long.
 *      ERR_SYS      System failure. errno(2) should be set.
 */
enum code path_check_len(const char *const path);

/*
 * Check if the given user has exclusive write access to every directory
 * and the file that comprise path, up to (and including) the path stop.
 *
 * Return code:
 *      OK             User has exclusive write access.
 *      ERR_NOT_EXCLW  User does not have exclusive write access.
 *      ERR_STR_LEN    The path is longer than STR_MAX_LEN.
 *      ERR_SYS        System failure. errno(2) should be set.
 */
enum code
path_check_wexcl(const uid_t uid, const gid_t gid,
                 const char *const path, const char *const stop);

/*
 * Check if the path super contains the path sub.
 *
 * Caveats:
 *      This check is meaningless unless both paths are canonical.
 */
bool path_contains(const char *const super, const char *const sub);


#endif /* Include guard. */
