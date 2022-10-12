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
 * You should have received a copy of the GNU General Public License along
 * with suCGI. If not, see <https://www.gnu.org/licenses>.
 */

#define _ISOC99_SOURCE
#define _POSIX_C_SOURCE 200809L

#if !defined(_FORTIFY_SOURCE)
#define _FORTIFY_SOURCE 3
#endif /* !defined(_FORTIFY_SOURCE) */

#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

#include "str.h"
#include "gids.h"
#include "error.h"


/*
 * getgrouplist(3) is neither in POSIX.1-2008 nor 4.4BSD, and its
 * implementations differ. So re-inventing the wheel seemed the most
 * straightforward course of action.
 *
 * Note, the function signature and the semantics of gids_get_list
 * differ from those of getgrouplist(3), if subtly.
 */
enum error
gids_get_list(const char *const logname, const gid_t gid,
              /* RATS: ignore; gids is bounds-checked. */
              gid_t (*const gids)[NGROUPS_MAX], int *const n)
{
	struct group *grp;	/* A group. */

	errno = 0;
	(*gids)[0] = gid;
	*n = 1;

	setgrent();
	while ((grp = getgrent())) {
		for (int i = 0; i < *n; i++) {
			if (grp->gr_gid == (*gids)[i]) {
				goto next;
			}
		}

		for (int i = 0; grp->gr_mem[i]; i++) {
			if (strcmp(grp->gr_mem[i], logname) == 0) {
				if (*n < NGROUPS_MAX) {
					(*gids)[*n] = grp->gr_gid;
				}
				(*n)++;
				break;
			}			
		}

		next:;
	}
	endgrent();

	if (errno != 0) {
		return ERR_SYS;
	}
	if (*n > NGROUPS_MAX) {
		return ERR_GIDS_MAX;
	}

	return OK;
}

