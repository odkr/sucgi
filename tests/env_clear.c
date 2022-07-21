/* 
 * Test env_clear.
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "../env.h"
#include "../err.h"
#include "../str.h"

extern char **environ;

int
main (void) {
	char **env = NULL;
	char **var = NULL;
	int n = 0;

	env_clear(NULL);

	assert(setenv("foo", "foo", 1) == 0);
	env_clear(&env);
	// flawfinder: ignore
	assert(getenv("foo") == NULL);

	for (var = environ; *var; var++) n++;
	assert(n == 0);

	n = 0;
	for (var = env; *var; var++) {
		char *name, *value;
		assert(str_vsplit(*var, "=", 2, &name, &value) == OK);
		if (str_eq(name, "foo")) {
			assert(str_eq(value, "foo"));
			n++;
		}
	}
	assert(n == 1);

	return EXIT_SUCCESS;
}
