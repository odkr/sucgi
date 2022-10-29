/*
 * Find an (un-)allocated user or group ID in a given range.
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
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* Largest permissible user ID. */
#if !defined(UID_MAX)
#define UID_MAX UINT_MAX
#endif /* !defined(UID_MAX) */

/* Largest permissible group ID. */
#if !defined(GID_MAX)
#define GID_MAX UINT_MAX
#endif /* !defined(GID_MAX) */


int
main (int argc, char **argv)
{
	bool find_unalloc;	/* Find an unallocated, not an allocated ID? */
	bool find_group;	/* Find a group, not a user? */
	bool print_name;	/* Print a name, not an ID (not for -N)? */
	long range[2];		/* ID range. */
	int64_t max;		/* Upper limit for IDs. */
	long *ids;		/* List of IDs (only for -N). */
	size_t n;		/* The number of IDs (only for -N). */
	int ch;			/* An option character. */

	find_unalloc = false;
	find_group = false;
	print_name = false;
	ids = NULL;
	n = 0;

	/* RATS: ignore */
	while ((ch = getopt(argc, argv, "Nugnh")) != -1) {
		switch (ch) {
			case 'N':
				find_unalloc = true;
				break;
			case 'u':
				find_group = false;
				break;
			case 'g':
				find_group = true;
				break;
			case 'n':
				print_name = true;
				break;
			case 'h':
				(void) puts(
"findid - find an (un-)allocated user or group ID in a given range.\n\n"
"Usage:     findid [-N] [-u|-g] [-n]  STA END\n"
"           findid -h\n\n"
"Operands:\n"
"    START  Start of range.\n"
"    END    End of range.\n\n"
"Options:\n"
"    -u     Search for a user ID (default).\n"
"    -g     Search for a group ID.\n"
"    -n     Print name instead of ID.\n"
"    -N     Search for a non-existent ID.\n"
"    -h     Print this help screen.\n\n"
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

	if (argc != 2) {
		(void) fputs("usage: findid [-N] [-u|-g] [-n] START END\n",
		             stderr);
		return EXIT_FAILURE;
	}

	max = (find_group) ? GID_MAX : UID_MAX;

	for (int i = 0; i < 2; i++) {
		errno = 0;
		range[i] = strtol(argv[i], NULL, 10);

		if (errno != 0) {
			err(EXIT_FAILURE, "strtol %s", argv[i]);
		}
		if (range[i] < 0) {
			errx(EXIT_FAILURE, "%s: is negative", argv[i]);
		}
		if ((int64_t) range[i] > max) {
			errx(EXIT_FAILURE, "%s: is too large", argv[i]);
		}
	}

	if (range[0] > range[1]) {
		errx(EXIT_FAILURE, "start greater than end");
	}

	if (find_unalloc) {
		ids = calloc((size_t) (range[1] - range[0]), sizeof(*ids));
		if (!ids) {
			err(EXIT_FAILURE, "calloc");
		}
	}

	if (find_group) {
		setgrent();		
	} else {
		setpwent();
	}

	errno = 0;
	do {
		long id;	/* The ID. */
		char *name;	/* The name. */
		
		if (find_group) {
			struct group *grp;
			
			grp = getgrent();
			if (!grp) {
				break;
			}

			id = (long) grp->gr_gid;
			name = grp->gr_name;
		} else {
			struct passwd *pwd;
			
			pwd = getpwent();
			if (!pwd) {
				break;
			}

			id = (long) pwd->pw_uid;
			name = pwd->pw_name;
		}
		
		if (!find_unalloc) {
			if (range[0] < id && id < range[1]) {
				if (print_name) {
					/* RATS: ignore */
					printf("%s\n", name);
				} else {
					/* RATS: ignore */
					printf("%ld\n", id);					
				}
				return EXIT_SUCCESS;
			}
		} else {
			ids[n] = id;
			n++;
		}
	} while (true);

	if (find_group) {
		endgrent();
	} else {
		endpwent();
	}

	if (find_unalloc) {
		for (long i = range[0]; i < range[1]; i++) {
			for (size_t j = 0; j < n; j++) {
				if (ids[j] == i) {
					goto next;
				}
			}

			/* RATS: ignore */
			printf("%ld\n", i);
			return EXIT_SUCCESS;
			
			next:;
		}
	}

	errx(EXIT_FAILURE, "no ID in given range");
}
