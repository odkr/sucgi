/* 
 * Test path_check_len.
 */

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../err.h"
#include "../path.h"
#include "../str.h"


int
main (void) {
	/*
	 * Test errors.
	 */
	
	{
		char *path = NULL;
		path = calloc((size_t) STR_MAX_LEN + 2, sizeof (char));
		assert(path);
		memset(path, 'x', (size_t) STR_MAX_LEN + 1);
		assert(strnlen(path, STR_MAX_LEN + 2) > STR_MAX_LEN);
		assert(path_check_len(path) == ERR_STR_LEN);
		free(path);
	}

#if PATH_MAX > -1
	{
		char *path = NULL;
		path = calloc(PATH_MAX + 2, sizeof (char));
		assert(path);
		memset(path, 'x', PATH_MAX + 1);
		assert(strnlen(path, PATH_MAX + 2) > PATH_MAX);
		assert(path_check_len(path) == ERR_STR_LEN);
		free(path);
	}
#endif


	/*
	 * Simple tests.
	 */

	{
		char *path = NULL;
		path = calloc(STR_MAX_LEN, sizeof(char));
		assert(path);
		assert(getcwd(path, STR_MAX_LEN));
		path_check_len(path);
		free(path);
	}


	/*
	 * All good.
	 */

	return EXIT_SUCCESS;
}
