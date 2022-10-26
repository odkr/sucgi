/*
 * Test file_is_wexcl.
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
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "../file.h"

/* Largest user ID. */
#if defined(UID_MAX)
#define UID_MAX_ UID_MAX
#else /* defined(UID_MAX) */
#define UID_MAX_ UINT_MAX
#endif /* defined(UID_MAX) */

int
main (int argc, char **argv)
{
	struct stat fstatus;
	const char *fname;
	long uid;

	if (argc != 3) {
		(void) fputs("usage: file_is_wexcl UID FNAME\n", stderr);
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

	fname = argv[2];
	errno = 0;
	/* RATS: ignore */
	if (stat(fname, &fstatus) != 0) {
		errx(EXIT_FAILURE, "stat %s", fname);
	}

	if (file_is_wexcl((uid_t) uid, fstatus)) {
		return EXIT_SUCCESS;
	}

	return EXIT_FAILURE;
}
