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

#if !defined(_FORTIFY_SOURCE)
#define _FORTIFY_SOURCE 3
#endif

#include <sys/types.h>
#include <assert.h>
#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <unistd.h>

#include "priv.h"
#include "types.h"


/*
 * Functions
 */

enum retval
priv_drop(const uid_t uid, const gid_t gid,
          const int ngids, const gid_t gids[ngids])
{
	assert(ngids > -1);

/*
 * Some older setgroups implementations assume that GIDs are of the type
 * int, rather than type gid_t. However, gid_t is typically an alias for
 * unsigned int, so that both types are of the same size. Moreover, suCGI
 * requires that GIDs are smaller than INT_MAX. So this is a distinction
 * without a difference. The most straightforward way to deal with older
 * setgroups implementations is, therefore, to tell the compiler to
 * ignore the signedness incongruency.
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wpointer-sign"
	errno = 0;
	if (setgroups(ngids, gids) != 0)
		return ERR_SETGRPS;
#pragma GCC diagnostic pop

	errno = 0;
	if (setgid(gid) != 0)
		return ERR_SETGID;
	errno = 0;
	if (setuid(uid) != 0)
		return ERR_SETUID;

	if (setgroups(1, (gid_t [1]) {0}) != -1)
		return FAIL;
	if (setgid(0) != -1)
		return FAIL;
	if (setuid(0) != -1)
		return FAIL;

	return OK;
}
