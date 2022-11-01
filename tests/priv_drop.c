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
	const char *logname;	/* The username. */
	struct passwd *pwd;	/* The passwd entry of the given user. */
	enum error rc;		/* A return code. */

	
	if (argc != 2) {
		(void) fputs("usage: priv_drop LOGNAME\n", stderr);
		return EXIT_FAILURE;
	}
	
	logname = argv[1];

	errno = 0;
	pwd = getpwnam(logname);
	if (!pwd) {
		if (errno == 0) {
			errx(EXIT_FAILURE, "user %s is unknown", logname);
		} else {
			err(EXIT_FAILURE, "getpwnam %s", logname);
		}
	}

	rc = priv_drop(pwd->pw_uid, pwd->pw_gid, 1, (gid_t[1]) {pwd->pw_gid});
       	switch (rc) {
       		case OK:
       			break;
       		case ERR_SETGROUPS:
       			error("setgroups %llu ...: %m.",
       			      (long long unsigned) pwd->pw_gid);
       		case ERR_SETGID:
       			error("setgid %llu: %m.",
       			      (long long unsigned) pwd->pw_gid);
       		case ERR_SETUID:
       			error("setuid %llu: %m.",
       			      (long long unsigned) pwd->pw_uid);
       		case FAIL:
       			error("could resume superuser privileges.");
       		default:
       			error("returned %u.", rc);
       	}

	/* RATS: ignore */
	(void) printf("euid=%llu egid=%llu ruid=%llu rgid=%llu\n",
	              (long long unsigned) geteuid(),
	              (long long unsigned) getegid(),
	              (long long unsigned) getuid(),
	              (long long unsigned) getgid());

	return EXIT_SUCCESS;
}
