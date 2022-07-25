/*
 * Marcros for fail handling.
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

#if !defined(SRC_ERR_H)
#define SRC_ERR_H

#include <stdio.h>
#include <stdlib.h>

#include "attr.h"


/*
 * Macros
 */

/* Return with err from the current function unless err is OK. */
#define reraise(err) 						\
	do {							\
		error _reraise_err = (err);			\
		if (_reraise_err != OK) return _reraise_err;	\
	} while (0)



/*
 * Data types
 */

typedef enum {
	/* Success. */
	OK = EXIT_SUCCESS,
	/* Generic error. Should only be used to initialise variables. */
	ERR = EXIT_FAILURE,
	/* Too many environment variables. */
	ERR_ENV_MAX,
	/* A file is not exclusively writably by the given user/group. */
	ERR_NOT_EXCLW,
	/* A string is too long. */
	ERR_STR_LEN,
	/* A system call failed. errno should be set. */
	ERR_SYS,
	/* An environment variable is the empty string. */
	ERR_VAR_EMPTY,
	/* An environment variable is ill-formed. */
	ERR_VAR_INVALID,
	/* An environment variable is undefined. */
	ERR_VAR_UNDEF
} error;


/*
 * Functions
 */

/* Log message as an fail and exit the programme with EXIT_FAILURE. */
// message is declared constant.
// flawfinder: igore.
__attribute__((noreturn, ACCESS_RO(1), format(printf, 1, 2)))
void fail(const char *const message, ...);


#endif /* !defined(SRC_ERR_H) */
