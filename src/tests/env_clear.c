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

#include "env.h"
#include "utils.h"


int
main (void) {
	/* Flawfinder: ignore */
	char *env[VAR_MAX] = {NULL};	/* A backup of the environment. */
	char **var = NULL;		/* An environment variable. */
	int n = 0;			/* Number of variables found. */

	/* 
	 * Start with a clean environmenet, hopefully.
	 */

	assert(env_clear(NULL) == OK);


	/*
	 * Check whether the environment is cleared.
	 */

	assert(setenv("foo", "foo", 1) == 0);
	assert(env_clear(env) == OK);
	/* Flawfinder: ignore */
	assert(getenv("foo") == NULL);
	for (var = environ; *var; var++) n++;
	assert(0 == n);


	/*
	 * Check whether the previous environment has been stored in env.
	 */

	n = 0;
	for (var = env; *var; var++) {
		/* Flawfinder: ignore */
		char name[STR_MAX] = {0};
		char *value = NULL;

		assert(str_split(*var, "=", &name, &value) == OK);
		if (str_eq(name, "foo")) {
			assert(str_eq(value, "foo"));
			n++;
		}
	}
	assert(1 == n);


	/*
	 * Check whether env_clear errors out if there are too many variables.
	 */
	
	assert(env_clear(NULL) == OK);
	
	for (size_t i = 0; i <= VAR_MAX; i++) {
		/* Flawfinder: ignore */
		char name[STR_MAX] = {0};
		
		/* Flawfinder: ignore */
		assert(snprintf(name, STR_MAX - 1U, "foo%zu", i) > 0);
		assert(setenv(name, "foo", true) == 0);
	}

	assert(env_clear(env) == ERR_VAR_MAX);

	/*
	 * All good.
	 */

	return EXIT_SUCCESS;
}
