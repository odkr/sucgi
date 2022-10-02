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

#if !defined(PATH_H)
#define PATH_H

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

#include "defs.h"
#include "err.h"
#include "str.h"


/*
 * Check if the user with ID UID has exclusive write access to the file
 * named FNAME and each parent directory of FNAME up to the path PARENT
 * and store a copy of the last path checked in CUR.
 *
 * PARENT and FNAME must be canonical.
 *
 * Return code:
 *      OK              Success.
 *      ERR_CNV*        File descriptor is too large (Linux only).
 *      ERR_PATH_OUT    FNAME is not within PARENT.
 *      ERR_PATH_WEXCL  UID does not have exclusive write access to FNAME.
 *      ERR_SYS         open(2) or stat(2) error. errno(2) should be set.
 *
 *      Errors marked with an asterisk should be impossible.
 */
__attribute__((nonnull(2, 3, 4), warn_unused_result))
enum error path_check_wexcl(const uid_t uid, const char *const parent,
                            const char *const fname,
                            char (*const cur)[STR_MAX]);

/*
 * Check if the path PARENT names a super-directory of the file named FNAME.
 *
 * PARENT and FNAME must be canonical.
 */
__attribute__((nonnull(1, 2), pure, warn_unused_result))
bool path_contains(const char *const parent, const char *const fname);


#endif /* !defined(PATH_H) */
