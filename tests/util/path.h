/*
 * Header for path.c.
 *
 * Copyright 2023 Odin Kroeger.
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

#if !defined(TESTS_UTIL_PATH_H)
#define TESTS_UTIL_PATH_H

#include <sys/types.h>
#include <stdarg.h>

#include "../../attr.h"
#include "types.h"

/*
 * Generate a path-like string with the given length, not counting the
 * terminating NUL, and copy it to address given by "path", which must
 * point to a memory area that is large enough to hold the generated
 * path, including the terminating NUL.
 */
_read_only(2) _nonnull(2)
void path_gen(size_t len, char *path);

/*
 * Join the given directory and the given filename and copy the
 * resulting path to address given by "path", which must point to
 * a memory area of the given size; that memory area must be large
 * enough to hold the joined path, including the terminating NUL.
 *
 * path_join also takes a pointer to an error handling function. If
 * an error occurs, this function is called after clean-up; if that
 * pointer is NULL, error handling is left to the caller.
 *
 * If the given size is 0, the resulting path is *not* copied to
 * the address pointed to by "path", but the length of the resulting
 * path is still returned.
 *
 * Return value:
 *     Non-negative  Length of joined path.
 *     Negative      Something went wrong; errno may be set.
 *
 * Errors:
 *    See snprintf.
 */
_write_only(2, 1) _read_only(3) _read_only(4) _nonnull(3, 4)
int path_join(size_t size, char *path, const char *dir, const char *fname,
              ErrorFn errh);

/*
 * Get a copy of the basename of the top-most directory of the given path
 * that is not "." and return a pointer to the remaining path, that is, the
 * one relative to the basename, in the addressed pointed to by "ptr".
 *
 * If the given path comprises only a basename, the remaining path is the
 * empty string and a pointer to the terminating NUL is returned in the
 * addressed pointed by "ptr".
 *
 * If the given path is the empty string, no copy is made, no memory is
 * allocated, and NULL is returned both as return value and in the address
 * pointed to by "ptr".
 *
 * If "ptr" is NULL, the remaning path is not returned.
 *
 * The memory for the copy of the basename is allocated automatically
 * and should be freed by the caller.
 *
 * path_split also takes a pointer to an error handling function.
 * See path_join for details.
 *
 * Return value:
 *     Non-negative  Length of the basename.
 *     Negative      Something went wrong; errno should be set.
 *
 * Errors:
 *     See strndup.
 *
 * Example:
 *     #include <err.h>
 *
 *     char *seg = NULL;
 *     char *ptr = path;
 *     while ((errno = 0, seg = path_split(ptr, &ptr, err)) != NULL) {
 *         if (*ptr == '\0') {
 *             printf("final segment: %s\n", seg);
 *         } else {
 *             printf("segment: %s\n", seg);
 *         }
 *         free(seg);
 *     }
 */
_read_only(1) _write_only(2) _nonnull(1) _nodiscard
char *path_split(const char *path, const char **ptr, ErrorFn errh);

/*
 * Split the given path into segments and apply the function pointed
 * to by "dirfn" to each segment except for the last and the function
 * pointed to by "basefn" to that last segment.
 *
 * The functions are given a path, the number of variadic arguments
 * to follow, and an argument pointer to those arguments, and should
 * return zero on success and non-zero on failure.
 *
 * If one of the two function pointers is NULL, no function is called for
 * the corresponding path segments. However, only one of them can be NULL.
 *
 * path_walk also takes a pointer to an error handling function.
 * See path_join for details.
 *
 * Additional arguments are passed on to "dirfn" and "basefn".
 * The number of these arguments must be given in "nargs".
 *
 * If the original working directory cannot be restored, the calling
 * process is terminated immediately by calling _exit.
 *
 * Return value:
 *     Zero      Success.
 *     Non-zero  Something went wrong; errno should be set.
 *
 * Caveats:
 *     Not async-safe.
 */
_read_only(1) _nonnull(1)
int path_walk(const char *fname, VFileFn dirfn, VFileFn basefn,
              ErrorFn, size_t nargs, ...);

#endif /* !defined(TESTS_UTIL_PATH_H) */
