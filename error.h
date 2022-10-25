/*
 * Header for error.c.
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

#if !defined(ERROR_H)
#define ERROR_H

#include "macros.h"


/*
 * Macros
 */

/* Return with RC from the current function unless RC equals OK. */
#define try(rc) 					\
	do { 						\
		enum error _try_rc = (rc);		\
		if (_try_rc != OK) return _try_rc;	\
	} while (0)


/*
 * Data types
 */

enum error {
	OK = 0,		/* Success. */
	FAIL,		/* Generic failure. */
	ERR_CNV,	/* Conversion error. */
	ERR_NIL,	/* No input. */
	ERR_LEN,	/* Input is out-of-bounds. */
	ERR_ILL,	/* Input is ill-formed. */
	ERR_CALLOC,	/* calloc(3) failed. */
	ERR_OPEN,	/* open(2)/openat2(2) failed. */
	ERR_CLOSE,	/* close(2) failed. */
	ERR_REALPATH,	/* realpath(3) failed. */
	ERR_STAT,	/* stat(2) failed. */
	ERR_GETENV,	/* getenv(3) failed. */
	ERR_SETENV,	/* setenv(3) failed. */
	ERR_GETGRENT,	/* getgrent(3) failed. */
	ERR_SETUID,	/* setuid(2) failed. */
	ERR_SETGID,	/* setgid(2) failed. */
	ERR_SETGROUPS	/* setgroups(2) failed. */
};


/*
 * Functions
 */

/* Log MESSAGE as an error and exit with status EXIT_FAILURE. */
__attribute__((nonnull(1), noreturn))
void error(const char *const message, ...);


#endif /* !defined(ERROR_H) */
