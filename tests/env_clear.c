/*
 * Test env_clear.
 */

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../env.h"
#include "../err.h"
#include "../str.h"

#include "../tools/lib.h"


int
main (void) {
	/* RATS: ignore */
	const char *env[ENV_MAX];	/* Backup of environ(2). */
	const char **var;		/* An environment variable. */
	int nvars;			/* The number of variables. */

	/* Start with a clean environmenet, hopefully. */
	req(env_clear(NULL) == OK, "failed to clear the environment.");

	/* Has the environment been cleared? */
	nvars = 0;
	req(setenv("foo", "bar", 1) == 0, "setenv: %s.", strerror(errno));
	req(env_clear(&env) == OK, "failed to clear the environment.");
        /* RATS: ignore */
	req(!getenv("foo"), "getenv foo: did not return NULL.");
	for (var = (const char**) environ; *var; var++) nvars++;
	req(nvars == 0, "environment not empty.");

	/* Was the environment backed-up? */
	nvars = 0;
	for (var = env; *var; var++) {
		/* RATS: ignore */
		char name[STR_MAX];
		char *value;

		req(str_split(*var, "=", &name, &value) == OK,
		    "failed to split variable.");
		if (strcmp(name, "foo") != 0) continue;
		if (strcmp(value, "bar") != 0) continue;
		nvars++;
	}
	if (nvars < 1) croak("failed to store environment.");
	if (nvars > 1) croak("stored too many variables.");

	/* Does env_clear error out if there are too many variables? */
	req(env_clear(NULL) == OK, "failed to clear the environment.");

	for (int i = 0; i <= ENV_MAX; i++) {
		/* RATS: ignore */
		char name[STR_MAX];

		/* RATS: ignore */
		req(snprintf(name, STR_MAX - 1U, "foo%d", i) > 0,
		    "failed to generate variable name.");
		req(setenv(name, "foo", true) == 0,
		    "setenv %s: %s.", name, strerror(errno));
	}
	req(env_clear(&env) == ERR_ENV_MAX,
	    "handled more than ENV_MAX variables, which is verboten.");

	/* All good. */
	return EXIT_SUCCESS;
}
