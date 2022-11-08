/*
 * Headers for userdir.c.
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

#if !defined(USERDIR_H)
#define USERDIR_H

#include <pwd.h>

#include "types.h"


/*
 * Take S to be a user directory pattern (see config.h) for the given USER,
 * resolve the pattern, and store the result in USER_DIR.
 *
 * The expanded string is stored in USER_DIR before it is resolved with
 * realpath(3) and can be used in error messages.
 *
 * Caveats:
 *     The address that USER_DIR points to if an error occurs is static.
 *     Its content is updated on every invocation of userdir_resolve.
 *
 * Return value:
 *     OK       Success.
 *     ERR_PRN  snprintf(3) failed.
 *     ERR_LEN  The expanded string is longer than MAX_FNAME - 1 bytes.
 *     ERR_RES  realpath(3) failed.
 */
__attribute__((nonnull(1, 2, 3), format(printf, 1, 0), warn_unused_result))
enum retval userdir_resolve(const char *const s, const struct passwd *user,
                            char **user_dir);


#endif /* !defined(USERDIR_H) */
