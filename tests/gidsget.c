/*
 * Test gids_get.
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
#include <pwd.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "../gids.h"

#define GIDS_SIZE 4096


int
main (int argc, char **argv)
{
	struct passwd *pwd;		/* User. */
	gid_t gids[GIDS_SIZE];		/* Groups the user is a member of. */
	int ngids;			/* Number of those groups. */
	int ch;				/* Option character. */
	enum retcode rc;		/* Return code. */

	ngids = GIDS_SIZE;

	/* RATS: ignore */
	while ((ch = getopt(argc, argv, "n:")) != -1)
		switch (ch) {
		case 'n':
			errno = 0;
			long n = strtol(optarg, NULL, 0);
			if (errno > 0)
				err(EXIT_FAILURE, "-n");
			if (n < 0)
				errx(EXIT_FAILURE, "-n: is negative");
			if (n > INT_MAX)
				errx(EXIT_FAILURE, "-n: is too large");
			ngids = (int) n;
			break;
		default:
			return EXIT_FAILURE;
		}
	argc -= optind;
	argv += optind;

	if (argc != 1) {
		(void) fputs("usage: gidsget [-n NUM] LOGNAME\n", stderr);
		return EXIT_FAILURE;
	}

	errno = 0;
	pwd = getpwnam(argv[0]);
	if (!pwd) {
		if (errno == 0)
			errx(EXIT_FAILURE, "no such user");
		else
			err(EXIT_FAILURE, "getpwnam");
	}

	rc = gids_get(pwd->pw_name, pwd->pw_gid, gids, &ngids);
	switch (rc) {
	case OK:
		break;
	case ERR_GETGR:
		err(EXIT_FAILURE, "getgrent");
	case ERR_LEN:
		errx(EXIT_FAILURE, "user %s belongs to too many groups",
		     pwd->pw_name);
	default:
		errx(EXIT_FAILURE, "returned %u.", rc);
	}

	for (int i = 0; i < ngids; i++)
		(void) printf("%llu\n", (long long unsigned) gids[i]);
	
	return EXIT_SUCCESS;
}
