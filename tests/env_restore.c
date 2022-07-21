/* 
 * Test env_restore.
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "../env.h"
#include "../err.h"
#include "../str.h"
#include "utils.h"

#include <stdio.h>

/*
 * Globals
 */

extern char **environ;


/*
 * Functions
 */

/* Create a new environment. */
enum code
env_create (size_t n)
{
	environ = malloc(sizeof(char *) * n);
	if (!environ) return ERR_SYS;
	*environ = NULL;
	return OK;
}


/* 
 * env_restore wrapper that clears the environment and
 * accepts strings as arguments.
 */
enum code
env_restore_w (const char *keep, const char *toss)
{
	char **vars;		/* Backup of the environment. */
	char **keepv, **tossv;	/* Arrays of patterns. */

	env_clear(&vars);
	assert(str_words(keep, &keepv) == OK);
	assert(str_words(toss, &tossv) == OK);
	return env_restore((const char *const *) vars, keepv, tossv);
}


/*
 * Main
 */

int
main (void) {
	char *var = NULL;	/* An environment variable. */

	/*
	 * Test failures.
	 */

	env_create(2);
	environ[0] = "";
	environ[1] = NULL;
	assert(env_restore_w("", "") == ERR_VAR_INVALID);

	env_create(2);
	environ[0] = "foo";
	environ[1] = NULL;
	assert(env_restore_w("foo", "") == ERR_VAR_INVALID);

	env_create(2);
	environ[0] = "=bar";
	environ[1] = NULL;
	assert(env_restore_w("bar", "") == ERR_VAR_INVALID);

	env_create(2);
	var = malloc(sizeof(char) * (STR_MAX_LEN + 2));
	assert(var);
	memset(var, 'x', STR_MAX_LEN + 1);
	var[STR_MAX_LEN + 1] = '\0';
	assert(strnlen(var, STR_MAX_LEN + 2) == STR_MAX_LEN + 1);
	environ[0] = var;
	environ[1] = NULL;
	assert(env_restore_w("", "") == ERR_STR_LEN);
	free(var);

	/*
	 * Simple tests.
	 */

	env_create(1);
	assert(setenv("foo", "foo", 1) == 0);
	assert(setenv("bar", "bar", 1) == 0);
	assert(setenv("baz", "baz", 1) == 0);
	assert(env_restore_w("foo", "") == OK);
	// flawfinder: ignore
	var = getenv("foo");
	assert(str_eq(var, "foo"));
	// flawfinder: ignore
	var = getenv("bar");
	assert(!var);
	// flawfinder: ignore
	var = getenv("baz");
	assert(!var);

	env_create(1);
	assert(setenv("foo", "foo", 1) == 0);
	assert(env_restore_w("", "") == OK);
	// flawfinder: ignore
	var = getenv("foo");
	assert(!var);

	env_create(1);
	assert(setenv("foo", "foo", 1) == 0);
	assert(setenv("bar", "bar", 1) == 0);
	assert(setenv("baz", "baz", 1) == 0);
	assert(env_restore_w("foo b*", "foo") == OK);
	// flawfinder: ignore
	var = getenv("foo");
	assert(var == NULL);
	// flawfinder: ignore
	var = getenv("bar");
	assert(var);
	assert(str_eq(var, "bar"));
	// flawfinder: ignore
	var = getenv("baz");
	assert(var);
	assert(str_eq(var, "baz"));

	env_create(1);
	assert(setenv("empty", "", 1) == 0);
	assert(setenv("assign", "==bar==", 1) == 0);
	assert(setenv("space", " ", 1) == 0);
	assert(setenv("tab", "\t", 1) == 0);
	assert(setenv("lf", "\n", 1) == 0);
	assert(env_restore_w("empty assign space tab lf", "") == OK);
	// flawfinder: ignore
	var = getenv("empty");
	assert(var);
	assert(str_eq(var, ""));
	// flawfinder: ignore
	var = getenv("assign");
	assert(var);
	assert(str_eq(var, "==bar=="));
	// flawfinder: ignore
	var = getenv("space");
	assert(var);
	assert(str_eq(var, " "));
	// flawfinder: ignore
	var = getenv("tab");
	assert(var);
	assert(str_eq(var, "\t"));
	// flawfinder: ignore
	var = getenv("lf");
	assert(var);
	assert(str_eq(var, "\n"));
}
