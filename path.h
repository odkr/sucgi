/*
 * Headers for path.c.
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

#if !defined(PATH_H)
#define PATH_H

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

#include "macros.h"
#include "error.h"
#include "str.h"

/*
 * Expand FORMAT with the given variadic arguments, store the resulting
 * filename in EXP, canonicalise it, and then compare it to FNAME.
 *
 * FNAME must be sanitised and canonical.
 *
 * Return code:
 *      OK            FNAME matches the expanded filename.
 *      ERR_REALPATH  realpath(3) failed.
 *      ERR_LEN       The expanded filename is longer than MAX_STR - 1 bytes.
 *      FAIL          FNAME does not match EXP.
 *
 * FIXME: Not unit-tested.
 */
__attribute__((nonnull(1, 2, 3), format(printf, 3, 4), warn_unused_result))
enum error path_check_format(const char *fname,
                             /* RATS: ignore; exp is bounds-checked. */
			     char (*const exp)[MAX_STR],
                             const char *format, ...);

/*
 * Check if the user with ID UID has exclusive write access to the file
 * named FNAME and each parent directory of FNAME up to PAR and store a
 * copy of the last path checked in CUR.
 *
 * PAR and FNAME must be sanitised and canonical.
 *
 * Return code:
 *      OK          Success.
 *      ERR_CNV*    File descriptor is too large (Linux only).
 *      ERR_ILL     FNAME is not within PAR.
 *      ERR_LEN     FNAME is too long.
 *      ERR_OPEN    open(2) or openat2(2) failed.
 *      ERR_CLOSE   close(2) failed.
 *      ERR_STAT    stat(2) failed.
 *      FAIL        UID does not have exclusive write access to FNAME.
 *
 *      Errors marked with an asterisk should be impossible.
 */
__attribute__((nonnull(2, 3, 4), warn_unused_result))
enum error path_check_wexcl(const uid_t uid, const char *const par,
                            const char *const fname,
			    /* RATS: ignore; cur is bounds-checked. */
                            char (*const cur)[MAX_STR]);

/*
 * Check if the path PAR names a super-directory of the file named FNAME.
 *
 * PAR and FNAME must be sanitisied and canonical.
 */
__attribute__((nonnull(1, 2), pure, warn_unused_result))
bool path_contains(const char *const par, const char *const fname);


#endif /* !defined(PATH_H) */
