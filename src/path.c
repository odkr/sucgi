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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "file.h"
#include "err.h"
#include "str.h"
#include "path.h"


/*
 * Check if LEN is smaller than the limit PC of the filesystem
 * mounted at PATH, where PC is a pathconf(2) system variable.
 * 
 * Return code:
 *      OK             LEN is within the limit.
 *      ERR_CONV       The limit returned by pathconf is too large.
 *      ERR_FILE_NAME  LEN exceeds the limit.
 */
static error
pc_check_len(const size_t len, const int pc, const char *const path)
{
	/* RATS: ignore; this TOCTOU gap is harmless. */
 	long max = pathconf(path, pc);

	if (errno != 0) return ERR_SYS;
	if (max < 0L) return OK;
	if ((uint64_t) max > (uint64_t) SIZE_MAX) return ERR_CONV;
	if (len > (size_t) max) return ERR_FILE_NAME;

	return OK;
}

error
path_check_len(const char *const path)
{
	/* RATS: ignore; str_cpn respects STR_MAX. */
	char super[STR_MAX] = {0};	/* Current super-path. */
	const char *sub = path;		/* Current sub-path. */
	const char *sep = NULL;		/* Position of current separator. */
	size_t path_len = 0U;		/* Overall path length. */
	size_t super_len = 0U;		/* Super-path length. */
	size_t sub_len = 0U;		/* Sub-path length. */
	size_t fname_len = 0U;		/* Filename length .*/

	/* Check the length of the given path against the OS limit. */
	assert(path);
	check(str_len(path, &path_len));

#if PATH_MAX > -1
	if (path_len > (size_t) PATH_MAX - 1U) return ERR_STR_MAX;
#endif

	/* Check each filename and each sub-path. */
	do {
		/* Get the next path seperator, if any. */
		sep = strpbrk(sub, "/");

		/*
		 * Calculate the length of the current sub-path 
		 * as well as that of the current filename. 
		 *
		 * Nota bene:
		 *      - The first 'sub'-path is the given path as a whole.
		 *      - super and super_len are initialised to the empty
		 *        string and 0 respectively, since the path as a
		 *        whole does not have a super-path.
		 *      - super, super_len, and sub are updated at the end
		 *        of the loop, from where they carry over.
		 *      - One byte is subtracted from the length of the
		 *        sub-path to account for the trailing "/".
		 */
		sub_len = path_len - super_len - 1U;
		fname_len = (sep != NULL) ? (size_t) (sep - sub) : sub_len;

		/* Check the length of the filename against the OS limit. */
#if -1 < NAME_MAX && NAME_MAX < FILENAME_MAX
		if (fname_len > (size_t) NAME_MAX) return ERR_FILE_NAME;
#else
		if (fname_len > (size_t) FILENAME_MAX) return ERR_FILE_NAME;
#endif

		/*
		 * If there is a super-path, check the length of the sub-path
		 * and that of the filename against the filesystem limits.
		 */
		if (*super != '\0') {
			check(pc_check_len(fname_len, _PC_NAME_MAX, super));
			check(pc_check_len(sub_len, _PC_PATH_MAX, super));
		}

		/* Exit if there are no more path segments. */
		if (sep == NULL) break;

		/* Move the pointer to the current sub-path forward. */
		sub = sep + 1U;

		/* Exit if the end of the string has been reached. */
		if (*sub == '\0') break;

		/* Calculate the super-path for the next iteration. */		
		if (sep == path) {
			super_len = 1;
			check(str_cp("/", &super));
		} else {
			super_len = (size_t) (sep - path);
			check(str_cpn(super_len, path, &super));
		}
	} while (true);

	return OK;
}

error
path_check_wexcl(const uid_t uid, const char *const start,
                 const char *const stop)
{
	/* RATS: ignore; str_cp respects STR_MAX. */
	char path[STR_MAX] = {0};	/* Copy of path. */
	char *file = path;		/* Path to current file. */

	assert(start && stop);
	check(str_cp(start, &path));

	while (true) {
		struct stat buf;	/* Status of the current file. */

		/* RATS: ignore; this TOCTOU gap is harmless. */
		if (stat(file, &buf) != 0) return ERR_SYS;
		if (!file_is_wexcl(uid, &buf)) return ERR_FILE_WEXCL;
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
	size_t len = 0U;	/* Super-directory length. */

	assert(super && (*super != '\0') && sub && (*sub != '\0'));
	len = strnlen(super, STR_MAX - 1U);
	if (str_eq(sub, "/")) return false;
	if ((sub[len] != '/') && (!str_eq(super, "/"))) return false;
	return (strncmp(super, sub, len) == 0);
}
