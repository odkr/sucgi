/* 
 * Test str_split.
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "../str.h"


int
main (void) {
	/*
	 * Test errors.
	 */
	
	{
		char *str = NULL;
		char *a = NULL;
		char *b = NULL;
		char *c = NULL;

		str = malloc(STR_MAX_LEN + 2);
		assert(str);
		memset(str, 'x', STR_MAX_LEN + 1);
		str[STR_MAX_LEN + 1] = '\0';

		assert(str_vsplit(str, ",", 3, &a, &b, &c) == ERR_STR_LEN);
		assert(a == NULL);
		assert(b == NULL);
		assert(c == NULL);

		free(str);
	}


	/*
	 * Simple tests.
	 */

	{
		char *a = NULL;
		char *b = NULL;
		char *c = NULL;

		assert(str_vsplit("a,b,c", ",", 3, &a, &b, &c) == OK);
		assert(str_eq(a, "a"));
		assert(str_eq(b, "b"));
		assert(str_eq(c, "c"));
		free(a);
		free(b);
		free(c);
	}

	{
		char *a = NULL;
		char *b = NULL;
		char *c = NULL;

		assert(str_vsplit("a,,b,c", ",", 3, &a, &b, &c) == OK);
		assert(str_eq(a, "a"));
		assert(str_eq(b, ""));
		assert(str_eq(c, "b,c"));
		free(a);
		free(b);
		free(c);
	}


	/*
	 * Test non-occurrence.
	 */

	{
		char *a = NULL;

		assert(str_vsplit("a", ",", 1, &a) == OK);
		assert(str_eq(a, "a"));

		free(a);
	}


	/*
	 * Test large lists.
	 */

	{
		char *str = NULL;
		char *a = NULL;
		char *b = NULL;
		char *c = NULL;

		// cppcheck-suppress pointerSize
		str = malloc(sizeof(char *) * 513);
		assert(str);
		for (int i = 0; i < 512; i+=2) {
			str[i] = 'x';
			str[i + 1] = ',';
		}
		str[512] = '\0';
		assert(strnlen(str, 1024) == 512);

		assert(str_vsplit(str, ",", 3, &a, &b, &c) == OK);
		assert(str_eq(a, "x"));
		assert(str_eq(b, "x"));
		// cppcheck-suppress nullPointer
		assert(strnlen(c, 1024) == 508);

		free(str);
		free(a);
		free(b);
		free(c);
	}


	/*
	 * Test redundant arguments.
	 */

	{
		char *a = NULL;
		char *b = NULL;

		assert(str_vsplit("a", ",", 1, &a, &b) == OK);
		assert(str_eq(a, "a"));
		assert(b == NULL);
		
		free(a);
	}


	/*
	 * Test usage similar to env_restore.
	 */

	{
		char *a = NULL;
		char *b = NULL;

		assert(str_vsplit("foo==bar", "=", 2, &a, &b) == OK);
		assert(str_eq(a, "foo"));
		assert(str_eq(b, "=bar"));
		
		free(a);
		free(b);
	}


	/*
	 * All good.
	 */

	return EXIT_SUCCESS;
}
