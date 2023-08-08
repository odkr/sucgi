/*
 * Header file for userdir.c.
 *
 * Copyright 2022 and 2023 Odin Kroeger.
 *
 * This file is part of suCGI.
 *
 * SuCGI is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * SuCGI is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General
 * Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with suCGI. If not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(USERDIR_H)
#define USERDIR_H

#include <pwd.h>

#include "attr.h"
#include "params.h"
#include "types.h"


/*
 * Expand the given user directory pattern for the given user and copy the
 * expanded user directory to the memory area "dir" points to, which must
 * be of the given size, and return the length of the expanded directory
 * in the variable pointed to by by "dirlen".
 *
 * Return value:
 *     OK       Success.
 *     ERR_LEN  The resolved directory is too long.
 *     ERR_SYS  snprintf failed.
 */
_read_only(1) _read_only(2) _write_only(4) _write_only(5, 3)
    _format(printf, 1, 0) _nonnull(1, 2, 5) _nodiscard
Error userdir_exp(const char *str, const struct passwd *user,
                  size_t size, size_t *dirlen, char *dir);


#endif /* !defined(USERDIR_H) */
