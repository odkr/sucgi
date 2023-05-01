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

#include "attr.h"
#include "types.h"


/*
 * Check if the file named FNAME is within BASEDIR.
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
 * Get the canonical path of FNAME, allocate enough memory to store that
 * path, and return a pointer to that memory in REAL; that memory should
 * be freed by the caller.
 *
 * Differs from realpath by checking whether the length of FNAME is longer
 * than MAX_FNAME_LEN. If REAL is NULL, then FNAME is too long; otherwise
 * REAL is too long.
 *
 * Return value:
 *     OK       Success.
 *     ERR_LEN  The filename is longer than MAX_FNAME_LEN - 1 bytes.
 *     ERR_SYS  realpath failed.
 *
 * FIXME: Not unit-tested.
 */
__attribute__((nonnull(1, 2), warn_unused_result))
Error pathreal(const char *fname, char **real);

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
