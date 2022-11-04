/*
 * Data types for suCGI
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

#if !defined(TYPES_H)
#define TYPES_H

/* A simple key-value store. */
struct pair {
	const char *const key;
	const char *const value;
};

/* Return code for functions. */
enum retcode {
	OK = 0,		/* Success. */
	FAIL,		/* Generic failure. */
	ERR_CNV,	/* Conversion error. */
	ERR_NIL,	/* No input. */
	ERR_LEN,	/* Input is out-of-bounds. */
	ERR_ILL,	/* Input is ill-formed. */
	ERR_MEM,	/* calloc(3) failed. */
	ERR_OPEN,	/* open(2)/openat2(2) failed. */
	ERR_CLOSE,	/* close(2) failed. */
	ERR_RES,	/* realpath(3) failed. */
	ERR_STAT,	/* stat(2) failed. */
	ERR_PRN,	/* printf(3) failed. */
	ERR_ENV,	/* getenv(3) or setenv(3) failed. */
	ERR_GETGR,	/* getgrent(3) failed. */
	ERR_SETUID,	/* setuid(2) failed. */
	ERR_SETGID,	/* setgid(2) failed. */
	ERR_SETGRPS	/* setgroups(2) failed. */
};


#endif /* !defined(TYPES_H) */

