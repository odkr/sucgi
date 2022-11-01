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

#if !defined(_FORTIFY_SOURCE)
#define _FORTIFY_SOURCE 3
#endif /* !defined(_FORTIFY_SOURCE) */

#include <errno.h>
#include <grp.h>
#include <string.h>

#include "str.h"
#include "gids.h"
#include "error.h"


/*
 * getgrouplist(3) is neither in POSIX.1-2008 nor in 4.4BSD, and its
 * implementations differ. So re-inventing the wheel seemed the most
 * straightforward course of action.
 *
 * Note, the function signature and the semantics of gids_get_list
 * differ from those of getgrouplist(3), if subtly.
 */
enum error
gids_get_list(const char *const logname, const gid_t basegid,
              gid_t (*const gids)[MAX_GROUPS], int *const ngids)
{
	struct group *grp;	/* A group. */

	(*gids)[0] = basegid;
	*ngids = 1;

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
				if (*ngids < MAX_GROUPS)
					(*gids)[*ngids] = gid;
				(*ngids)++;
				break;
			}
		}

	}
	endgrent();

	if (errno != 0)
		return ERR_GETGRENT;
	if (*ngids > MAX_GROUPS)
		return ERR_LEN;

	return OK;
}

