/*
 * Header for tmp.c.
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

#if !defined(TESTS_UTIL_TMP_H)
#define TESTS_UTIL_TMP_H

#include <stdbool.h>

#include "../../attr.h"


/*
 * Create a temporary directory and returns its canonical name.
 * Subsequent invocations are ignored until tmpdirremove was called.
 *
 * Return value:
 *     Non-NULL  Success.
 *     NULL      Something went wrong; errno should be set.
 *
 * Caveats:
 *     Not fully async-safe.
 */
__attribute__((warn_unused_result))
char *tmpdirmake(void);

/*
 * Remove the temporary directory created by tmpdirmake.
 * Subsequent invocations are ignored until tmpdirmake is called again.
 *
 * Return value:
 *      0  Success.
 *     -1  Something went wrong; errno should be set.
 *
 * Caveats:
 *     Not fully async-safe.
 */
__attribute__((warn_unused_result))
int tmpdirremove(void);


#endif /* !defined(TESTS_UTIL_TMP_H) */
