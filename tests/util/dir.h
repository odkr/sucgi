/*
 * Header for dir.c.
 *
 * Copyright 2023 Odin Kroeger.
 *
 * This file is part of suCGI.
 *
 * suCGI is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * suCGI is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General
 * Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with suCGI. If not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(TESTS_UTIL_DIR_H)
#define TESTS_UTIL_DIR_H

#include "../../attr.h"
#include "types.h"

/*
 * Remove a file recursively.
 *
 * Otherwise, the same as dir_walk.
 */
_read_only(1) _nonnull(1)
int dir_tree_rm(const char *fname, ErrorFn errh);

/*
 * Walk the given directory tree in the given order and call the given
 * function for each file, including symlinks, but do not follow symlinks
 * or cross filesystems boundaries.
 *
 * dir_walk also takes a pointer to an error handling function. If an error
 * occurs, this function is called after clean-up; if that pointer is NULL,
 * error handling is left to the caller.
 *
 * If the original working directory cannot be restored, the calling process
 * is terminated immediately by calling _exit.
 *
 * dir_walk differs from ftw and nftw by fully traversing directory
 * trees even if they contain files the absolute paths of which are
 * longer than PATH_MAX.
 *
 * Return value:
 *     Zero      Success.
 *     Non-zero  Something went wrong; errno should be set.
 *
 * Caveats:
 *     Not async-safe.
 */
_read_only(1) _nonnull(1, 2)
int dir_walk(const char *fname, FileFn func, Order order, ErrorFn errh);

#endif /* !defined(TESTS_UTIL_DIR_H) */
