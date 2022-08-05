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
	/* cppcheck-suppress cert-STR05-C; not a constant. */
	/* Flawfinder: ignore */
	char large[STR_MAX] = "";	/* A string just within limits. */
	/* cppcheck-suppress cert-STR05-C; not a constant. */
	/* Flawfinder: ignore */
	char huge[STR_MAX + 1] = "";	/* A string that exceeds STR_MAX. */
	/* cppcheck-suppress cert-STR05-C; not a constant. */
	/* Flawfinder: ignore */
	char head[STR_MAX] = "";	/* First token. */
	char *tail = NULL;		/* Remainder. */
	
	memset(huge, 'x', STR_MAX);
	memset(large, 'x', STR_MAX - 1);

	/* Test overly long string. */
	assert(strnlen(huge, STR_MAX + 1) == STR_MAX);
	assert(str_split(huge, ",", &head, &tail) == ERR_STR_MAX);
	assert(!tail);

	/* Test long string. */
	assert(strnlen(large, STR_MAX) == STR_MAX - 1);
	assert(str_split(large, ",", &head, &tail) == OK);
	assert(str_eq(head, large));
	assert(tail == NULL);

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
