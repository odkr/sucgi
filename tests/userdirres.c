/*
 * Test userdir_resolve.
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
#include <pwd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../max.h"
#include "../userdir.h"


int
main (int argc, char **argv)
{
	char *user_dir;			/* User directory. */
	struct passwd *user;		/* User. */
	enum retval rc;			/* Return code. */

	if (argc != 3) {
		(void) fputs("usage: userdir FMT LOGNAME\n", stderr);
		return EXIT_FAILURE;
	}

	errno = 0;
	user = getpwnam(argv[2]);
	if (!user) {
		if (errno == 0)
			errx(EXIT_FAILURE, "no such user");
		else
			err(EXIT_FAILURE, "getpwnam %s", argv[2]);
	}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
	rc = userdir_resolve(argv[1], user, &user_dir);
#pragma GCC diagnostic pop

	switch (rc) {
	case OK:
		break;
	case ERR_RES:
		err(EXIT_FAILURE, "realpath %s", user_dir);
	case ERR_PRN:
		err(EXIT_FAILURE, "vsnprintf");
	case ERR_LEN:
		errx(EXIT_FAILURE, "expanded user directory is too long");
	default:
		errx(EXIT_FAILURE, "returned %u", rc);
	}

	(void) puts(user_dir);

	return EXIT_SUCCESS;
}
