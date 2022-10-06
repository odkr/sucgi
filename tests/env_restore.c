/*
 * Test env_restore.
 */

#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../env.h"
#include "../err.h"
#include "../str.h"
#include "../tools/lib.h"


/*
 * Constants
 */

/* Shorthand for an empty list. */
#define NIL {NULL}


/*
 * Data types
 */

/* Test case. */
struct signature {
	char *env_vars[ENV_MAX];		/* RATS: ignore */
	const char *safe_vars[ENV_MAX];		/* RATS: ignore */
	const char *env_clean[ENV_MAX];		/* RATS: ignore */
	const enum error ret;
};


/*
 * Globals
 */

/* String that is longer than STR_MAX. */
char huge[STR_MAX + 1U] = {0};	/* RATS: ignore */

/* Tests. */
struct signature tests[] = {
	/* Errors. */
	{{huge, NULL}, NIL, NIL, ERR_ENV_LEN},
	{{"", NULL}, NIL, NIL, ERR_ENV_MAL},
	{{"foo", NULL}, NIL, NIL, ERR_ENV_MAL},
	{{"=foo", NULL}, NIL, NIL, ERR_ENV_MAL},

	/* Other illegal names. */
	{{" foo=foo", NULL}, NIL, NIL, ERR_ENV_MAL},
	{{"1foo=foo", NULL}, NIL, NIL, ERR_ENV_MAL},
	{{"*=foo", NULL}, NIL, NIL, ERR_ENV_MAL},
	{{"FOO =foo", NULL}, NIL, NIL, ERR_ENV_MAL},
	{{"$(foo)=foo", NULL}, NIL, NIL, ERR_ENV_MAL},
	{{"`foo`=foo", NULL}, NIL, NIL, ERR_ENV_MAL},

	/* Simple tests. */
	{{"foo=bar", NULL}, {"foo", NULL}, {"foo=bar", NULL}, OK},
	{{"foo=bar", NULL}, NIL, NIL, OK},
	{{"foo=foo", "bar=bar", "baz=baz", NULL}, {"*", NULL},
	 {"foo=foo", "bar=bar", "baz=baz", NULL}, OK},
	{{"foo=foo", "bar=bar", "baz=baz", NULL}, {"f*", "b*", NULL},
	 {"foo=foo", "bar=bar", "baz=baz", NULL}, OK},
	{{"foo=foo", "bar=bar", "baz=baz", NULL}, {"f*", NULL},
	 {"foo=foo", "bar", "baz", NULL}, OK},
	
	/* Odd, but legal, values. */
	{{"empty=", "assign==", "space= ", "tab=\t", "lf=\n", NULL},
	 {"*", NULL},
	 {"empty=", "assign==", "space= ", "tab=\t", "lf=\n", NULL}, OK},

	/* Terminator. */
	{NIL, NIL, NIL, OK}
};


/*
 * Functions
 */

static enum error
env_init(/* RATS: ignore */
	 char *const vars[ENV_MAX])
{
	size_t n = 0;

	while (vars[n]) n++;
	environ = calloc(n + 1U, sizeof(char *));
	if (!environ) return ERR_SYS;
	for (size_t i = 0; i < n; i++) environ[i] = vars[i];

	return OK;
}


/*
 * Main
 */

int
main (void) {
	(void) memset((void *) huge, 'x', STR_MAX);

	for (int i = 0; tests[i].env_vars[0]; i++) {
		/* RATS: ignore */
		const char *vars[ENV_MAX];
		const struct signature t = tests[i];
		enum error ret;

		req(env_init(t.env_vars) == OK,
		    "failed to initialise the environment.");
		req(env_clear(&vars) == OK,
		    "failed to clear the environment.");

		ret = env_restore(vars, t.safe_vars);

		req(ret == t.ret,
		    "test %d: env_restore returned %u, not %u.",
		    i + 1, ret, t.ret);

		for (int j = 0; t.env_clean[j]; j++) {
			const char *v = t.env_clean[j];
			const char *val;
			/* RATS: ignore */
			char name[STR_MAX];
			char *exp;

			req(str_split(v, "=", &name, &exp) == OK,
			    "failed to split variable %s.", v);

                        /* RATS: ignore */
			val = getenv(name);
			
			if (val && !exp) {
			    croak("test %d: $%s is set.", i + 1, name);
			}
			if (!val && exp) {
			    croak("test %d: $%s is unset.", i + 1, name);
			}
			if (val && exp && strcmp(val, exp) != 0) {
				croak("test %d: $%s is '%s'.",
				      i + 1, name, val);
			}
		}
	}

	return EXIT_SUCCESS;
}
