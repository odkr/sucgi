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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#if !defined(UID_MAX)
#define UID_MAX_ UINT_MAX
#else /* !defined(UID_MAX) */
#define UID_MAX_ UID_MAX
#endif /* !defined(UID_MAX) */

#if !defined(GID_MAX)
#define GID_MAX_ UINT_MAX
#else /* !defined(GID_MAX) */
#define GID_MAX_ GID_MAX
#endif /* !defined(GID_MAX) */

typedef enum {
	USER,
	GROUP
} id_type;

typedef enum {
	ALLOC,
	UNALLOC
} search_mode;

int
main (int argc, char **argv)
{
	id_type type;		/* The ID type. */
	search_mode mode;	/* The search mode. */
	long id;		/* The ID. */
	long range[2];		/* An ID range. */
	long max;		/* The upper limit for IDs. */
	int ch;			/* An option character. */

	errno = 0;
	type = USER;
	mode = ALLOC;

	/* RATS: ignore */
	while ((ch = getopt(argc, argv, "ugnh")) != -1) {
		switch (ch) {
			case 'u':
				type = USER;
				break;
			case 'g':
				type = GROUP;
				break;
			case 'n':
				mode = UNALLOC;
				break;
			case 'h':
				(void) puts(
"findid - find an (un-)allocated user or group ID in a given range.\n\n"
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

	max = (long) (type == USER) ? UID_MAX_ : GID_MAX_;

	for (int i = 0; i < 2; i++) {
		range[i] = strtol(argv[i], NULL, 10);

		if (errno != 0) {
			err(EXIT_FAILURE, "strtol %s", argv[i]);
		}

		if (range[i] < 0) {
			errx(EXIT_FAILURE, "%s: is negative", argv[i]);
		}

		if ((long) range[i] > max) {
			errx(EXIT_FAILURE, "%s: too large", argv[i]);
		}
	}

	if (range[0] > range[1]) {
		errx(EXIT_FAILURE, "beginning of range past the end");
	}

	for (id = range[0]; id <= range[1]; id++) {
		struct passwd *pwd;	/* A passwd entry. */
		struct group *grp;	/* A group entry. */

		pwd = NULL;
		grp = NULL;

		if (type == USER) {
			pwd = getpwuid((uid_t) id);
			if (!pwd) {
				if (errno != 0) {
					err(EXIT_FAILURE, "getpwuid %ld", id);
				}
			}
		} else {
			grp = getgrgid((gid_t) id);
			if (!grp) {
				if (errno != 0) {
					err(EXIT_FAILURE, "getgrgid %ld", id);
				}
			}
		}
		

		if (pwd || grp) {
			if (mode == ALLOC) {
				break;
			}
		} else {
			if (mode == UNALLOC) {
				break;
			}
		}
	}

	if (id > range[1]) {
		errx(EXIT_FAILURE, "no matching ID found in range");
	}

	/* RATS: ignore; the format string is a literal. */
	(void) printf("%ld\n", id);

	return EXIT_SUCCESS;
}
