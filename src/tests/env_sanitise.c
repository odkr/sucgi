/*
 * Test env_restore.
 */

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "../env.h"
#include "../err.h"
#include "../str.h"

#include "env.h"
#include "utils.h"
#include "str.h"


/*
 * Globals
 */

extern char **environ;


/*
 * Functions
 */

/*
 * Run env_sanitise with the given patterns, and return its return code.
 * Patters are whitespace-separated strings.
 */
error
env_sanitise_(const char *keep, const char *toss)
{
	/* Flawfinder: ignore */
	char *keepv[ENV_MAX] = {NULL};	/* Array of keep patterns. */
	/* Flawfinder: ignore */
	char *tossv[ENV_MAX] = {NULL};	/* Array of toss patterns. */

	assert(str_splitn(keep, " \f\n\r\t\v", ENV_MAX, keepv, NULL) == OK);
	assert(str_splitn(toss, " \f\n\r\t\v", ENV_MAX, tossv, NULL) == OK);
	return env_sanitise(
	        (const char *const *const) keepv,
	        (const char *const *const) tossv
	);
}


/*
 * Main
 */

int
main (void) {
	/* Flawfinder: ignore */
	char huge[STR_MAX +1] = "";	/* A huge string. */
	char *var = NULL;		/* An environment variable. */


	/*
	 * Failures
	 */

	memset(huge, 'x', STR_MAX);
	assert(strnlen(huge, STR_MAX + 1) == STR_MAX);
	env_init(2, huge);
	assert(env_sanitise_("", "") == ERR_STR_MAX);

	env_init(2, "");
	assert(env_sanitise_("", "") == ERR_VAR_INVALID);

	env_init(2, "foo");
	assert(env_sanitise_("foo", "") == ERR_VAR_INVALID);

	env_init(2, "=bar");
	assert(env_sanitise_("bar", "") == ERR_VAR_INVALID);


	/*
	 * Simple tests
	 */

	env_init(1);
	assert(setenv("foo", "foo", 1) == 0);
	assert(setenv("bar", "bar", 1) == 0);
	assert(setenv("baz", "baz", 1) == 0);
	assert(env_sanitise_("foo", "") == OK);
	/* Flawfinder: ignore */
	var = getenv("foo");
	assert(var);
	assert(str_eq(var, "foo"));
	/* Flawfinder: ignore */
	var = getenv("bar");
	assert(!var);
	/* Flawfinder: ignore */
	var = getenv("baz");
	assert(!var);

	env_init(1);
	assert(setenv("foo", "foo", 1) == 0);
	assert(env_sanitise_("", "") == OK);
	/* Flawfinder: ignore */
	var = getenv("foo");
	assert(!var);

	env_init(1);
	assert(setenv("foo", "foo", 1) == 0);
	assert(setenv("bar", "bar", 1) == 0);
	assert(setenv("baz", "baz", 1) == 0);
	assert(env_sanitise_("foo b*", "foo") == OK);
	/* Flawfinder: ignore */
	var = getenv("foo");
	assert(!var);
	/* Flawfinder: ignore */
	var = getenv("bar");
	assert(var);
	assert(str_eq(var, "bar"));
	/* Flawfinder: ignore */
	var = getenv("baz");
	assert(var);
	assert(str_eq(var, "baz"));


	/*
	 * Odd values
	 */

	env_init(1);
	assert(setenv("empty", "", 1) == 0);
	assert(setenv("assign", "==bar==", 1) == 0);
	assert(setenv("space", " ", 1) == 0);
	assert(setenv("tab", "\t", 1) == 0);
	assert(setenv("lf", "\n", 1) == 0);
	assert(env_sanitise_("empty assign space tab lf", "") == OK);
	/* Flawfinder: ignore */
	var = getenv("empty");
	assert(var);
	assert(str_eq(var, ""));
	/* Flawfinder: ignore */
	var = getenv("assign");
	assert(var);
	assert(str_eq(var, "==bar=="));
	/* Flawfinder: ignore */
	var = getenv("space");
	assert(var);
	assert(str_eq(var, " "));
	/* Flawfinder: ignore */
	var = getenv("tab");
	assert(var);
	assert(str_eq(var, "\t"));
	/* Flawfinder: ignore */
	var = getenv("lf");
	assert(var);
	assert(str_eq(var, "\n"));
}
