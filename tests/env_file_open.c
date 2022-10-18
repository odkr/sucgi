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
#include <errno.h>
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

	errno = 0;
	rc = env_file_open(jail, var, flags, &fname, &fd);
	switch (rc) {
		case OK:
			break;
		case ERR_SYS:
			err(EXIT_FAILURE, "$%s", var);
		case ERR_ENV_LEN:
			errx(EXIT_FAILURE, "$%s: path too long", var);
		case ERR_ENV_MAL:
			errx(EXIT_FAILURE, "$%s: not in %s.", var, jail);
		case ERR_ENV_NIL:
			errx(EXIT_FAILURE, "$%s: unset or empty.", var);
		default:
			errx(EXIT_FAILURE, "unexpected return code %u.", rc);
	}

	ssize_t n;		/* Bytes read. */
	char buf[STR_MAX];	/* Buffer. */

	while ((n = read(fd, &buf, STR_MAX)) > 0) {
		(void) write(1, buf, (size_t) n);
	}

	if (n < 0) {
		err(EXIT_FAILURE, "read");
	}

	return EXIT_SUCCESS;
}
