/*
 * Header for user.c.
 *
 * Copyright 2023 Odin Kroeger.
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

#if !defined(TESTS_UTIL_USER_H)
#define TESTS_UTIL_USER_H

#include <sys/types.h>
#include <pwd.h>

#include "../../attr.h"
#include "types.h"


/* NOLINTBEGIN(bugprone-reserved-identifier,cert-dcl37-c); see attr.h. */

#if defined(__GNUC__) && \
    (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 9)) && \
    !defined(__INTEL_COMPILER)
#define _returns_nonnull __attribute__((returns_nonnull))
#else
#define _returns_nonnull
#endif

/* NOLINTEND(bugprone-reserved-identifier,cert-dcl37-c) */


/*
 * Get the primary GID of the given user and return it in "gid".
 *
 * user_get_gid also takes a pointer to an error handling function. If an
 * error occurs, this function is called after clean-up; if that pointer
 * is NULL, error handling is left to the caller. However, although it does
 * count an error if the given user cannot be found, and a non-zero value
 * will ber returned, the error handler will *not* be called for that error.
 *
 * Return value:
 *     Zero      Success.
 *     Non-zero  Something went wrong; errno may be set.
 */
_write_only(2) _nonnull(2)
int user_get_gid(uid_t uid, gid_t *gid, ErrorFn errh);

/*
 * Search for a non-superuser and return it in "pwd".
 *
 * user_get_regular also takes a pointer to an error handling function.
 * See user_get_gid for details. However, although it is weird if no
 * non-superuser can be found, this does *not* count as an error, and
 * the error handler will *not* be called for that condition.
 *
 * Return value:
 *     Negative  Something went wrong; errno should be set.
 *     Zero      A non-superuser was found.
 *     Positive  No non-superuser.
 *
 * Errors:
 *     See setpwent, getpwent, and endpwent.
 */
_write_only(1) _nonnull(1)
int user_get_regular(struct passwd *pwd, ErrorFn errh);

/*
 * Convert the given ID into a string and return a pointer to that string.
 * The memory pointed to should be freed by the caller.
 *
 * user_id_to_str also takes a pointer to an error handling function.
 * See user_get_gid for details.
 *
 * If user_id_to_str succeeds, it resets errno to the value it
 * had when user_id_to_str was called, so that it can be used
 * for composing error messages.
 *
 * Return value:
 *     NULL      Conversion failed; errno should be set.
 *     Non-NULL  Pointer to the converted ID.
 */
_returns_nonnull _nodiscard
char *user_id_to_str(id_t id, ErrorFn errh);


#endif /* !defined(TESTS_UTIL_USER_H) */
