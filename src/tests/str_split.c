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
	/* flawfinder: ignore */
	char huge[STR_MAX_LEN + 2] = "";	/* A huge string. */
	str4096 head = "";			/* First token. */
	char *tail = NULL;			/* Remainder. */

	/* Test overly long string. */
	memset(huge, 'x', STR_MAX_LEN + 1);
	assert(strnlen(huge, STR_MAX_LEN + 2) == STR_MAX_LEN + 1);
	assert(str_split(huge, ",", &head, &tail) == ERR_STR_LEN);
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
