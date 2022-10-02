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

#if !defined(ERR_H)
#define ERR_H

#include <stdlib.h>

#include "defs.h"


/*
 * Macros
 */

/* Return with RC from the current function unless RC equals OK. */
#define try(rc) STMT( enum error _try = (rc); if (_try != OK) return _try; )


/*
 * Data types
 */

enum error {
	OK = 0,		        /* Success. */
	ERR_SYS,		/* System error. errno should be set. */
	ERR_CNV,		/* Value could not be converted. */
	ERR_FTYPE,		/* Filetype is wrong. */
	ERR_GIDS_MAX,	        /* User belongs to too many groups. */
	ERR_PATH_OUT,	        /* A file is outside of a given path. */
	ERR_PATH_WEXCL,	        /* Path not exclusively writable by user. */
	ERR_PRIV,		/* Privileges could be resumed. */
	ERR_SCPT_NO_HDL,	/* No script handler registered. */
	ERR_SCPT_NO_SFX,	/* Filename has no suffix. */
	ERR_SCPT_ONLY_SFX,	/* Filename starts with a dot. */
	ERR_STR_LEN,		/* String is too long. */
	ERR_ENV_LEN,		/* Too long environment variable. */
	ERR_ENV_MAL,		/* Malformed environment variable. */
	ERR_ENV_MAX,		/* Too many environment variables. */
	ERR_ENV_NIL		/* Unset or empty environment variable. */
};


/*
 * Functions
 */

/* Log MESSAGE as an error and exit with status EXIT_FAILURE. */
/* RATS: ignore; not a call to printf(3). */
__attribute__((format(printf, 1, 2), nonnull(1), noreturn))
void error(const char *const message, ...);


#endif /* !defined(ERR_H) */
