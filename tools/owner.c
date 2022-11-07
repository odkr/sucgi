/*
 * Print the login name of a file's owner.
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

#include <sys/stat.h>
#include <err.h>
#include <errno.h>
#include <pwd.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


int
main (int argc, char **argv)
{
	struct passwd *pwd;	/* The user. */
	struct stat fstatus;	/* The file status. */
	int ch;			/* An option character. */

	/* RATS: ignore */
	while ((ch = getopt(argc, argv, "h")) != -1) {
		switch (ch) {
			case 'h':
				(void) puts(
"owner - print the login name of a file's owner\n\n"
"Usage:    owner FILE\n"
"          owner -h\n\n"
"Operands:\n"
"    FILE  A filename.\n\n"
"Options:\n"
"    -h    Print this help screen.\n\n"
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

	if (argc != 1) {
		(void) fputs("usage: owner FILE\n", stderr);
		return EXIT_FAILURE;
	}

	errno = 0;
	if (stat(argv[0], &fstatus) != 0)
		err(EXIT_FAILURE, "stat %s", argv[1]);

	errno = 0;
	pwd = getpwuid(fstatus.st_uid);
	if (!pwd) {
		if (errno == 0)
			errx(EXIT_FAILURE, "owned by unallocated UID %llu",
			     (long long unsigned) fstatus.st_uid);
		else
			err(EXIT_FAILURE, "getpwuid %llu",
			    (long long unsigned) fstatus.st_uid);
	}

	(void) puts(pwd->pw_name);
}
