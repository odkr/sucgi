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

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "file.h"
#include "err.h"
#include "str.h"


#if NAME_MAX > -1 && NAME_MAX < FILENAME_MAX
#define MAX_FNAME NAME_MAX
#else
#define MAX_FNAME FILENAME_MAX
#endif


error
path_check_len(const char *const path)
{
	size_t path_len = PATH_MAX + 1;
	const char *sep = NULL;
	const char *sub = NULL;

	assert(path);
	check(str_len(path, &path_len));
#if PATH_MAX > -1
	if (path_len > PATH_MAX - 1) return ERR_STR_MAX;
#endif

	sub = path;
	do {
		/* Flawfinder: ignore; str_cpn respects STR_MAX. */
		char super[STR_MAX] = "";	/* Super-directory. */
		size_t super_len = 0;		/* Super-dir path length. */
		size_t sub_len = 0;		/* Sub-dir path length. */
		size_t fname_len = 0;		/* Filename length .*/

		if (sep) {
			super_len = (size_t) (sep - path + 1);
			check(str_cpn(super_len, path, &super));
		}

		sep = strpbrk(sub, "/");
		if (sep) {
			fname_len = (size_t) (sep - sub) + 1;
			sub_len = path_len - super_len;
			sub = sep + 1;
		} else {
			fname_len = path_len - super_len;
			sub_len = fname_len;
			sub = NULL;
		}

		if (fname_len > MAX_FNAME) return ERR_FILE_NAME;

		if (!str_eq(super, "")) {
 			long name_max = pathconf(super, _PC_NAME_MAX);
			long path_max = pathconf(super, _PC_PATH_MAX);

			if (name_max < 0) {
				if (errno > 0) return ERR_SYS;
			} else if (fname_len > (size_t) name_max) {
				return ERR_FILE_NAME;
			}

			if (path_max < 0) {
				if (errno > 0) return ERR_SYS;
			} else if (sub_len > (size_t) path_max) {
				return ERR_STR_MAX;
			}
		}
	} while (sep);

	return OK;
}

error
path_check_wexcl(const uid_t uid, const char *const start,
                 const char *const stop)
{
	/* Flawfinder: ignore; str_cp respects STR_MAX. */
	char path[STR_MAX] = "";	/* Copy of path. */
	char *file = path;		/* Path to current file. */

	assert(start && stop);
	check(str_cp(start, &path));

	while (true) {
		struct stat fstatus;

		if (stat(file, &fstatus) != 0) return ERR_SYS;
		if (!file_is_wexcl(uid, &fstatus)) return ERR_FILE_WEXCL;
		if (str_eq(file, stop) ||
		    str_eq(file, "/")  ||
		    str_eq(file, ".")) break;
		file = dirname(file);
	}

	return OK;
}

bool
path_contains(const char *const super, const char *const sub)
{
	size_t len = 0;	/* Super-directory length. */

	assert(super && sub);
	len = strnlen(super, STR_MAX - 1);
	if (str_eq(super, "")) return false;
	if (str_eq(sub, "/")) return false;
	if (sub[len] != '/' && !str_eq(super, "/")) return false;
	return (strncmp(super, sub, len) == 0);
}
