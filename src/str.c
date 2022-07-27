/*
 * String functions for suCGI.
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
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "err.h"
#include "str.h"


error
str_cp(const char *const src, char **dest)
{
	size_t len = 0;

	assert(src && dest);
	reraise(str_len(src, &len));
	*dest = malloc(len + 1);
	if (!*dest) return ERR_SYS;
	// - dest is guaranteed to be large enough to hold SRC.
	// - dest is null-terminated immediately after the call to memcpy. 
	//
	// flawfinder: ignore
	*dest = memcpy(*dest, src, len);
	(*dest)[len] = '\0';

	return OK;
}

bool
str_eq(const char *const s1, const char *const s2)
{
	assert(s1);
	assert(s2);

	return (strcmp(s1, s2) == 0);
}

error
str_len(const char *const s, size_t *len)
{
	size_t n = strnlen(s, STR_MAX_LEN + 2);
	if (n > STR_MAX_LEN) return ERR_STR_LEN;

	*len = n;
	return OK;
}

error
str_split(const char *const s, const char *sep, const int max,
          char ***subs, int *n)
{
	char *str = NULL;	/* Copy of s. */
	char *token = NULL;	/* Start of current substring. */
	char **tokens = NULL;	/* Substrings. */
	int ntokens = 0;	/* Number of substrings. */

	tokens = calloc((size_t) max + 2, sizeof(char *));
	if (!tokens) return ERR_SYS;
	assert(s && sep && subs);
	reraise(str_cp(s, &str));

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
	char **arg = NULL;	/* Alias for the current variadic argument. */
	char **tokens = NULL;	/* Substrings. */
	int ntokens = 0;	/* Number of substrings. */

	assert(s && sep);
	reraise(str_split(s, sep, n - 1, &tokens, &ntokens));

	va_start(ap, n);

	for (int i = 0; i < ntokens; i++) {
		arg = va_arg(ap, char**);
		assert(arg);
		reraise(str_cp(tokens[i], arg));
	}

	for (int i = ntokens; i < n; i++) {
		arg = va_arg(ap, char**);
		assert(arg);
		*arg = NULL;
	}

	va_end(ap);

	free(tokens);
	return OK;
}
