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

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>


/* The environment. See environ(7) */
extern char **environ;


int
main (int argc, char **argv)
{
	char **vars;		/* The new environment variables. */
	long n;			/* Number of those variables. */
	long ncur;		/* Number of current variables. */
	bool copy;		/* Copy current environment? */
	int ch;			/* An option character. */

	n = -1;
	ncur = 0;
	copy = true;

	/* RATS: ignore */
	while ((ch = getopt(argc, argv, "in:h")) != -1)
		switch (ch) {
		case 'i':
			copy = false;
			break;
		case 'n':
			n = strtol(optarg, NULL, 10);
			if (errno != 0)
				err(EXIT_FAILURE, "-n");
			if (n < 0)
				errx(EXIT_FAILURE, "-n: is negative");
			if (n > INT_MAX)
				errx(EXIT_FAILURE, "-n: is too large");
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
	argv += optind;
	argc -= optind;

	if ((int) n > argc)
		errx(EXIT_FAILURE, "-n: not that many arguments");

	if (n < 0)
		for (n = 0; n < argc && strstr(argv[n], "="); n++)
			; /* Empty on purpose. */
	if (copy)
		for (; environ[ncur]; ncur++)
			; /* Empty on purpose. */

	vars = calloc((size_t) (ncur + n + 1), sizeof(char *));
	if (!vars)
		err(EXIT_FAILURE, "calloc");

	for (long i = 0; i < ncur; i++)
		vars[i] = environ[i];
	for (long i = 0; i < n; i++)
		vars[ncur + i] = argv[i];

	argv += n;
	if (*argv) {
		(void) execve(*argv, argv, vars);
		err(EXIT_FAILURE, "exec %s", *argv);
	}

	for (; *vars; vars++)
		(void) puts(*vars);

	return EXIT_SUCCESS;
}
