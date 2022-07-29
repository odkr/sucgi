/*
 * Test env_clear.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../env.h"
#include "../err.h"
#include "../str.h"
#include "utils.h"


extern char **environ;

int
main (void) {
	/* flawfinder: ignore */
	char *env[ENV_MAX] = {0};
	char **var = NULL;
	int n = 0;

	env_clear(NULL);

	assert(setenv("foo", "foo", 1) == 0);
	assert(env_clear(env) == OK);
	/* flawfinder: ignore */
	assert(getenv("foo") == NULL);

	for (var = environ; *var; var++) n++;
	assert(n == 0);

	n = 0;
	for (var = env; *var; var++) {
		char *name = calloc(STR_MAX_LEN + 1, sizeof(char));
		char *value = calloc(STR_MAX_LEN + 1, sizeof(char));
		assert(name && value);

		assert(str_vsplit(*var, "=", 2, name, value) == OK);
		if (str_eq(name, "foo")) {
			assert(str_eq(value, "foo"));
			n++;
		}

		free(name);
		free(value);
	}
	assert(n == 1);

	assert(env_clear(NULL) == OK);
	for (int i = 0; i <= ENV_MAX; i++) {
		/* flawfinder: ignore */
		char name[STR_MAX_LEN];
		/* flawfinder: ignore */
		assert(snprintf(name, STR_MAX_LEN, "foo%d", i) > 0);
		assert(setenv(name, "foo", true) == 0);
	}

	assert(env_clear(env) == ERR_ENV_MAX);

	return EXIT_SUCCESS;
}
