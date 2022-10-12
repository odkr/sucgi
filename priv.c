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
#define _POSIX_C_SOURCE 200809L
#define _BSD_SOURCE
#define _DARWIN_C_SOURCE
#define _DEFAULT_SOURCE

#if !defined(_FORTIFY_SOURCE)
#define _FORTIFY_SOURCE 3
#endif /* !defined(_FORTIFY_SOURCE) */

#include <assert.h>
#include <limits.h>
#include <unistd.h>

#include "error.h"
#include "priv.h"


enum error
priv_drop(const uid_t uid, const gid_t gid,
          const int ngids, const gid_t gids[ngids])
{
	if (setgroups(ngids, gids) != 0) {
		return ERR_SYS;
	}
	if (setgid(gid) != 0) {
		return ERR_SYS;
	}
	if (setuid(uid) != 0) {
		return ERR_SYS;
	}
	
	if (setgroups(1, (gid_t [1]) {gid}) != -1) {
		return ERR_PRIV;
	}
	if (setgid(0) != -1) {
		return ERR_PRIV;
	}
	if (setuid(0) != -1) {
		return ERR_PRIV;
	}

	return OK;
}
