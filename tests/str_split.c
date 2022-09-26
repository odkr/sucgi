/*
 * Test str_split.
 */

#include <stdlib.h>
#include <string.h>

#include "../err.h"
#include "../str.h"
#include "../tools/lib.h"


/* Test case. */
struct tcase {
	const char *s;
	const char *sep;
	const char *head;
	const char *tail;
	enum error ret;
};

/* A string just within limits. */
char large[STR_MAX] = {0};

/* A string that exceeds STR_MAX. */
char huge[STR_MAX + 1U] = {0};

/* Tests. */
const struct tcase tests[] = {
	/* Overly long string. */
	{huge, ",", large, NULL, SC_ERR_STR_LEN},

	/* Barely fitting string. */
	{large, ",", large, NULL, SC_OK},

	/* Simple test. */
	{"a,b", ",", "a", "b", SC_OK},

	/* Empty strings. */
	{",b", ",", "", "b", SC_OK},
	{"a,", ",", "a", "", SC_OK},
	{"a,b", "", "a,b", NULL, SC_OK},

	/* Environment-like tests. */
	{"foo=bar", "=", "foo", "bar", SC_OK},
	{"foo=", "=", "foo", "", SC_OK},
	{"foo==bar", "=", "foo", "=bar", SC_OK},
	{"=bar", "=", "", "bar", SC_OK},
	{"foo", "=", "foo", NULL, SC_OK},

	/* Terminator. */
	{NULL, NULL, NULL, NULL, SC_OK}
};



int
main(void)
{
	(void) memset(huge, 'x', STR_MAX);
	(void) memset(large, 'x', STR_MAX - 1U);

	for (int i = 0; tests[i].s; i++) {
		const struct tcase t = tests[i];
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
