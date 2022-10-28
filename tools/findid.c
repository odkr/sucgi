/*
 * Find an (un-)allocated user or group ID in a given rng.
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
#if defined(UID_MAX)
#define UID_MAX_ UID_MAX
#else /* defined(UID_MAX) */
#define UID_MAX_ UINT_MAX
#endif /* defined(UID_MAX) */

/* Largest permissible group ID. */
#if defined(GID_MAX)
#define GID_MAX_ GID_MAX
#else /* defined(GID_MAX) */
#define GID_MAX_ UINT_MAX
#endif /* defined(GID_MAX) */

/* What type of ID to search for. */
typedef enum {
	USR,
	GRP
} ent_type;

/* Search for allocated or unallocated IDs? */
typedef enum {
	ALLOC,
	UALLC
} search_mode;

int
main (int argc, char **argv)
{
	struct passwd *pwd;	/* A passwd entry. */
	ent_type type;		/* The ID type. */
	search_mode mode;	/* The search mode. */
	long rng[2];		/* The ID rng. */
	long max;		/* The upper limit for IDs. */
	long *ids;		/* A list of IDs (only for -n). */
	size_t n;		/* The number of IDs (only for -n). */
	int ch;			/* An option character. */

	errno = 0;
	type = USR;
	mode = ALLOC;
	ids = NULL;
	n = 0;

	/* RATS: ignore */
	while ((ch = getopt(argc, argv, "ugnh")) != -1) {
		switch (ch) {
			case 'u':
				type = USR;
				break;
			case 'g':
				type = GRP;
				break;
			case 'n':
				mode = UALLC;
				break;
			case 'h':
				(void) puts(
"findid - find an (un-)allocated user or group ID in a given rng.\n\n"
"Usage:     findid [-u|-g] [-n] START END\n"
"           findid -h\n\n"
"Operands:\n"
"    START  Start of range.\n"
"    END    End of range.\n\n"
"Options:\n"
"    -u     Search for a user ID (default).\n"
"    -g     Search for a group ID.\n"
"    -n     Search for a non-existent ID.\n"
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
		(void) fputs("usage: findid [-u|-g] [-n] START END\n", stderr);
		return EXIT_FAILURE;
	}

	max = (long) (type == USR) ? UID_MAX_ : GID_MAX_;

	for (int i = 0; i < 2; i++) {
		rng[i] = strtol(argv[i], NULL, 10);

		if (errno != 0) {
			err(EXIT_FAILURE, "strtol %s", argv[i]);
		}

		if (rng[i] < 0) {
			errx(EXIT_FAILURE, "%s: is negative", argv[i]);
		}

		if (rng[i] > max) {
			errx(EXIT_FAILURE, "%s: is too large", argv[i]);
		}
	}

	if (rng[0] > rng[1]) {
		errx(EXIT_FAILURE, "start greater than end");
	}

	if (mode == UALLC) {
		ids = calloc((size_t) rng[1] - rng[0], sizeof(ids));
		if (!ids) {
			err(EXIT_FAILURE, "calloc");
		}
	}

	errno = 0;
	
	if (type == USR) {
		setpwent();
	} else {
		setgrent();
	}

	do {
		long id;
		
		if (type == USR) {
			struct passwd *pwd;
			
			pwd = getpwent();
			if (!pwd) {
				break;
			}

			id = (long) pwd->pw_uid;
		} else {
			struct group *grp;
			
			grp = getgrent();
			if (!grp) {
				break;
			}

			id = (long) grp->gr_gid;
		}

		if (mode == ALLOC) {
			if (rng[0] < id && id < rng[1]) {
				/* RATS: ignore */
				printf("%ld\n", id);
				return EXIT_SUCCESS;
			}
		} else {
			ids[n] = id;
			n++;
		}
	} while (true);

	if (type == USR) {
		endpwent();
	} else {
		endgrent();
	}

	if (mode == UALLC) {
		for (long i = rng[0]; i < rng[1]; i++) {
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

	errx(EXIT_FAILURE, "no ID in given range matches");
}
