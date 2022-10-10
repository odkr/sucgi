/*
 * Find an unallocated user or group ID in a given range.
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
#include <grp.h>
#include <pwd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "lib.h"


int
main (int argc, char **argv)
{
	char type;		/* The ID type. */
	long id;		/* The ID. */
	long rng[2];		/* An ID range. */
	int optc;		/* An option character. */

	errno = 0;
	prog_name = "unallocid";
	type = 'u';

	/* RATS: ignore */
	while ((optc = getopt(argc, argv, "guh")) != -1) {
		switch (optc) {
			case 'u':
				type = 'u';
				break;
			case 'g':
				type = 'g';
				break;
			case 'h':
				(void) puts(
"unallocid - find an unallocated user or group ID in a given range.\n\n"
"Usage:     unallocid [-u] [-g] BEGIN END\n"
"           unallocid -h\n\n"
"Operands:\n"
"    BEGIN  the beginning of the range.\n"
"    END    the end of the range.\n\n"
"Options:\n"
"    -u     Search for an unallocated user ID (default).\n"
"    -g     Search for an unallocated group ID.\n"
"    -h     Print this help screen.\n\n"
"Copyright 2022 Odin Kroeger.\n"
"Released under the GNU General Public License.\n"
"This programme comes with ABSOLUTELY NO WARRANTY."
				);
				return EXIT_SUCCESS;
			default:
				return EXIT_FAILURE;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc != 2) {
		(void) fputs("usage: unallocid [-u] [-g] BEGIN END\n", stderr);
		return EXIT_FAILURE;
	}

	for (int i = 0; i < 2; i++) {
		rng[i] = strtol(argv[i], NULL, 10);
		if (errno != 0) {
			die("strtol %s", argv[i]);
		}
		if (rng[i] < 0) {
			die("IDs must be equal to or greater than 0.");
		}
	}

	for (id = rng[0]; id <= rng[1]; id++) {
		if (type == 'u') {
			struct passwd *pwd;	/* A passwd entry. */

			pwd = getpwuid((uid_t) id);
			if (!pwd) {
				if (errno != 0) {
					die("getpwuid %ld", id);
				}
				break;
			}
		} else {
			struct group *grp;	/* A group entry. */

			grp = getgrgid((gid_t) id);
			if (!grp) {
				if (errno != 0) {
					die("getgrgid %ld", id);
				}
				break;
			}
		}
	}

	if (id > rng[1]) {
		die("no unallocated ID in range %ld-%ld.", rng[0], rng[1]);
	}

	/* RATS: ignore; the format string is a literal. */
	(void) printf("%ld\n", id);

	return EXIT_SUCCESS;
}
