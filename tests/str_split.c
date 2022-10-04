/*
 * Test str_split.
 */

#include <stdlib.h>
#include <string.h>

#include "../err.h"
#include "../str.h"
#include "../tools/lib.h"


/* Test case. */
struct signature {
	const char *s;
	const char *sep;
	const char *head;
	const char *tail;
	enum error ret;
};

/* A string just within limits. */
/* RATS: ignore */
char large[STR_MAX] = {0};

/* A string that exceeds STR_MAX. */
/* RATS: ignore */
char huge[STR_MAX + 1U] = {0};

/* Tests. */
const struct signature tests[] = {
	/* Overly long string. */
	{huge, ",", large, NULL, ERR_STR_LEN},

	/* Barely fitting string. */
	{large, ",", large, NULL, OK},

	/* Simple test. */
	{"a,b", ",", "a", "b", OK},

	/* Empty strings. */
	{",b", ",", "", "b", OK},
	{"a,", ",", "a", "", OK},
	{"a,b", "", "a,b", NULL, OK},

	/* Environment-like tests. */
	{"foo=bar", "=", "foo", "bar", OK},
	{"foo=", "=", "foo", "", OK},
	{"foo==bar", "=", "foo", "=bar", OK},
	{"=bar", "=", "", "bar", OK},
	{"foo", "=", "foo", NULL, OK},

	/* Terminator. */
	{NULL, NULL, NULL, NULL, OK}
};



int
main(void)
{
	(void) memset(huge, 'x', STR_MAX);
	(void) memset(large, 'x', STR_MAX - 1U);

	for (int i = 0; tests[i].s; i++) {
		const struct signature t = tests[i];
		/* RATS: ignore */
		char head[STR_MAX] = {0};
		char *tail;
		enum error ret;

		ret = str_split(t.s, t.sep, &head, &tail);

		if (ret != t.ret) {
			die("str_split '%s' '%s' returned %u.",
			    t.s, t.sep, ret);
		}
		if (!(t.head == head || strcmp(t.head, head) == 0)) {
			die("str_split '%s' '%s': head is '%s'.",
			    t.s, t.sep, head);
		}
		if (!(t.tail == tail || strcmp(t.tail, tail) == 0)) {
			die("str_split '%s' '%s': tail is '%s'.",
			    t.s, t.sep, head);
		}
	}

	return EXIT_SUCCESS;
}
