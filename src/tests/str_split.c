/*
 * Test str_split.
 */

#include <assert.h>
#include <string.h>

#include "../err.h"
#include "../str.h"


int
main(void)
{
	/* Flawfinder: ignore */
	char large[STR_MAX] = {0};	/* A string just within limits. */
	/* Flawfinder: ignore */
	char huge[STR_MAX + 1U] = {0};	/* A string that exceeds STR_MAX. */
	/* Flawfinder: ignore */
	char head[STR_MAX] = {0};	/* First token. */
	char *tail = NULL;		/* Remainder. */
	
	(void) memset(huge, 'x', STR_MAX);
	(void) memset(large, 'x', STR_MAX - 1U);

	/* Test overly long string. */
	assert(strnlen(huge, STR_MAX + 1U) == STR_MAX);
	assert(str_split(huge, ",", &head, &tail) == ERR_STR_MAX);
	assert(!tail);

	/* Test long string. */
	assert(strnlen(large, STR_MAX) == STR_MAX - 1U);
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
