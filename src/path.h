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

#if !defined(SRC_PATH_H)
#define SRC_PATH_H

#include <stdbool.h>
#include <sys/types.h>

#include "attr.h"
#include "err.h"


/*
 * Check whether:
 *      - path's length is within PATH_MAX and STR_MAX_LEN.
 *      - each sub-path in path is within pathconf(parent, _PC_PATH_MAX).
 *      - each segment of the given path is within NAME_MAX, FILENAME_MAX,
 *        STR_MAX_LEN, and pathconf(parent, _PC_NAME_MAX).
 *
 * Return code:
 *      OK             The path is within limits.
 *      ERR_FNAME_LEN  The filename of a path segment is too long.
 *      ERR_STR_LEN    The path or a path segment is too long.
 *      ERR_SYS        System failure. errno(2) should be set.
 */
__attribute__((RO(1)))
error path_check_len(const char *const path);

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
__attribute__((RO(2), RO(3)))
error path_check_wexcl(const uid_t uid, const char *const path,
                       const char *const stop);

/*
 * Check if the path super contains the path sub.
 *
 * Caveats:
 *      This check is meaningless unless both paths are canonical.
 */
__attribute__((RO(1), RO(2)))
bool path_contains(const char *const super, const char *const sub);


#endif /* !defined(SRC_PATH_H) */
