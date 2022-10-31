/*
 * Print users or groups.
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
#include <grp.h>
#include <search.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


/*
 * The number of elements that the array of IDs that have already been seen
 * should be incremented by when it's full.
 */
#define INC 1024


/*
 * Exit statuses.
 */
enum status {
	OK = 0,			/* Success. */
	ERR_USAGE = 64,		/* Usage errors. */
	ERR_NOMATCH = 67,	/* No matches. */
	ERR_SIZE = 69,		/* Too many entries. */
	ERR_OS = 71		/* System error. */
};


/*
 * Compare ID A with ID B and return 0
 * if they are equal and non-zero otherwise.
 */
int
id_eq (const void *const a, const void *const b)
{	
	return (*(id_t *) a == *(id_t *) b) ? 0 : 1;
}

/*
 * Main
 */
int
main(int argc, char **argv)
{
	id_t *ids;		/* Seen IDs. */
	size_t nids;		/* Number of seen IDs. */
	long from;		/* Only print IDs at least this large. */
	long to;		/* Only print IDs at most this large. */
	long nmax;		/* Print at most than many IDs. */
	bool getgr;		/* Print group entries, not passwd ones? */
	int ch;			/* An option character. */

	getgr = false;
	from = -1;
	to = -1;
	nmax = -1;

	/* RATS: ignore */
	while ((ch = getopt(argc, argv, "ugf:c:n:h")) != -1) {
		switch (ch) {
			case 'u':
				getgr = false;
				break;
			case 'g':
				getgr = true;
				break;
			case 'f':
				errno = 0;
				from = strtoll(optarg, NULL, 0);
				if (errno != 0) {
					err(ERR_USAGE,
					    "-f: strtoll %s", optarg);
				}
				if (from < 0) {
					errx(ERR_USAGE,
					     "-f: %s is negative", optarg);
				}
				break;
			case 'c':
				errno = 0;
				to = strtoll(optarg, NULL, 0);
				if (errno != 0) {
					err(ERR_USAGE,
					    "-c: strtoll %s", optarg);
				}
				if (from < 0) {
					errx(ERR_USAGE,
					     "-c: %s is negative", optarg);
				}
				break;
			case 'n':
				errno = 0;
				nmax = strtoll(optarg, NULL, 0);
				if (errno != 0) {
					err(ERR_USAGE,
					    "-n: strtoll %s", optarg);
				}
				if (nmax < 1) {
					errx(ERR_USAGE,
					     "-n: %s is non-positive", optarg);
				}
				break;
			case 'h':
				(void) puts(
"ents - print users or groups\n\n"
"Usage:    ents [-u|-g] [-f N] [-c N]\n"
"          ents -h\n\n"
"Options:\n"
"    -u    Print users (default).\n"
"    -g    Print groups.\n"
"    -f N  Only print entries with an ID greater than or equal to N.\n"
"    -c N  Only print entries with an IDs less than or equal to N.\n"
"    -h    Print this help screen.\n\n"
"Exit statuses:\n"
"     0    Success.\n"
"    64    Usage error.\n"
"    67    No matches.\n"
"    69    Too many matches.\n"
"    71    System error.\n\n"
"Copyright 2022 Odin Kroeger.\n"
"Released under the GNU General Public License.\n"
"This programme comes with ABSOLUTELY NO WARRANTY."
				);
				return OK;
			default:
				return ERR_USAGE;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc > 0) {
		fputs("usage: ents [-u|-g] [-f N] [-c N]\n", stderr);
		return ERR_USAGE;
	}

	ids = malloc(sizeof(*ids) * INC);
	nids = 0;
	if (!ids) {
		err(ERR_OS, "malloc");
	}

	setpwent();
	setgrent();

	do {
		id_t id;		/* Entry ID. */
		char *name;		/* Entry name. */
		
		if (getgr) {
			struct group *grp;	/* A group entry. */
			
			grp = getgrent();
			if (!grp) {
				if (errno == 0) {
					break;
				} else {
					err(ERR_OS, "getgrent");
				}
			}
			
			id = grp->gr_gid;
			name = grp->gr_name;
		} else {
			struct passwd *pwd;	/* A passwd entry. */
			
			pwd = getpwent();
			if (!pwd) {
				if (errno == 0) {
					break;
				} else {
					err(ERR_OS, "getpwent");
				}
			}
			
			id = pwd->pw_uid;
			name = pwd->pw_name;
		}

		if (-1 < from && id < (long long unsigned) from) {
			continue;
		}

		if (-1 < to && id > (long long unsigned) to) {
			continue;
		}

		if (lfind(&id, ids, &nids, sizeof(*ids), id_eq)) {
			continue;
		}

		ids[nids++] = id;
		if (nids % INC == 0) {
			size_t cur;
			size_t new;
			
			cur = sizeof(*ids) * nids;
			new = cur + sizeof(*ids) * INC;
			if (new < cur) {
				errx(ERR_SIZE, "too many entries");
			}

			/* RATS: ignore; no use of uninitialised memory. */
			ids = realloc(ids, new);
			if (!ids) {
				err(ERR_OS, "realloc");
			}
		}

		/* RATS: ignore; format string is a literal. */
		printf("%llu:%s\n", (long long unsigned) id, name);
	} while (nmax < 0 || nids < (size_t) nmax);
	
	if (nids == 0) {
		errx(ERR_NOMATCH, "no matching ID");
	}
	
	return EXIT_SUCCESS;
}
