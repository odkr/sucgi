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

#include <stdbool.h>

#include "attr.h"
#include "max.h"
#include "types.h"


/*
 * Check if the user with ID UID has exclusive write access to the file
 * named FNAME and each parent directory of FNAME up to, and including,
 * PARENT and store a copy of the last path checked in CUR.
 *
 * PARENT and FNAME must be sanitised and canonical.
 *
 * Return value:
 *      OK          Success.
 *      ERR_CNV*    File descriptor is too large (Linux only).
 *      ERR_LEN     FNAME is too long.
 *      ERR_OPEN    open(2) or openat2(2) failed.
 *      ERR_CLOSE   close(2) failed.
 *      ERR_STAT    stat(2) failed.
 *      FAIL        UID does not have exclusive write access to FNAME.
 *
 *      Errors marked with an asterisk should be impossible.
 */
__attribute__((nonnull(2, 3, 4), warn_unused_result))
enum retval path_check_wexcl(const uid_t uid, const char *const fname,
                             const char *const parent, char cur[MAX_FNAME]);

/*
 * Check if FNAME is a sub-directory of PARENT.
 *
 * FNAME and PARENT must be sanitisied and canonical.
 */
__attribute__((nonnull(1, 2), pure, warn_unused_result))
bool path_is_subdir(const char *const fname, const char *const parent);


#endif /* !defined(PATH_H) */
