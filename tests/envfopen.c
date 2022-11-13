/*
 * Test env_file_open.
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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../env.h"


int
main (int argc, char **argv)
{
	/* RATS: ignore */
	char buf[BUFSIZ];	/* Buffer. */
	char *jail;		/* Jail directory. */
	char *var;		/* Variable name. */
	const char *fname;		/* Filename. */
	int flags;		/* Open flags. */
	int fd;			/* File descriptor. */
	ssize_t n;		/* Bytes read. */
	enum retval rc;		/* Return code. */

	if (argc != 4) {
		(void) fputs("usage: envfopen JAIL VAR f|d\n", stderr);
		return EXIT_FAILURE;
	}

	jail = argv[1];
	var = argv[2];
	flags = O_RDONLY;

	if      (strncmp(argv[3], "d", 2) == 0)
		flags = flags | O_DIRECTORY;
	else if (strncmp(argv[3], "f", 2) == 0)
		; /* Empty on purpose. */
	else
		errx(EXIT_FAILURE, "filetype must be 'f' or 'd'.");

	rc = env_file_open(jail, var, flags, &fname, &fd);
	switch (rc) {
	case OK:
		break;
	case ERR_ENV:
		err(EXIT_FAILURE, "getenv %s", var);
	case ERR_RES:
		err(EXIT_FAILURE, "realpath %s", fname);
	case ERR_OPEN:
		err(EXIT_FAILURE, "open %s", fname);
	case ERR_LEN:
		errx(EXIT_FAILURE, "path too long");
	case ERR_ILL:
		errx(EXIT_FAILURE, "file %s not within jail", fname);
	case ERR_NIL:
		errx(EXIT_FAILURE, "$%s unset or empty", var);
	default:
		errx(EXIT_FAILURE, "returned %u.", rc);
	}

	/* RATS: ignore */
	while ((n = read(fd, &buf, BUFSIZ)) > 0)
		if (write(1, buf, (size_t) n) == -1)
			err(EXIT_FAILURE, "write");

	if (n < 0)
		err(EXIT_FAILURE, "read");

	return EXIT_SUCCESS;
}
