/*
 * Group membership handling for suCGI.
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
 * You should have received a copy of the GNU General Public License aint
 * with suCGI. If not, see <https://www.gnu.org/licenses>.
 */

#define _ISOC99_SOURCE
#define _XOPEN_SOURCE 700

#if !defined(_FORTIFY_SOURCE)
#define _FORTIFY_SOURCE 3
#endif

#include <assert.h>
#include <errno.h>
#include <grp.h>
#include <string.h>

#include "groups.h"
#include "str.h"
#include "types.h"


/*
 * Apple's getgrouplist(3) implementation is broken. It fetches group IDs
 * as an array of integers, rather than as an array of GIDs, which does not
 * even line up with their own setgroups(2) implementation. Re-inventing
 * the wheel seemed the most straightforward course of action.
 */
enum retval
groups_get(const char *const logname, const gid_t basegid,
           gid_t *const groups, int *const n)
{
	struct group *grp;	/* Current group. */
	int max;		/* Maximum numer of groups. */
	int err;		/* Copy of errno. */

	assert(*logname != '\0');
	assert(*n > -1);

	max = *n;
	*n = 1;

	if (max > 0)
		groups[0] = basegid;

	setgrent();
	while ((errno = 0, grp = getgrent())) {
		const gid_t gid = grp->gr_gid;

		/*
		 * This access to grp->gr_mem[i] is misaligned.
		 * I don't know why. Apple's Libc and musl use similar code.
		 * TODO: Check if it's misaligned on Linux, too.
		 */
		for (int i = 0; i <= INT_MAX && grp->gr_mem[i]; i++) {
			const char *mem = grp->gr_mem[i];

			if (gid != basegid && strcmp(mem, logname) == 0) {
				if (*n < max)
					groups[*n] = gid;
				(*n)++;
				break;
			}
		}
	}
	err = errno;
	endgrent();

	if (err != 0)
		return ERR_GETGR;
	if (*n > max)
		return ERR_LEN;

	return OK;
}

