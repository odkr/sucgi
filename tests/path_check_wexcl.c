/*
 * Test path_check_wexcl.
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
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "../error.h"
#include "../path.h"

#if !defined(UID_MAX)
#define UID_MAX_ UINT_MAX
#else /* !defined(UID_MAX) */
#define UID_MAX_ UID_MAX
#endif /* !defined(UID_MAX) */

int
main (int argc, char **argv)
{
	/* RATS: ignore */
	char cur[MAX_STR];	/* Current directory. */
	const char *parent;	/* Parent directory. */
	const char *fname;	/* Filename. */
	long uid;		/* User ID. */
	enum error rc;		/* Return code. */

	if (argc != 4) {
		(void) fputs("usage: path_check_wexcl UID DIR FILE\n", stderr);
		return EXIT_FAILURE;
	}

	errno = 0;
	uid = strtol(argv[1], NULL, 10);

	if (errno != 0) {
		err(EXIT_FAILURE, "strtol %s", argv[1]);
	}

	if (uid < 0) {
		errx(EXIT_FAILURE, "user IDs must be non-negative");
	}

	if ((unsigned long) uid > UID_MAX_) {
		errx(EXIT_FAILURE, "user ID is too large");
	}

	parent = argv[2];
	fname = argv[3];

	rc = path_check_wexcl((uid_t) uid, parent, fname, &cur);
	switch (rc) {
		case OK:
			break;
		case ERR_OPEN:
			error("open %s: %m.", cur);
		case ERR_CLOSE:
			error("close %s: %m.", cur);
		case ERR_STAT:
			error("stat %s: %m.", cur);
		case FAIL:
		        error("%s is writable by user IDs other than %lu.",
			      cur, uid);
		default:
			error("returned %u.", rc);
	}

	(void) puts(cur);

	return EXIT_SUCCESS;
}
