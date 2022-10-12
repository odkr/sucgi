/*
 * Test file_safe_stat.
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
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "../error.h"
#include "../file.h"


int
main (int argc, char **argv)
{
	struct stat fstatus;

	errno = 0;

	if (argc != 2) {
		(void) fputs("usage: file_safe_stat FNAME\n", stderr);
		return EXIT_FAILURE;
	}

	if (file_safe_stat(argv[1], &fstatus) != OK) {
		err(EXIT_FAILURE, "open %s", argv[1]);
	}

	/* RATS: ignore. */
	(void) printf("uid=%llu gid=%llu mode=%o size=%llu\n",
	              (unsigned long long) fstatus.st_uid,
	              (unsigned long long) fstatus.st_gid,
	                                   fstatus.st_mode,
	              (unsigned long long) fstatus.st_size);

	return EXIT_SUCCESS;
}
