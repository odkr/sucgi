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

#include "defs.h"
#include "err.h"


/*
 * Data types
 */

/* A simple key-value store. */
struct scpt_ent {
	const char *const suffix;
	const char *const handler;
};


/*
 * Functions
 */

/*
 * Find a script handler matching the filename suffix of SCPT in the array
 * of filename suffix-script handler pairs HANDLERDB and store it in HANDLER.
 *
 * Return code:
 *      OK                 Success.
 *      ERR_SCPT_NO_HDL    No handler registered. 
 *      ERR_SCPT_ONLY_SFX  Filename starts with a dot (".").
 *      ERR_SCPT_NO_SFX    Filename has no filename suffix.
 *      ERR_STR_LEN        Filename is longer than STR_MAX - 1 bytes.
 */
__attribute__((nonnull(1, 2, 3), pure, warn_unused_result))
enum error scpt_get_handler(const struct scpt_ent handlerdb[],
                            const char *const scpt,
                            const char **const handler);


#endif /* !defined(SCPT_H) */
