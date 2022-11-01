/*
 * Test gids_get_list.
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

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../gids.h"

/* Exit status for failures. */
#define T_FAIL 2

/* Calculate the number of elements in an array. */
#define NELEMS(x) (sizeof((x)) / sizeof(*(x)))

/* Largest user ID. */
#if defined(UID_MAX)
#define UID_MAX_ UID_MAX
#else /* defined(UID_MAX) */
#define UID_MAX_ UINT_MAX
#endif /* defined(UID_MAX) */

/* Largest group ID. */
#if defined(GID_MAX)
#define GID_MAX_ GID_MAX
#else /* defined(GID_MAX) */
#define GID_MAX_ UINT_MAX
#endif /* defined(GID_MAX) */

int
main (void)
{
	/* UIDs to test. */
	const uid_t uids[] = {0, getuid(), UID_MAX_};

	/* Primary GIDs to test. */
	const gid_t pgids[] = {0, getgid(), GID_MAX_};

	gid_t gids[MAX_GROUPS];		/* GIDs found. */
	int ngids;			/* Number of GIDs. */
	enum error rc;			/* Return code. */

	for (size_t i = 0; i < NELEMS(uids); i++) {
		for (size_t j = 0; j < NELEMS(pgids); j++) {
			const uid_t uid = uids[i];	/* UID. */
			const gid_t pgid = pgids[j];	/* GID. */
			struct passwd *pwd;		/* User. */

			errno = 0;
			pwd = getpwuid(uid);
			if (!pwd) {
				if (errno == 0) {
					continue;
				}

				err(EXIT_FAILURE, "getpwuid %llu",
				    (long long unsigned) uid);
			}

			warnx("checking (%s, %llu, ...) -> OK ...",
			      pwd->pw_name, (long long unsigned) pgid);

			rc = gids_get_list(pwd->pw_name, pgid, &gids, &ngids);

			if (rc != OK) {
				errx(T_FAIL, "returned %u", rc); 
			}
			if (ngids < 1) {
				errx(T_FAIL, "no GIDs saved");
			}
			if (gids[0] != pgid) {
				errx(T_FAIL, "first GID is not %llu",
				     (long long unsigned) pgid);
			}
		}
	}
	
	warnx("all tests passed");
	return EXIT_SUCCESS;
}
