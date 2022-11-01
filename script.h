/*
 * Header for scpt.c.
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

#if !defined(SCPT_H)
#define SCPT_H

#include "macros.h"
#include "error.h"
#include "str.h"


/*
 * Data types
 */

/* A simple key-value store. */
struct pair {
	const char *const key;
	const char *const value;
};


/*
 * Functions
 */

/*
 * Find a script interpreter matching the filename suffix of SCRIPT in the
 * array of filename suffix-interpreter pairs DB and store it in INTER.
 *
 * Return code:
 *      OK        Success.
 *      ERR_LEN   FNAME is longer than MAX_STR - 1 bytes.
 *      ERR_ILL   FNAME has no filename suffix.
 *      FAIL      No handler has been registered for FNAME's suffix.
 */
__attribute__((nonnull(1, 2, 3), pure, warn_unused_result))
enum error script_get_inter(const struct pair db[],
                            const char *const script,
                            char (*const inter)[MAX_STR]);


#endif /* !defined(SCPT_H) */
