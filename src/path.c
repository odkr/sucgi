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
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "file.h"
#include "err.h"
#include "str.h"


error
path_check_len(const char *const path)
{
	size_t len = 0;			/* Length of current (sub-)dir. */
	const char *super = NULL;	/* Currot super-directory. */
	const char *sub = NULL;		/* Current sub-direcotry. */
	char *pivot = NULL;		/* Current super/sub border. */	

	assert(path);
	len = strnlen(path, STR_MAX_LEN + 1);
	if (len > STR_MAX_LEN) return ERR_STR_LEN;

#if PATH_MAX > -1
	if (len > PATH_MAX - 1) return ERR_STR_LEN;
#endif

	reraise(str_cp(path, &pivot));
	while ((pivot = strpbrk(pivot, "/"))) {
		if (super && sub) {
			long max = pathconf(super, _PC_PATH_MAX);

			if (max < 0) {
				if (errno > 0) return ERR_SYS;
			} else if (strnlen(sub, STR_MAX_LEN) > (size_t) max) {
				return ERR_STR_LEN;
			}
		}
		super = strndup(path, (size_t) (pivot - path + 1));
		sub = ++pivot;
	}
}

error
path_check_wexcl(const uid_t uid, const char *const path,
                 const char *const stop)
{
	char *p = NULL;	/* Current path. */

	assert(path && stop);
	reraise(str_cp(path, &p));
	while (true) {
		struct stat fstatus;
		if (stat(p, &fstatus) != 0) return ERR_SYS;
		if (!file_is_wexcl(uid, &fstatus)) return ERR_NOT_EXCLW;
		if (str_eq(p, stop) || str_eq(p, "/") || str_eq(p, ".")) break;
		p = dirname(p);
	}

	return OK;
}

bool
path_contains(const char *const super, const char *const sub)
{
	size_t len = 0;

	assert(super && sub);
	len = strnlen(super, STR_MAX_LEN);
	if (super[0] == '\0') return false;
	if (str_eq(sub, "/")) return false;
	if (sub[len] != '/' && !str_eq(super, "/")) return false;
	return (strncmp(super, sub, len) == 0);
}
