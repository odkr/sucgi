/* 
 * Test str_cp.
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "../err.h"
#include "../str.h"


int
main (void) {
	{
		// flawfinder: ignore
		char *huge = NULL;
		char *str = NULL;

		huge = malloc(STR_MAX_LEN + 2);
		assert(huge);
		memset(huge, 'x', STR_MAX_LEN + 1);
		huge[STR_MAX_LEN + 1] = '\0';
		assert(str_cp((const char*) huge, &str) == ERR_STR_LEN);
		assert(str == NULL);

		free(huge);
	}

	{
		char *str = NULL;
		assert(str_cp("foo", &str) == OK);
		assert(str_eq(str, "foo"));
		free(str);
	}

	{
		char *str = NULL;
		assert(str_cp("", &str) == OK);
		assert(str_eq(str, ""));
		free(str);
	}

	return EXIT_SUCCESS;
}
