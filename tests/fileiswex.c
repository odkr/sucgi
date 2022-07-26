/*
 * Test file_is_wexcl.
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
#include <stdio.h>
#include <stdlib.h>

#include "../file.h"


int
main (int argc, char **argv)
{
	struct passwd *pwd;	/* The user. */
	struct stat fstatus;	/* The file's status. */

	if (argc != 3) {
		(void) fputs("usage: fileiswex LOGNAME FNAME\n", stderr);
		return EXIT_FAILURE;
	}

	errno = 0;
	pwd = getpwnam(argv[1]);
	if (!pwd) {
		if (errno == 0)
			errx(EXIT_FAILURE, "no such user");
		else
			err(EXIT_FAILURE, "getpwnam");
	}

	errno = 0;
	if (stat(argv[2], &fstatus) != 0)
		errx(EXIT_FAILURE, "stat %s", argv[2]);

	if (file_is_wexcl(pwd->pw_uid, fstatus))
		return EXIT_SUCCESS;

	return EXIT_FAILURE;
}
