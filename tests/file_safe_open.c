/*
 * Test file_safe_open.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../error.h"
#include "../file.h"


int
main (int argc, char **argv)
{
	char *fname;		/* File name. */
	int flags;		/* Open flags. */
	
	if (argc != 3) {
		(void) fputs("usage: file_safe_open FNAME f|d\n", stderr);
		return EXIT_FAILURE;
	}

	fname = argv[1];
	flags = O_RDONLY | O_CLOEXEC;

	if (strncmp(argv[2], "d", 2) == 0) {
		flags = flags | O_DIRECTORY;
	} else if (strncmp(argv[2], "f", 2) == 0) {
		/* Do nothing. */
	} else {
		errx(EXIT_FAILURE, "filetype must be 'f' or 'd'.");
	}


	int fd;			/* File descriptor. */
	enum error rc;		/* Return code. */

	errno = 0;
	rc = file_safe_open(fname, flags, &fd);
	switch (rc) {
		case OK:
			break;
		case ERR_SYS:
			err(EXIT_FAILURE, "open %s", fname);
		case ERR_CNV:
			errx(EXIT_FAILURE, "conversion error");
		default:
			errx(EXIT_FAILURE, "unexpected return code %u.", rc);
	}


	FILE *file;		/* Stream. */
	int ch;			/* Character. */

	errno = 0;
	file = fdopen(fd, "r");
	if (!file) {
		err(EXIT_FAILURE, "open %s", fname);
	}

	/* RATS: ignore */
	while ((ch = fgetc(file)) != EOF) {
		/* RATS: ignore */
		(void) fputc(ch, stdout);
	}

	if (ferror(file)) {
		errx(EXIT_FAILURE, "error while reading from %s", fname);
	}

	return EXIT_SUCCESS;
}
