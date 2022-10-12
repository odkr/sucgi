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

#include "../env.h"
#include "../error.h"


int
main (int argc, char **argv)
{
	char *jail;		/* Jail directory. */
	char *var;		/* Variable name. */
	const char *fname;	/* Filename. */
	struct stat fstatus;	/* Filesystem metadata. */
	int fd;			/* File descriptor. */
	enum error rc;		/* Return code. */

	errno = 0;

	if (argc != 3) {
		(void) fputs("usage: env_file_open JAIL VAR\n", stderr);
		return EXIT_FAILURE;
	}

	jail = argv[1];
	var = argv[2];

	rc = env_file_open(jail, var, O_RDONLY | O_CLOEXEC, &fname, &fd);
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

	if (fstat(fd, &fstatus) != 0) {
		err(EXIT_FAILURE, "stat %s", fname);
	}

	/* RATS: ignore. */
	(void) printf("uid=%llu gid=%llu mode=%o size=%llu\n",
	              (unsigned long long) fstatus.st_uid,
	              (unsigned long long) fstatus.st_gid,
	                                   fstatus.st_mode,
	              (unsigned long long) fstatus.st_size);

	return EXIT_SUCCESS;
}
