/*
 * Run a programme under the given UID and GID.
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
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int
main (int argc, char **argv)
{
	struct passwd *pwd;	/* The user. */
	char **cmd;		/* The command. */
	int ch;			/* An option character. */

	/* RATS: ignore */
	while ((ch = getopt(argc, argv, "h")) != -1) {
		switch (ch) {
			case 'h':
				(void) puts(
"runas - run a command as a user\n\n"
"Usage:   runas LOGNAME CMD [ARG ...]\n"
"         runas -h\n\n"
"Operands:\n"
"    LOGNAME  A login name.\n"
"    CMD      A command.\n"
"    ARG      An argument to CMD.\n\n"
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

	argc -= optind;
	argv += optind;

	if (argc < 2) {
		(void) fputs("usage: runas LOGNAME CMD [ARG ...]\n", stderr);
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

	if (setgroups(1, (gid_t[1]) {(gid_t) pwd->pw_gid}) != 0)
		err(EXIT_FAILURE, "setgroups %llu",
		    (long long unsigned) pwd->pw_gid);
	if (setgid(pwd->pw_gid) != 0)
		err(EXIT_FAILURE, "setgid %llu",
		    (long long unsigned) pwd->pw_gid);
	if (setuid(pwd->pw_uid) != 0)
		err(EXIT_FAILURE, "setuid %llu",
		    (long long unsigned) pwd->pw_uid);

	cmd = &argv[1];
	/* RATS: ignore */
	(void) execvp(*cmd, cmd);

	/* This point should not be reached. */
	err(EXIT_FAILURE, "exec %s", *cmd);
}
