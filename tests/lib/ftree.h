/*
 * Header for ftree.c.
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

#if !defined(TESTS_LIB_FTREE_H)
#define TESTS_LIB_FTREE_H

#include "../../attr.h"


/*
 * Data types
 */

/* Orders in which to traverse trees. */
typedef enum {
    FTREE_PRE,
    FTREE_POST
} FTreeOrder;

/* Function to apply files in a tree. */
typedef int (*FTreeFunc)(const char *, int);


/*
 * Functions
 */

/*
 * Remove the directory FNAME recursively.
 *
 * Return value:
 *     Zero      Success.
 *     Non-zero  Something went wrong; errno should be set.
 */
__attribute__((nonnull(1), warn_unused_result))
int ftreerm(const char *fname);

/*
 * Walk the directory tree FNAME in the given ORDER and apply FUNC to
 * each file, but do not follow symbolic links or cross filesystems.
 *
 * FUNC is given a filename and the current recursion depth and
 * should return zero on success and non-zero on failure.
 *
 * ftreewalk differs from ftw(3) and nftw(3) by fully traversing trees even
 * if they contain files the absolute paths of which are longer than PATH_MAX;
 * that said, this behaviour may be system-dependent.
 *
 * Return value:
 *     Zero      Success.
 *     Non-zero  Something went wrong; errno should be set.
 */
__attribute__((nonnull(1, 2), warn_unused_result))
int ftreewalk(const char *fname, FTreeFunc func, FTreeOrder order);


#endif /* !defined(TESTS_LIB_FTREE_H) */
