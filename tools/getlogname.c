/*
 * Print the login name associated with a user ID.
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

#include <errno.h>
#include <pwd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "lib.h"


int
main (int argc, char **argv)
{
	struct passwd *pwd;	/* The user's passwd record. */
	long uid;		/* The user's ID. */
	int optc;		/* An option character. */

	errno = 0;

	/* RATS: ignore */
	prog_name = "getlogname";

	/* RATS: ignore */
	while ((optc = getopt(argc, argv, "h")) != -1) {
		switch (optc) {
			case 'h':
				(void) puts(
"getlogname - print the login name associated with a user ID.\n\n"
"Usage:   getlogname UID\n"
"         getlogname -h\n\n"
"Operands:\n"
"    UID  A user ID.\n\n"
"Options:\n"
"    -h   Print this help screen.\n\n"
"Copyright 2022 Odin Kroeger.\n"
"Released under the GNU General Public License.\n"
"This programme comes with ABSOLUTELY NO WARRANTY."
				);
				return EXIT_SUCCESS;
			default:
				return EXIT_FAILURE;
		}
	}

	if ((argc - optind) != 1) {
		(void) fputs("usage: getlogname UID\n", stderr);
		return EXIT_FAILURE;
	}

	uid = strtol(argv[optind], NULL, 10);
	if (errno != 0) {
		die("strtol %s", argv[optind]);
	}
	if (uid < 0) {
		die("user IDs must be equal to or greater than 0.");
	}

	pwd = getpwuid((uid_t) uid);
	if (!pwd) {
		if (errno == 0) {
			die("getpwuid %ld: no such user.", uid);
		} else {
			die("getpwuid %ld", uid);
		}
	}

	/* RATS: ignore */
	(void) puts(pwd->pw_name);

	return EXIT_SUCCESS;
}
