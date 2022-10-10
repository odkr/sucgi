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

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "lib.h"


int
main (int argc, char **argv)
{
	char **cmd;	/* The command. */
	long uid;	/* The user ID. */
	long gid;	/* The group ID. */
	int optc;	/* An option. */

	errno = 0;
        prog_name = "runas";

	/* RATS: ignore */
	while ((optc = getopt(argc, argv, "h")) != -1) {
		switch (optc) {
			case 'h':
				(void) puts(
"runas - run a command under the given user and group IDs\n\n"
"Usage:   runas UID GID CMD [ARG ...]\n"
"         runas -h\n\n"
"Operands:\n"
"    UID  A user ID.\n"
"    GID  A group ID.\n"
"    CMD  The command.\n"
"    ARG  An argument to CMD.\n\n"
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

	if (argc < 3) {
		(void) fputs("usage: runas UID GID COMM [ARG ...]\n", stderr);
		return EXIT_FAILURE;
	}

	uid = strtol(argv[0], NULL, 10);
	if (errno != 0) {
		die("strtol %s", argv[1]);
	}
	if (uid < 0) {
		die("user IDs must be equal to or greater than 0.");
	}

	gid = strtol(argv[1], NULL, 10);
	if (errno != 0) {
		die("strtol %s", argv[2]);
	}
	if (gid < 0) {
		die("group IDs must be equal to or greater than 0.");
	}

	if (setgroups(1, (gid_t[1]) {(gid_t) gid}) != 0) {
		die("setgroups %ld", gid);
	}
	if (setgid((gid_t) gid) != 0) {
		die("setgid %ld", gid);
	}
	if (setuid((uid_t) uid) != 0) {
		die("setuid %ld", uid);
	}

	cmd = &argv[2];
	/* RATS: ignore */
	(void) execvp(*cmd, cmd);

	/* This point should not be reached. */
	die("exec %s", *cmd);
}
