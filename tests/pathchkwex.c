/*
 * Test path_check_wexcl.
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

#include "../error.h"
#include "../path.h"
#include "../sysdefs.h"


int
main (int argc, char **argv)
{
	/* RATS: ignore */
	char cur[PATH_SIZE];		/* Current directory. */
	struct passwd *pwd;		/* User. */
	enum retcode rc;		/* Return code. */

	if (argc != 4) {
		(void) fputs("usage: pathchkwex LOGNAME FILE PARENT\n",
		             stderr);
		return EXIT_FAILURE;
	}

	errno = 0;
	pwd = getpwnam(argv[1]);
	if (!pwd) {
		if (errno == 0)
			errx(EXIT_FAILURE, "no such user");
		else
			err(EXIT_FAILURE, "getpwam");
	}

	rc = path_check_wexcl(pwd->pw_uid, argv[2], argv[3], cur);
	switch (rc) {
	case OK:
		break;
	case ERR_OPEN:
		err(EXIT_FAILURE, "open %s", cur);
	case ERR_CLOSE:
		err(EXIT_FAILURE, "close %s", cur);
	case ERR_STAT:
		err(EXIT_FAILURE, "stat %s", cur);
	case FAIL:
	        errx(EXIT_FAILURE, "%s is writable by users other than %s",
		     cur, pwd->pw_name);
	default:
		errx(EXIT_FAILURE, "returned %u", rc);
	}

	(void) puts(cur);

	return EXIT_SUCCESS;
}
