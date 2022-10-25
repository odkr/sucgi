/*
 * Test file_is_exec.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "../file.h"

int
main (int argc, char **argv)
{
	struct stat fstatus;
	
	if (argc != 2) {
		(void) fputs("usage: file_is_exec FNAME\n", stderr);
		return EXIT_FAILURE;
	}

	errno = 0;
	/* RATS: ignore */
	if (stat(argv[1], &fstatus) != 0) {
		err(EXIT_FAILURE, "stat %s", argv[1]);
	}

	if (file_is_exec(fstatus)) {
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}
