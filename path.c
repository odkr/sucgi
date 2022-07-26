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
#define _XOPEN_SOURCE 700

#if !defined(_FORTIFY_SOURCE)
#define _FORTIFY_SOURCE 3
#endif

#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "file.h"
#include "max.h"
#include "path.h"
#include "str.h"
#include "types.h"

enum retval
path_check_wexcl(const uid_t uid, const char *const fname,
                 const char *const parent, char cur[MAX_FNAME])
{
	const char *pos;	/* Current position in filename. */

	assert(*parent != '\0');
	assert(*fname != '\0');
	assert(strnlen(parent, MAX_FNAME) < MAX_FNAME);
	assert(strnlen(fname, MAX_FNAME) < MAX_FNAME);
	/* RATS: ignore; not a permission check. */
	assert(access(parent, F_OK) == 0);
	/* RATS: ignore; not a permission check. */
	assert(access(fname, F_OK) == 0);
	/* RATS: ignore; the length of parent is checked above. */
	assert(strncmp(realpath(parent, NULL), parent, MAX_FNAME) == 0);
	/* RATS: ignore; the length of fname is checked avove. */
	assert(strncmp(realpath(fname, NULL), fname, MAX_FNAME) == 0);
	assert(path_is_subdir(fname, parent));

	/* RATS: ignore; parent should be NUL-terminated. */
	pos = fname + strlen(parent);
	do {
		struct stat fstatus;	/* Current file's status. */
		int fd;			/* Current file. */
		int err;		/* stat error. */
		enum retval rc;		/* file_sopen return code. */

		(void) str_cp((size_t) (pos - fname), fname, cur);

		rc = file_sec_open(cur, O_RDONLY, &fd);
		if (rc != OK)
			return rc;

		errno = 0;
		err = fstat(fd, &fstatus);
		
		if (close(fd) != 0)
			return ERR_CLOSE;
		if (err != 0)
			return ERR_STAT;
		if (!file_is_wexcl(uid, fstatus))
			return FAIL;

		/* Move past the current path separator ... */
		pos += strspn(pos, "/");
		if (!*pos)
			break;

		/* ... and to the next one/the end of fname. */
		pos += strcspn(pos, "/");
	} while (true);

	return OK;
}

bool
path_is_subdir(const char *const fname, const char *const parent)
{
	size_t len;		/* Parent-directory length. */

	assert(*parent != '\0');
	assert(*fname != '\0');
	assert(strnlen(parent, MAX_FNAME) < MAX_FNAME);
	assert(strnlen(fname, MAX_FNAME) < MAX_FNAME);

	if (strncmp(fname, "/", 2) == 0)
		return false;
	if (strncmp(parent, "/", 2) == 0 && *fname == '/')
		return true;
	if (strncmp(fname, ".", 2) == 0)
		return false;
	if (strncmp(parent, ".", 2) == 0 && *fname != '/')
		return true;

	/* RATS: ignore; parent should be NUL-terminated. */
	len = strlen(parent);
	return (fname[len] == '/') && strncmp(parent, fname, len) == 0;
}
