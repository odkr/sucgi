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

#if !defined(TESTS_LIB_USER_H)
#define TESTS_LIB_USER_H

#include <sys/types.h>

#include "../../attr.h"


/*
 * Data types
 */

typedef enum {
    USER_SUCCESS = 0,   /* Success. */
    USER_NOTFOUND,      /* A user was not found. */
    USER_ERROR,         /* An error occurred. */
} UserError;


/*
 * Functions
 */

/*
 * Return the user ID of a non-superuser in UID.
 *
 * Return value:
 *     USER_SUCCESS   Non-superuser was found.
 *     USER_NOTFOUND  No non-superuser was not found.
 *     USER_ERROR     getpwent failed, errno should be set.
 */
__attribute__((nonnull(1), warn_unused_result))
UserError usergetregular(uid_t *const uid);

/*
 * Return the passwd entry of the user whose ID is UID in PWD.
 *
 * Return value:
 *     USER_SUCCESS   User was found.
 *     USER_NOTFOUND  User was not found.
 *     USER_ERROR     malloc or getpwuid_r failed, errno should be set.
 */
__attribute__((nonnull(2)))
UserError userget(uid_t uid, struct passwd *pwd);

#endif /* !defined(TESTS_LIB_USER_H) */
