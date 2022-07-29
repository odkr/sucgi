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


/* FIXME Undocumented. */
static error
test_longer_than(size_t len)
{
	size_t n = len + 1;
	/* flawfinder: ignore */
	char path[n + 1];

	memset(path, 'c', n);
	path[n] = '\0';
	assert(strnlen(path, n) > len);

	return path_check_len(path);
}

int
main (void) {
	/* flawfinder: ignore */
	char cwd[STR_MAX_LEN];
	
	assert(test_longer_than(STR_MAX_LEN) == ERR_STR_LEN);

#if PATH_MAX > -1
	assert(test_longer_than(PATH_MAX) == ERR_STR_LEN);
#endif

#if NAME_MAX > -1
	assert(test_longer_than(NAME_MAX) == ERR_FNAME_LEN);
#endif

	assert(getcwd(cwd, STR_MAX_LEN));
	assert(cwd);
	assert(path_check_len(cwd) == OK);

	return EXIT_SUCCESS;
}
