/*
 * Test priv_drop.
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

#include <sys/types.h>
#include <err.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../error.h"
#include "../priv.h"


int
main (int argc, char **argv)
{
	struct passwd *pwd;	/* The passwd entry of the given user. */
	enum retcode rc;	/* A return code. */

	if (argc != 2) {
		(void) fputs("usage: privdrop LOGNAME\n", stderr);
		return EXIT_FAILURE;
	}
	
	errno = 0;
	pwd = getpwnam(argv[1]);
	if (!pwd) {
		if (errno == 0)
			errx(EXIT_FAILURE, "no such user");
		else
			err(EXIT_FAILURE, "getpwnam %s", argv[1]);
	}

	rc = priv_drop(pwd->pw_uid, pwd->pw_gid, 1, (gid_t[1]) {pwd->pw_gid});
       	switch (rc) {
	case OK:
		break;
	case ERR_SETGRPS:
		err(EXIT_FAILURE,"setgroups %llu ...",
		    (long long unsigned) pwd->pw_gid);
	case ERR_SETGID:
		err(EXIT_FAILURE, "setgid %llu",
		    (long long unsigned) pwd->pw_gid);
	case ERR_SETUID:
		err(EXIT_FAILURE, "setuid %llu",
		    (long long unsigned) pwd->pw_uid);
	case FAIL:
		errx(EXIT_FAILURE, "could resume superuser privileges.");
	default:
		errx(EXIT_FAILURE, "returned %u.", rc);
       	}

	(void) printf("euid=%llu egid=%llu ruid=%llu rgid=%llu\n",
	              (long long unsigned) geteuid(),
	              (long long unsigned) getegid(),
	              (long long unsigned) getuid(),
	              (long long unsigned) getgid());

	return EXIT_SUCCESS;
}
