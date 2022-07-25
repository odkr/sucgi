/* Utilities for checks. */

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../str.h"

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
str_words (const char *const restrict s, char ***subs)
{
	int n = 0;

	assert(s);
	assert(subs);

	reraise(str_split(s, " \f\n\r\t\v", STR_MAX_SUBS, subs, &n));
	if (n > STR_MAX_SUBS) return ERR;
	return OK;
}
