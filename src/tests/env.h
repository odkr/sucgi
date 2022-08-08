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

#if !defined(SRC_TESTS_ENV_H)
#define SRC_TESTS_ENV_H

#include "../attr.h"
#include "../env.h"
#include "../err.h"


/*
 * Functions
 */

/* 
 * Create a new environment with space for at least n elements
 * and initialise with the given variadic arguments,
 * which must be strings.
 * 
 * Return code:
 * 	OK               Success.
 * 	ERR_SYS          System error. errno(2) should be set.
 */
error env_init(const size_t n, ...);


#endif /* !defined(SRC_TESTS_ENV_H) */
