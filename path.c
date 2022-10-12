/*
 * Path handling for suCGI.
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
#endif /* !defined(_FORTIFY_SOURCE) */

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "file.h"
#include "error.h"
#include "path.h"
#include "str.h"


enum error
path_check_wexcl(const uid_t uid, const char *const par,
                 /* RATS: ignore; cur is bounds-checked. */
                 const char *const fname, char (*const cur)[STR_MAX])
{
	const char *p;		/* Current position in filename. */

	assert(*par != '\0');
	assert(*fname != '\0');
	assert(strnlen(par, STR_MAX) < STR_MAX);
	assert(strnlen(fname, STR_MAX) < STR_MAX);
        /* RATS: ignore; this use of realpath should be safe. */
        assert(strncmp(realpath(par, NULL), par, STR_MAX) == 0);
        /* RATS: ignore; this use of realpath should be safe. */
	assert(strncmp(realpath(fname, NULL), fname, STR_MAX) == 0);

	/* FIXME: Not unit-tested. */
	if (!path_contains(par, fname)) {
		return ERR_PATH_OUT;
	}

	/* RATS: ignore; par should be NUL-terminated. */
	p = fname + ((strncmp(par, "/", 2) == 0) ? 0U : strlen(par));
	do {
		struct stat buf;	/* Current file's status. */
		size_t len;		/* Current flename's length. */

		/* 
		 * Move to next path separator,
		 * but do not skip the root directory.
		 */
		p += (p == fname && *fname == '/') ? 1U : strcspn(p, "/");
		len = (size_t) (p - fname);
		if (len >= STR_MAX) {
			return ERR_STR_LEN;
		}

		(void) str_cp(len, fname, *cur);

		try(file_safe_stat(*cur, &buf));
		if (!file_is_wexcl(uid, buf)) {
			return ERR_PATH_WEXCL;
		}

		p += strspn(p, "/");
	} while (*p != '\0');

	return OK;
}

bool
path_contains(const char *const par, const char *const fname)
{
	size_t len;		/* Parent-directory length. */

	assert(*par != '\0');
	assert(*fname != '\0');
	assert(strnlen(par, STR_MAX) < STR_MAX);
	assert(strnlen(fname, STR_MAX) < STR_MAX);

	/* The root directory cannot be contained by any path. */
	if (strncmp(fname, "/", 2) == 0) {
		return false;
	}

	/* The root directory contains any absolute path, save for itself. */
	if (strncmp(par, "/", 2) == 0 && *fname == '/') {
		return true;
	}

	/* By fiat, the working directory cannot be contained by any path. */
	if (strncmp(fname, ".", 2) == 0) {
		return false;
	}

	/* The working directory contains any relative path. */
	if (strncmp(par, ".", 2) == 0 && *fname != '/') {
		return true;
	}

	/* RATS: ignore; path_contains should only receive sane paths. */
	len = strlen(par);
	if (fname[len] != '/') {
		return false;
	}
	
	return strncmp(par, fname, len) == 0;
}
