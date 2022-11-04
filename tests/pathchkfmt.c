/*
 * Test path_check_format.
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
#include <stdlib.h>
#include <stdio.h>

#include "../error.h"
#include "../path.h"
#include "../sysdefs.h"


int
main (int argc, char **argv)
{
	/* RATS: ignore */
	char exp[PATH_SIZE];		/* Current directory. */
	enum retcode rc;		/* Return code. */

	if (argc != 5) {
		(void) fputs("usage: pathchkfmt FILE FMT VA1 VA2\n", stderr);
		return EXIT_FAILURE;
	}

#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wformat-extra-args"
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
	rc = path_check_format(argv[1], exp, argv[2], argv[3], argv[4]);
#pragma GCC diagnostic pop

	switch (rc) {
	case OK:
		break;
	case ERR_RES:
		err(EXIT_FAILURE, "realpath %s", exp);
	case ERR_LEN:
		errx(EXIT_FAILURE, "expanded user directory is too long");
	case FAIL:
		errx(EXIT_FAILURE, "file %s does not match format", argv[1]);
	default:
		errx(EXIT_FAILURE, "returned %u.", rc);
	}

	return EXIT_SUCCESS;
}
