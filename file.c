/*
 * File handling.
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

#define _XOPEN_SOURCE 700

#if defined(__OPTIMIZE__) && !defined(_FORTIFY_SOURCE)
#define _FORTIFY_SOURCE 3
#endif

#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>

#include "file.h"


bool
file_is_exe(const uid_t uid, const gid_t gid, const struct stat fstatus)
{
    mode_t perm = fstatus.st_mode;

    if (uid == 0) {
        return (perm & (S_IXUSR | S_IXGRP | S_IXOTH)) != 0;
    }

    if (fstatus.st_uid == uid) {
        return (perm & S_IXUSR) != 0;
    }
    if (fstatus.st_gid == gid) {
        return (perm & S_IXGRP) != 0;
    }
    return (perm & S_IXOTH) != 0;
}

bool
file_is_wexcl(const uid_t uid, const struct stat fstatus)
{
    mode_t perm = fstatus.st_mode;

    return fstatus.st_uid == uid && (perm & (S_IWGRP | S_IWOTH)) == 0;
}

