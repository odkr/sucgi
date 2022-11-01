/*
 * Privilege handling for suCGI.
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

#if !defined(_FORTIFY_SOURCE)
#define _FORTIFY_SOURCE 3
#endif /* !defined(_FORTIFY_SOURCE) */

#include <assert.h>
#include <grp.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <unistd.h>

#include "error.h"
#include "priv.h"


enum error
priv_drop(const uid_t uid, const gid_t gid,
          const int ngids, const gid_t gids[ngids])
{
	errno = 0;

	if (setgroups(ngids, gids) != 0) {
		return ERR_SETGROUPS;
	}

	if (setgid(gid) != 0) {
		return ERR_SETGID;
	}

	if (setuid(uid) != 0) {
		return ERR_SETUID;
	}
	
	if (setgroups(1, (gid_t [1]) {0}) != -1) {
		return FAIL;
	}

	if (setgid(0) != -1) {
		return FAIL;
	}
	
	if (setuid(0) != -1) {
		return FAIL;
	}

	return OK;
}
