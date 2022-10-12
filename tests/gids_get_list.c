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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "../error.h"
#include "../gids.h"


int
main (int argc, char **argv)
{
	const char *logname;		/* A login name. */
	long gid;			/* A group ID. */
	gid_t gids[NGROUPS_MAX];	/* The list of group IDs. */
	int ngids;			/* The number of group IDs. */
	enum error rc;			/* gids_get_list's return code. */

	errno = 0;

	if (argc != 3) {
		(void) fputs("usage: gids_get_list LOGNAME GID\n", stderr);
		return EXIT_FAILURE;
	}

	logname = argv[1];
	gid = strtol(argv[2], NULL, 10);
	if (errno != 0) {
		err(EXIT_FAILURE, "strtol %s", argv[1]);
	}
	if (gid < 0) {
		errx(EXIT_FAILURE, "group IDs must be non-negative");
	}
#if defined(GID_MAX)
	if ((unsigned long) gid > GID_MAX) {
#else
	if ((unsigned long) gid > UINT_MAX) {
#endif
		errx(EXIT_FAILURE, "group ID is too large");
	}

	rc = gids_get_list(logname, (gid_t) gid, &gids, &ngids);
	switch (rc) {
		case OK:
			break;
		case ERR_SYS:
			err(EXIT_FAILURE, "getgrent");
		case ERR_GIDS_MAX:
			errx(EXIT_FAILURE, "%s: in too many groups.", logname);
		default:
			errx(EXIT_FAILURE, "unexpected return code %u", rc);
	}

	for (int i = 0; i < ngids; i++) {
		/* RATS: ignore */
		(void) printf("%llu\n", (unsigned long long) gids[i]);
	}

	return EXIT_SUCCESS;
}
