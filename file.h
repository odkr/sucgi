/*
 * Header file for file.c.
 *
 * Copyright 2022 and 2023 Odin Kroeger.
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

#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>

#include "cattr.h"


/*
 * Check whether FSTATUS indicates that the user with the user ID UID
 * and the primary group ID GID can execute the corresponding file.
 */
__attribute__((const, warn_unused_result))
bool file_is_exe(uid_t uid, gid_t gid, struct stat fstatus);

/*
 * Check whether FSTATUS indicates that only the user with the
 * user ID UID has write access to the corresponding file.
 */
__attribute__((const, warn_unused_result))
bool file_is_wexcl(uid_t uid, struct stat fstatus);


#endif /* !defined(FILE_H) */
