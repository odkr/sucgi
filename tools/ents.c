/*
 * Print passwd entries.
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
#include <pwd.h>
#include <grp.h>
#include <search.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#define ELEM_INC 512


void
arr_resize(id_t **arr, size_t nelems, size_t inc)
{
	id_t *ptr;
	uint64_t size;

	if (nelems % inc != 0) {
		return;
	}

	size = (nelems + inc) * sizeof(**arr);
	if (size > SIZE_MAX) {
		errx(EXIT_FAILURE, "too many groups");
	}
	
	ptr = realloc(*arr, (size_t) size);
	if (!ptr) {
		err(EXIT_FAILURE, "realloc");
	}
	
	*arr = ptr;
}

int
id_eq(const void *const a, const void *const b)
{
	if (*((id_t *) a) == *((id_t *) b)) {
		return 0;
	}
	
	return 1;
}


int
main(int argc, char **argv)
{
	id_t *ids;		/* Seen IDs. */
	size_t nids;		/* Number of seen IDs. */
	bool getgrps;		/* Print group entries, not passwd ones? */
	int ch;			/* An option character. */

	getgrps = false;

	/* RATS: ignore */
	while ((ch = getopt(argc, argv, "ugh")) != -1) {
		switch (ch) {
			case 'u':
				getgrps = false;
				break;
			case 'g':
				getgrps = true;
				break;
			case 'h':
				(void) puts(
"ents - print user or group ID-name pairs\n\n"
"Usage:   ents [-u|-g]\n"
"         ents -h\n\n"
"Options:\n"
"    -u   Print user name-ID pairs (default).\n"
"    -g   print group name-ID pairs.\n"
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

	if (argc > 0) {
		fputs("usage: ents [-u|-g]\n", stderr);
		return EXIT_FAILURE;
	}

	errno = 0;
	ids = malloc(ELEM_INC * sizeof(*ids));
	if (!ids) {
		err(EXIT_FAILURE, "malloc");
	}
	nids = 0;

	if (getgrps) {
		struct group *grp;	/* A passwd entry. */

		errno = 0;		
		setgrent();
		while ((grp = getgrent())) {
			if (!lfind(&grp->gr_gid, ids,
			          &nids, sizeof(*ids), id_eq))
			{
				printf("%llu:%s\n",
				       (long long unsigned) grp->gr_gid,
				       grp->gr_name);

       				ids[nids] = grp->gr_gid;
       				nids++;

				arr_resize(&ids, nids, ELEM_INC);
			}
		}
		endgrent();

		if (errno != 0) {
			err(EXIT_FAILURE, "getgrent");
		}
	} else {
		struct passwd *pwd;	/* A passwd entry. */

		errno = 0;		
		setpwent();
		while ((pwd = getpwent())) {
			if (!lfind(&pwd->pw_uid, ids,
			          &nids, sizeof(*ids), id_eq))
			{
				printf("%llu:%s\n",
				       (long long unsigned) pwd->pw_uid,
				       pwd->pw_name);

       				ids[nids] = pwd->pw_uid;
       				nids++;

				arr_resize(&ids, nids, ELEM_INC);
			}
		}
		endpwent();

		if (errno != 0) {
			err(EXIT_FAILURE, "getpwent");
		}
	}


	
	return EXIT_SUCCESS;
}
