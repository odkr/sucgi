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

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../env.h"
#include "../error.h"


int
main (int argc, char **argv)
{
	char *jail;		/* Jail directory. */
	char *var;		/* Variable name. */
	int flags;		/* Open flags. */

	
	if (argc != 4) {
		(void) fputs("usage: env_file_open JAIL VAR f|d\n", stderr);
		return EXIT_FAILURE;
	}

	jail = argv[1];
	var = argv[2];
	flags = O_RDONLY | O_CLOEXEC;

	if (strncmp(argv[3], "d", 2) == 0) {
		flags = flags | O_DIRECTORY;
	} else if (strncmp(argv[3], "f", 2) == 0) {
		/* Do nothing. */
	} else {
		errx(EXIT_FAILURE, "filetype must be 'f' or 'd'.");
	}


	const char *fname;	/* Filename. */
	int fd;			/* File descriptor. */
	enum error rc;		/* Return code. */

	rc = env_file_open(jail, var, flags, &fname, &fd);
	switch (rc) {
		case OK:
			break;
		case ERR_GETENV:
			error("getenv %s: %m.", var);
		case ERR_REALPATH:
			error("realpath %s: %m.", fname);
		case ERR_OPEN:
			error("open %s: %m.", fname);
		case ERR_LEN:
			error("path to file is too long.");
		case ERR_ILL:
			error("file %s not within jail.", fname);
		case ERR_NIL:
			error("$%s is unset or empty.", var);
		default:
			error("returned %u.", rc);
	}

	/* RATS: ignore */
	char buf[MAX_STR];	/* Buffer. */
	ssize_t n;		/* Bytes read. */

	/* RATS: ignore */
	while ((n = read(fd, &buf, MAX_STR)) > 0) {
		(void) write(1, buf, (size_t) n);
	}

	if (n < 0) {
		err(EXIT_FAILURE, "read");
	}

	return EXIT_SUCCESS;
}
