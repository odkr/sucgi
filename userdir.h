/*
 * Header file for userdir.c.
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

#if !defined(USERDIR_H)
#define USERDIR_H

#include <pwd.h>

#include "cattr.h"
#include "max.h"
#include "types.h"


/*
 * Resolve the user directory pattern STR for the given USER and return
 * the resolved user directory in DIR, which must be large enough to hold
 * MAX_FNAME_LEN bytes, including the terminating NUL.
 *
 * Return value:
 *     OK       Success.
 *     ERR_LEN  Resolved directory is too long.
 *     ERR_SYS  snprintf failed.
 */
__attribute__((nonnull(1, 2, 3), format(printf, 1, 0), warn_unused_result))
Error userdirexp(const char *str, const struct passwd *user,
                      char dir[MAX_FNAME_LEN]);


#endif /* !defined(USERDIR_H) */
