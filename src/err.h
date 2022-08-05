/*
 * Header for err.c.
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

#include <stdlib.h>

#include "attr.h"


/*
 * Macros
 */

/* Return with err from the current function unless err is OK. */
#define check(err) 						\
	do {							\
		error _check_err = (err);			\
		if (_check_err != OK) return _check_err;	\
	} while (0)


/*
 * Data types
 */

typedef enum {
	OK = EXIT_SUCCESS,	/* Success. */
	ERR = EXIT_FAILURE,	/* Generic error. Should be used sparingly. */
	ERR_SYS,		/* System error. errno should be set. */
	ERR_CONV,		/* Conversion error. */
	ERR_FILE_NAME,		/* Filename too long. */
	ERR_FILE_TYPE,		/* Filetype is wrong. */
	ERR_FILE_WEXCL,		/* File not writable exclusively by user. */
	ERR_STR_MAX,		/* String too long. */
	ERR_VAR_MAX,		/* Too many environment variables. */
	ERR_VAR_EMPTY,		/* Environment variable is empty. */
	ERR_VAR_INVALID,	/* Environment variable is ill-formed. */
	ERR_VAR_UNDEF		/* Environment variable is undefined. */
} error;


/*
 * Functions
 */

/* Log message as an error and exit the programme with EXIT_FAILURE. */
/* Flawfinder: ignore; this is not a call to printf(3). */
__attribute__((format(printf, 1, 2), noreturn))
void fail(const char *const message, ...);


#endif /* !defined(SRC_ERR_H) */
