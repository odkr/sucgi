/*
 * Marcros for error handling.
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

#if !defined(INCLUDED_ERR)
#define INCLUDED_ERR

#include <stdio.h>
#include <stdlib.h>

#include "attr.h"


/*
 * Macros
 */

/* Return with rc from the current function if rc is not OK. */
#define reraise(rc) 						\
	do {							\
		enum code _reraise_rc = (rc);			\
		if (_reraise_rc != OK) return _reraise_rc;	\
	} while (0)



/*
 * Data types
 */

enum code {
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
};


/*
 * Functions
 */

/* Log message as an error and exit the programme with EXIT_FAILURE. */
// This is not a call to the access() function.
// flawfinder: ignore
__attribute__((noreturn, access(read_only, 1), format(printf, 1, 2)))
void error(const char *const message, ...);


#endif /* Include guard. */
