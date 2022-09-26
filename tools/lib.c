/* Utility functions for tools and/or the test suite.
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

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../err.h"
#include "../str.h"

#include "lib.h"


void
die (const char *const message, ...)
{
	va_list ap;

	assert(message != NULL);
	assert(message[0] != '\0');

	va_start(ap, message);
	/* RATS: ignore; format strings are always literals. */
	(void) vfprintf(stderr, message, ap);
	va_end(ap);
	(void) fputs("\n", stderr);

	exit(EXIT_FAILURE);
}

enum error
str_to_id (const char *const s, id_t *id)
{
	unsigned long n = 0;
	char *end = NULL;

	errno = 0;
	n = strtoul(s, &end, 10);
	if (errno != 0) return SC_ERR_SYS;
	if (*end != '\0') return SC_ERR_CNV;

	*id = (id_t) n;
	return SC_OK;
}
