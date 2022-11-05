/*
 * User directory handling for suCGI.
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

#define _ISOC99_SOURCE
#define _POSIX_C_SOURCE 200809L

#if !defined(_FORTIFY_SOURCE)
#define _FORTIFY_SOURCE 3
#endif

#include <assert.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "str.h"
#include "types.h"
#include "userdir.h"


enum retval
userdir_resolve(const char *const s, const struct passwd *user,
                char **user_dir)
{
	char *expan;		/* Expanded string. */
	char *resolved;		/* Resolved path. */
	int len;		/* Length of expanded string. */

	assert(*s != '\0');

	expan = (char *) malloc(PATH_SIZE * sizeof(char *)) ;
	if (!expan)
		return ERR_MEM;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
	if (*s != '/')
		len = snprintf(expan, PATH_SIZE, "%s/%s", user->pw_dir, s);
	else if (strstr(s, "%s"))
		/* RATS: ignore; format is controlled by the administrator. */
		len = snprintf(expan, PATH_SIZE, s, user->pw_name);
	else
		len = snprintf(expan, PATH_SIZE, "%s/%s", s, user->pw_name);
#pragma GCC diagnostic pop

	if (len < 0) {
		free(expan);
		return ERR_PRN;
	}
	if ((size_t) len >= PATH_SIZE) {
		free(expan);
		return ERR_LEN;
	}

	*user_dir = expan;

	/* RATS: ignore; the length of expan is checked above. */
	resolved = realpath(expan, NULL);
	if (!resolved)
		return ERR_RES;

	*user_dir = resolved;
	free(expan);

	return OK;
}
