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

#if !defined(FILE_H)
#define FILE_H

#include <stdbool.h>
#include <sys/stat.h>

#include "defs.h"
#include "error.h"


/*
 * Check if FSTATUS indicates that the current user has execute permissions.
 */
__attribute__((pure, warn_unused_result))
bool file_is_exec(const struct stat fstatus);

/*
 * Check if FSTATUS indicates that only UID has write permissions.
 */
__attribute__((pure, warn_unused_result))
bool file_is_wexcl(const uid_t uid, const struct stat fstatus);

/*
 * Open FNAME with FLAGS and store its file descriptor in FD.
 * FNAME must not contain symbolic links.
 *
 * Return code:
 *      OK        Success.
 *      ERR_CNV*  File descriptor is too large (Linux only).
 *      ERR_SYS   System failure. errno(2) should be set.
 *
 *      Errors marked with an asterisk should be impossible.
 */
__attribute__((nonnull(1, 3), warn_unused_result))
enum error file_safe_open(const char *const fname,
                          const int flags, int *const fd);

/*
 * Store FNAME's filesystem status in FSTATUS.
 * FNAME must not contain symbolic links.
 *
 * Return code:
 *      See file_safe_open.
 */
 __attribute__((nonnull(1, 2), warn_unused_result))
enum error file_safe_stat(const char *const fname,
                          struct stat *const fstatus);


#endif /* !defined(FILE_H) */
