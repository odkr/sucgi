/* Utilities for the test suite.
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

#include "../str.h"
#include "utils.h"

void
die (const char *const message, ...)
{
	va_list ap;

	va_start(ap, message);
	// flawfinder: ignore
	vfprintf(stderr, message, ap);
	va_end(ap);
	fputs("\n", stderr);

	exit(EXIT_FAILURE);
}

error
str_to_ulong (const char *const s, unsigned long *n)
{
	long long m = 0;
	char *end = NULL;

	assert(s);

	m = strtoll(s, &end, 10);
	if (*end != '\0') return ERR;
	if (m == 0 && errno != 0) return ERR_SYS;

	*n = (unsigned long) m;
	return OK;
}

error
str_nsplit(const char *const s, const char *sep, const int max,
           char ***subs, int *n)
{
	char *str = NULL;	/* Copy of s. */
	char *token = NULL;	/* Start of current substring. */
	char **tokens = NULL;	/* Substrings. */
	int ntokens = 0;	/* Number of substrings. */

	assert(s && sep && subs);
	str = calloc(STR_MAX_LEN + 1, sizeof(char));
	if (!str) return ERR_SYS;
	reraise(str_cp(s, str));

	tokens = calloc((size_t) max + 2, sizeof(char *));
	if (!tokens) {
		free(str);
		return ERR_SYS;
	}

	ntokens = 0;
	tokens[0] = str;
	while ((ntokens < max) && (token = strpbrk(str, sep))) {
		*token = '\0';
		str = token + 1;
		tokens[++ntokens] = str;
	}
	tokens[++ntokens] = NULL;

	if (n) *n = ntokens;
	*subs = tokens;
	return OK;
}

error
str_vsplit(const char *const s, const char *sep, const int n, ...)
{
	va_list ap;		/* Current variadic argument. */
	char *arg = NULL;	/* Alias for the current variadic argument. */
	char **tokens = NULL;	/* Substrings. */
	int ntokens = 0;	/* Number of substrings. */

	assert(s && sep);
	reraise(str_nsplit(s, sep, n - 1, &tokens, &ntokens));

	va_start(ap, n);

	for (int i = 0; i < ntokens; i++) {
		arg = va_arg(ap, char*);
		assert(arg);
		reraise(str_cp(tokens[i], arg));
	}

	va_end(ap);

	free(tokens);
	return OK;
}

error
str_words (const char *const restrict s, char ***subs)
{
	int n = 0;

	assert(s);
	assert(subs);

	reraise(str_nsplit(s, " \f\n\r\t\v", STR_MAX_SUBS, subs, &n));
	if (n > STR_MAX_SUBS) return ERR;
	return OK;
}
