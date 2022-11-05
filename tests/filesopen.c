/*
 * Test file_sopen.
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

#include <sys/stat.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../file.h"
#include "../str.h"
#include "testdefs.h"
#include "testdefs.h"


int
main (int argc, char **argv)
{
	/* RATS: ignore */
	char buf[BUFSIZ];	/* Buffer. */
	char *fname;		/* File name. */
	int flags;		/* Open flags. */
	int fd;			/* File descriptor. */
	ssize_t n;		/* Bytes read. */
	enum retval rc;		/* Return code. */

	if (argc != 3) {
		(void) fputs("usage: filesopen FNAME f|d\n", stderr);
		return EXIT_FAILURE;
	}

	fname = argv[1];
	flags = O_RDONLY;

	if      (strncmp(argv[2], "d", 2) == 0)
		flags = flags | O_DIRECTORY;
	else if (strncmp(argv[2], "f", 2) == 0)
		; /* Do nothing. */
	else
		errx(EXIT_FAILURE, "filetype must be 'f' or 'd'.");

	rc = file_sopen(fname, flags, &fd);
	switch (rc) {
	case OK:
		break;
	case ERR_OPEN:
		err(EXIT_FAILURE, "open %s", fname);
	case ERR_CNV:
		errx(EXIT_FAILURE, "conversion error");
	default:
		errx(EXIT_FAILURE, "returned %u.", rc);
	}

	/* RATS: ignore */
	while ((errno = 0, n = read(fd, &buf, BUFSIZ)) > 0)
		(void) write(1, buf, (size_t) n);

	if (n < 0)
		err(EXIT_FAILURE, "read");

	return EXIT_SUCCESS;
}
