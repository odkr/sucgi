/*
 * Header for tmpdir.c.
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

#if !defined(TESTS_UTIL_TMPDIR_H)
#define TESTS_UTIL_TMPDIR_H

#include <sys/types.h>

#include "../../attr.h"
#include "types.h"

/*
 * Create a temporary directory in ${TMPDIR:-/tmp} using the given
 * filename template, replacing every 'X' with a random character,
 * and return the filename of that directory in "dir".
 *
 * The memory for the filename is allocated automatically and should
 * be freed by the caller.
 *
 * tmpdir_make takes a pointer to an error handling function. If an error
 * occurs, this function is called after clean-up; if that pointer is NULL,
 * error handling is left to the caller.
 *
 * Return value:
 *     Zero      Success.
 *     Non-zero  Something went wrong; errno should be set.
 */
_read_only(1) _write_only(2) _nonnull(1, 2)
int tmpdir_make(const char *fname, char **dir, ErrorFn errh);

#endif /* !defined(TESTS_UTIL_TMPDIR_H) */
