/*
 * Run a programme with a given environment, any environment.
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
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "lib.h"


/* The environment. See environ(7) */
extern char **environ;


int
main (int argc, char **argv)
{
	char **new;		/* The new environment. */
	char **vars;		/* The new variables. */
	char **cmd;		/* The command. */
	long nenv;		/* Number of current variables. */
	long nvars;		/* Number of new variables. */
	int optc;		/* An option character. */
	bool ienv;		/* Inherit current environment? */


	errno = 0;
	prog_name = "badenv";
	nvars = -1;
	nenv = 0;
	ienv = true;

	/* RATS: ignore */
	while ((optc = getopt(argc, argv, "in:h")) != -1) {
		switch (optc) {
			case 'i':
				ienv = false;
				break;
			case 'n':
				nvars = strtol(optarg, NULL, 10);
				if (errno != 0) {
					die("strtol %s", optarg);
				}
				if (nvars < 0) {
					die("-n: must be a positive number.");
				}
				break;
			case 'h':
				(void) puts(
"badenv - run a programme with a given environment, any environment\n\n"
"Usage:    badenv [-i] [-n N] [VAR ...] [CMD [ARG ...]]\n"
"          badenv -h\n\n"
"Operands:\n"
"    VAR   An environment variable. Use -n if VAR contains no '='.\n"
"    CMD   A command to run. Not searched for in $PATH.\n"
"    ARG   An argument to CMD.\n\n"
"Options:\n"
"    -i    Do not inherit the current environment.\n"
"    -n N  Treat the first N arguments as environment variables.\n"
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

	if ((int) nvars > (argc - optind)) {
		die("-n: not that many arguments.");
	}

	vars = &argv[optind];
	if (nvars < 0) {
		for (nvars = 0; vars[nvars] && strstr(vars[nvars], "="); nvars++)
			/* Body of loop empty on purpose. */;
	}
	if (ienv) {
		for (; environ[nenv]; nenv++)
			/* Body of lopp empty on purpose. */;
	}

	new = calloc((size_t) (nenv + nvars + 1), sizeof(char *));
	if (!new) {
		die("calloc");
	}

	for (long i = 0; i < nenv; i++) {
		new[i] = environ[i];
	}
	for (long i = 0; i < nvars; i++) {
		new[nenv + i] = vars[i];
	}

	cmd = &argv[(size_t) (optind + nvars)];
	if (*cmd) {
		/* RATS: ignore */
		(void) execve(*cmd, cmd, new);
		die("exec %s", *cmd);
	}

	for (; *new; new++) {
		(void) puts(*new);
	}

	return EXIT_SUCCESS;
}
