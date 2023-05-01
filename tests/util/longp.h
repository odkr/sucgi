/*
 * Header for longp.c.
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

#if !defined(TESTS_UTIL_LONGP_H)
#define TESTS_UTIL_LONGP_H

#include <stdarg.h>

#include "../../attr.h"


/*
 * Data types
 */

/* Role a directory entry. */
typedef enum {
    LONGP_FILE,
    LONGP_DIR
} LongPRole;

/* Function signature for longpdo. */
typedef int (*LongPFunc)(LongPRole role, const char *path, va_list argp);


/*
 * Functions
 */

/*
 * Apply FUNC to each path segment in PATH.
 *
 * FUNC is passed the role of the path segment, its filename,
 * and a pointer to the remaining arguments; it should return
 * zero on success and non-zero on failure.
 *
 * Return value:
 *     Zero      Success.
 *     Non-zero  Something went wrong errno should be set.
 */
__attribute__((nonnull(1, 2), warn_unused_result))
int longpdo(LongPFunc func, const char *path, ...);

/*
 * Create every directory that is an ancestor of the file named by PATH,
 * but not PATH itself, with the given permissions.
 *
 * Return value:
 *     Zero      Success.
 *     Non-zero  Something went wrong errno should be set.
 */
__attribute__((nonnull(1), warn_unused_result))
int longpmake(const char *path, const mode_t perms);


#endif /* !defined(TESTS_UTIL_LONGP_H) */
