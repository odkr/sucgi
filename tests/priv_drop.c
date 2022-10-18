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

#include <err.h>
#include <errno.h>
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
	const char *login;	/* The username. */
	struct passwd *user;	/* The passwd entry of the given user. */
	enum error rc;		/* A return code. */

	
	if (argc != 2) {
		(void) fputs("usage: priv_drop LOGNAME\n", stderr);
		return EXIT_FAILURE;
	}
	
	login = argv[1];

	errno = 0;
	user = getpwnam(login);
	if (!user) {
		if (errno == 0) {
			errx(EXIT_FAILURE, "getpwnam %s: no such user", login);
		} else {
			err(EXIT_FAILURE, "getpwnam %s", login);
		}
	}

	errno = 0;
	rc = priv_drop(user->pw_uid, user->pw_gid, 1,
	               (gid_t[1]) {user->pw_gid});
	switch (rc) {
		case OK:
			break;
		case ERR_SYS:
			err(EXIT_FAILURE, "failed to set IDs or groups");
		case ERR_PRIV:
			errx(EXIT_FAILURE, "could resume privileges");
		default:
			errx(EXIT_FAILURE, "returned %u", rc);
	}

	/* RATS: ignore */
	(void) printf("euid=%llu egid=%llu ruid=%llu rgid=%llu\n",
	              (unsigned long long) geteuid(),
	              (unsigned long long) getegid(),
	              (unsigned long long) getuid(),
	              (unsigned long long) getgid());

	return EXIT_SUCCESS;
}
