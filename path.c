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
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "file.h"
#include "error.h"
#include "path.h"
#include "str.h"


enum error
path_check_format(const char *fname,
                  /* RATS: ignore; cur is bounds-checked. */
                  char (*const exp)[MAX_STR], const char *format, ...)
{
	char *resolved;		/* Canonicalised path. */
	int n;			/* Size of expanded path. */
	int match;		/* Result of comparison. */
	va_list ap;		/* Argument pointer. */

	assert(*fname != '\0');
	assert(*format != '\0');
	assert(strnlen(fname, MAX_STR) < MAX_STR);
	assert(strnlen(format, MAX_STR) < MAX_STR);
	assert(access(fname, F_OK) == 0);
	/* RATS: ignore; this use of realpath should be safe. */
	assert(strncmp(realpath(fname, NULL), fname, MAX_STR) == 0);

	va_start(ap, format);
	n = vsnprintf(*exp, MAX_STR, format, ap);
	va_end(ap);

	if ((*exp)[n] != '\0') {
		return ERR_LEN;
	}

	/* RATS: ignore; this use of realpath should be safe. */
	resolved = realpath(*exp, NULL);
	if (!resolved) {
		return ERR_REALPATH;
	}

	match = strncmp(fname, resolved, MAX_STR);
	free(resolved);

	return (match == 0) ? OK : FAIL;
}

enum error
path_check_wexcl(const uid_t uid, const char *const par,
		 /* RATS: ignore; cur is bounds-checked. */
		 const char *const fname, char (*const cur)[MAX_STR])
{
	const char *p;		/* Current position in filename. */

	assert(*par != '\0');
	assert(*fname != '\0');
	assert(strnlen(par, MAX_STR) < MAX_STR);
	assert(strnlen(fname, MAX_STR) < MAX_STR);
	assert(access(par, F_OK) == 0);
	assert(access(fname, F_OK) == 0);
	/* RATS: ignore; this use of realpath should be safe. */
	assert(strncmp(realpath(par, NULL), par, MAX_STR) == 0);
	/* RATS: ignore; this use of realpath should be safe. */
	assert(strncmp(realpath(fname, NULL), fname, MAX_STR) == 0);

	/* FIXME: Not unit-tested. */
	if (!path_contains(par, fname)) {
		return ERR_ILL;
	}

	/*
	 * Start parsing after the parent-directory portion,
	 * unless the parent directory is the root directory.
	 */
	p = fname;
	if (strncmp(par, "/", 2) != 0) {
		/* RATS: ignore; par should be NUL-terminated. */
		p += strlen(par);
	}

	do {
		struct stat buf;	/* Current file's status. */
		size_t len;		/* Current filename's length. */
		int fd;			/* Current file. */
		int rc;			/* Return code. */

		/* 
		 * Move to the next path separator,
		 * but do not skip the root directory.
		 */
		p += (p == fname && *fname == '/') ? 1U : strcspn(p, "/");
		len = (size_t) (p - fname);
		if (len >= MAX_STR) {
			return ERR_LEN;
		}

		(void) str_cp(len, fname, *cur);

		try(file_safe_open(*cur, O_RDONLY | O_CLOEXEC, &fd));

		errno = 0;
		rc = fstat(fd, &buf);
		
		if (close(fd) != 0) {
			return ERR_CLOSE;
		}
		if (rc != 0) {
			return ERR_STAT;
		}
		if (!file_is_wexcl(uid, buf)) {
			return FAIL;
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
	assert(strnlen(par, MAX_STR) < MAX_STR);
	assert(strnlen(fname, MAX_STR) < MAX_STR);

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

	/* Check if fname resides in par. */
	return fname[len] == '/' && strncmp(par, fname, len) == 0;

}
