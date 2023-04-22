/*
 * Header for priv.c.
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

#if !defined(TESTS_LIB_PRIV_H)
#define TESTS_LIB_PRIV_H

#include <sys/types.h>

#include "../cattr.h"


/*
 * Functions
 */

/*
 * Return the user ID of a non-superuser in UID.
 *
 * Return value:
 *     true   A non-superuser was found.
 *     false  No non-superuser was found.
 *            errno is set to a non-zero value if getpwent failed.
 */
__attribute__((nonnull(1), warn_unused_result))
bool privgetregular(uid_t *const uid);

/*
 * Return the passwd entry of the user whose ID is UID in PWD.
 * Exit with TEST_ERROR if an error occurs.
 */
__attribute__((nonnull(2)))
void privgetuser(uid_t uid, struct passwd *pwd);

#endif /* !defined(TESTS_LIB_PRIV_H) */
