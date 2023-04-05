/*
 * Header file for for path.c.
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

#if !defined(PATH_H)
#define PATH_H

#include <sys/types.h>

#include "cattr.h"
#include "types.h"


/*
 * Check if the file naemd FNAME is within BASEDIR.
 * FNAME and BASEDIR must be canonical.
 *
 * Return value:
 *     OK           FNAME is within BASEDIR.
 *     ERR_BASEDIR  FNAME is not within of BASEDIR.
 *     ERR_LEN      BASEDIR is longer than MAX_FNAME_LEN - 1 bytes.
 */
__attribute__((nonnull(1, 2), warn_unused_result))
Error pathchkloc(const char *basedir, const char *fname);

/*
 * Check whether the user with the user ID UID has exclusive write access
 * to the file named FNAME and each parent directory of FNAME up to, and
 * including, BASEDIR.
 *
 * Return value:
 *     OK            UID has exclusive write access to FNAME.
 *     ERR_BASEDIR   FNAME is not within BASEDIR.
 *     ERR_WEXCL     UID does NOT have exclusive write access to FNAME.
 *     ERR_SYS       stat failed.
 */
__attribute__((nonnull(2, 3), warn_unused_result))
Error pathchkperm(uid_t uid, const char *basedir, const char *fname);

/*
 * Return a pointer to the filename suffix of FNAME in SUFFIX.
 *
 * Return value:
 *     OK          Success.
 *     ERR_SUFFIX  FNAME has no filename suffix.
 */
__attribute__((nonnull(1, 2), warn_unused_result))
Error pathsuffix(const char *fname, const char **suffix);


#endif /* !defined(PATH_H) */
