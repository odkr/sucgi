/* Headers for utils.c
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

#if !defined(SRC_TESTS_UTILS_H)
#define SRC_TESTS_UTILS_H

#include "../attr.h"
#include "../err.h"


/*
 * Functions
 */

/* Print a formatted error message to STDERR and exit with EXIT_FAILURE. */
__attribute__((READ_ONLY(1), noreturn))
void die(const char *const message, ...);


#endif /* !defined(SRC_TESTS_UTILS) */
