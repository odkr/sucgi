/*
 * Test try.
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
#include <stdlib.h>

#include "../error.h"


/* try wrapper that should always fail. */
static enum error
try_ok (void)
{
	try(OK);
	
	return ERR_SYS;
}

/* try wrapper that should always fail. */
static enum error
try_err (void)
{
	try(ERR_SYS);

	/* This point should not be reached. */
	return OK;
}

int
main (void) {
	warnx("checking (OK) ...");
	if (try_ok() != ERR_SYS) {
		errx(EXIT_FAILURE, "failed");
	}

	warnx("checking (ERR_SYS) ...");
	if (try_err() != ERR_SYS) {
		errx(EXIT_FAILURE, "failed");
	}

	warnx("success");
	return EXIT_SUCCESS;
}
