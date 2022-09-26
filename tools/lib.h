/* Headers for lib.c
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

#if !defined(TOOLS_LIB_H)
#define TOOLS_LIB_H

#include "../defs.h"
#include "../env.h"
#include "../err.h"


/*
 * Macros
 */

/*
 * Convert N to a string literal.
 */
#define TOSTRING(n) TOSTRING_(n)
#define TOSTRING_(n) #n

/*
 * Prefix MESSAGE with the name of the current file and a line number,
 * then print it to STDERR and exit with status EXIT_FAILURE.
 */
#define croak(...) die(__FILE__ ":" TOSTRING(__LINE__) ": " __VA_ARGS__)

/*
 * Abort the programme with the given MESSAGE unless COND is true.
 */
#define req(cond, ...) do { if (!(cond)) croak(__VA_ARGS__); } while (0)


/*
 * Functions
 */

/*
 * Print MESSAGE to STDERR and exit with status EXIT_FAILURE.
 */
__attribute__((format(printf, 1, 2), nonnull(1), noreturn))
void die(const char *const message, ...);

/*
 * Covert S to an ID and store its value in the variable pointed to by ID.
 *
 * Return code:
 *      SC_OK       Success.
 *      SC_ERR_CNV  S could not be converted.
 *      SC_ERR_SYS  System error. errno(2) should be set.
 */
__attribute__((warn_unused_result))
enum error str_to_id (const char *const s, id_t *id);


#endif /* !defined(TOOLS_LIB_H) */
