/*
 * Test str_split.
 */

#include <assert.h>
#include <string.h>

#include "../err.h"
#include "../str.h"


#include <stdio.h>
int
main(void)
{
	/* Flawfinder: ignore */
	char huge[STR_MAX + 1] = "";	/* A huge string. */
	/* Flawfinder: ignore */
	char head[STR_MAX] = "";	/* First token. */
	char *tail = NULL;		/* Remainder. */

	/* Test overly long string. */
	memset(huge, 'x', STR_MAX);
	assert(strnlen(huge, STR_MAX + 1) == STR_MAX);
	assert(str_split(huge, ",", &head, &tail) == ERR_STR_MAX);
	assert(!tail);

	/* Simple test. */
	assert(str_split("a,b", ",", &head, &tail) == OK);
	assert(str_eq(head, "a"));
	assert(str_eq(tail, "b"));

	/* Empty strings. */
	assert(str_split(",b", ",", &head, &tail) == OK);
	assert(str_eq(head, ""));
	assert(str_eq(tail, "b"));

	assert(str_split("a,", ",", &head, &tail) == OK);
	assert(str_eq(head, "a"));
	assert(str_eq(tail, ""));

	assert(str_split("a,b", "", &head, &tail) == OK);
	assert(str_eq(head, "a,b"));
	assert(!tail);
}
