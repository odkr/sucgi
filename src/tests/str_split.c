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
		char **subs = NULL;
		int n = 0;

		str = malloc(STR_MAX_LEN + 2);
		assert(str);
		memset(str, 'x', STR_MAX_LEN + 1);
		str[STR_MAX_LEN + 1] = '\0';
		assert(str_split(str, ",", 8, &subs, &n) == ERR_STR_LEN);
		assert(subs == NULL);
		assert(n == 0);
		free(str);
		free(subs);
	}


	/*
	 * Simple tests.
	 */

	{
		char **subs = NULL;
		int n = 0;

		assert(str_split("a,b,c", ",", 8, &subs, &n) == OK);
		assert(n == 3);
		assert(str_eq(subs[0], "a"));
		assert(str_eq(subs[1], "b"));
		assert(str_eq(subs[2], "c"));
		assert(subs[3] == NULL);
		free(subs);
	}
	
	{
		char **subs = NULL;
		int n = 0;

		assert(str_split("a,,b,c", ",", 8, &subs, &n) == OK);
		assert(n == 4);
		assert(str_eq(subs[0], "a"));
		assert(str_eq(subs[1], ""));
		assert(str_eq(subs[2], "b"));
		assert(str_eq(subs[3], "c"));
		assert(subs[4] == NULL);
		free(subs);
	}

	{
		char **subs = NULL;
		int n = 0;

		assert(str_split("a,", ",", 8, &subs, &n) == OK);
		assert(n == 2);
		assert(str_eq(subs[0], "a"));
		assert(str_eq(subs[1], ""));
		assert(subs[2] == NULL);
		free(subs);
	}

	{
		char **subs = NULL;
		int n = 0;

		assert(str_split("a,b|c d", ",|\n\t ", 8, &subs, &n) == OK);
		assert(n == 4);
		assert(str_eq(subs[0], "a"));
		assert(str_eq(subs[1], "b"));
		assert(str_eq(subs[2], "c"));
		assert(str_eq(subs[3], "d"));
		assert(subs[4] == NULL);
		free(subs);
	}

	{
		char **subs = NULL;
		int n = 0;

		assert(str_split("", ",|\n\t ", 8, &subs, &n) == OK);
		assert(n == 1);
		assert(str_eq(subs[0], ""));
		assert(subs[1] == NULL);
		free(subs);
	}

	{
		char **subs = NULL;
		int n = 0;

		assert(str_split("a,b,c", "", 8, &subs, &n) == OK);
		assert(n == 1);
		assert(str_eq(subs[0], "a,b,c"));
		assert(subs[1] == NULL);
		free(subs);
	}

	/*
	 * Test non-occurrence.
	 */

	{
		char **subs = NULL;
		int n = 0;
	
		assert(str_split("a", ",", 8, &subs, &n) == OK);
		assert(n == 1);
		assert(str_eq(subs[0], "a"));
		assert(subs[1] == NULL);
		free(subs);
	}


	/*
	 * Test large lists.
	 */

	{
		char *str = NULL;
		char **subs = NULL;
		int n = 0;

		// cppcheck-suppress pointerSize
		str = malloc(sizeof(char *) * 513);
		assert(str);
		for (int i = 0; i < 512; i+=2) {
			str[i] = 'x';
			str[i + 1] = ',';
		}
		str[512] = '\0';
		assert(strnlen(str, 1024) == 512);

		assert(str_split(str, ",", 512, &subs, &n) == OK);
		assert(n == 257);
		for (int i = 0; i < 256; i++) assert(str_eq(subs[i], "x"));
		assert(str_eq(subs[256], ""));

		free(str);
		free(subs);
	}


	/*
	 * Test maxsplit.
	 */

	{
		char **subs = NULL;
		int n = 0;
		
		assert(str_split(",,a,b,c,,", ",", 0, &subs, &n) == OK);
		assert(n == 1);
		assert(str_eq(subs[0], ",,a,b,c,,"));
		assert(subs[1] == NULL);
		free(subs);
	}

	{
		char **subs = NULL;
		int n = 0;

		assert(str_split("a,b,c,d,e", ",",3, &subs, &n) == OK);
		assert(n == 4);
		assert(str_eq(subs[0], "a"));
		assert(str_eq(subs[1], "b"));
		assert(str_eq(subs[2], "c"));
		assert(str_eq(subs[3], "d,e"));
		assert(subs[4] == NULL);
		free(subs);
	}


	/*
	 * Test usage similar to env_restore.
	 */

	{
		char **subs = NULL;
		int n = 0;

		assert(str_split("foo b*", " \t\n", 128, &subs, &n) == OK);
		assert(n == 2);
		assert(str_eq(subs[0], "foo"));
		assert(str_eq(subs[1], "b*"));
		assert(subs[2] == NULL);
		free(subs);
	}

	{
		char **subs = NULL;
		int n = 0;

		assert(str_split("foo==bar", "=", 1, &subs, &n) == OK);
		assert(n == 2);
		assert(str_eq(subs[0], "foo"));
		assert(str_eq(subs[1], "=bar"));
		assert(subs[2] == NULL);
		free(subs);
	}


	/*
	 * All good.
	 */

	return EXIT_SUCCESS;
}
