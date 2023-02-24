/*
 * Test error.
 *
 * Copyright 2022 and 2023 Odin Kroeger.
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

#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>

#include "../error.h"

int
main (int argc, char **argv) {
	if (argc < 2 || argc > 3) {
		fputs("usage: error FORMAT [ARG]\n", stderr);
		return EXIT_FAILURE;
	}

	atexit(closelog);
	openlog("error", LOG_CONS | LOG_NDELAY | LOG_PERROR, LOG_AUTH);

	(void) error(argv[1], argv[2]);

	/* This point should not be reached. */
	return EXIT_SUCCESS;
}
