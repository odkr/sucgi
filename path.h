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
#include <stdbool.h>

#include "attr.h"
#include "types.h"

/*
 * Get the canonical path of the given file and return its length in the
 * variable pointed to by "reallen" and a pointer to the canonical path
 * itself in the variable by "real". The given filename must be of the
 * given length. The memory needed to store the canonical filename is
 * allocated automatically and should be freed by the caller.
 *
 * Differs from realpath, and possibly some implementations of realpath, by
 * checking whether the length of the given filename and the length of the
 * file's canonical name is longer than MAX_FNAME_LEN. If NULL is returned
 * in "real", then the given filename is too long; otherwise, the canonical
 * name is too long.
 *
 * Return value:
 *     OK       Success.
 *     ERR_LEN  A filename is longer than MAX_FNAME_LEN - 1 bytes.
 *     ERR_SYS  realpath failed.
 */
_read_only(2, 1) _write_only(3) _write_only(4) _nonnull(2, 4) _nodiscard
Error path_real(size_t fnamelen, const char *fname,
                size_t *reallen, char **real);

/*
 * Return a pointer to the suffix of the given filename in "suffix".
 *
 * Return value:
 *     OK          Success.
 *     ERR_SUFFIX  The filename has no suffix.
 */
_read_only(1) _write_only(2) _nonnull(1, 2) _nodiscard
Error path_suffix(const char *fname, const char **suffix);

/*
 * Check if the given file is within the given base directory.
 * The filename and the base directory name must be of the given
 * lengths and should be canonical.
 */
_read_only(2, 1) _read_only(4, 3) _nonnull(2, 4) _nodiscard
bool path_within(size_t fnamelen, const char *fname,
                 size_t basedirlen, const char *basedir);

#endif /* !defined(PATH_H) */
