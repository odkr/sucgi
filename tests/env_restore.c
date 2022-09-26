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
struct tcase {
	char *env_vars[SC_ENV_MAX];
	const char *env_keep[SC_ENV_MAX];
	const char *env_toss[SC_ENV_MAX];
	const char *env_clean[SC_ENV_MAX];
	const enum error ret;
};


/*
 * Globals
 */

/* String that is longer than STR_MAX. */
char huge[STR_MAX + 1U] = {0};	/* RATS: ignore */

/* Tests. */
struct tcase tests[] = {
	/* Errors. */
	{{huge, NULL}, NIL, NIL, NIL, SC_ERR_ENV_LEN},
	{{"", NULL}, NIL, NIL, NIL, SC_ERR_ENV_MAL},
	{{"foo", NULL}, NIL, NIL, NIL, SC_ERR_ENV_MAL},
	{{"=foo", NULL}, NIL, NIL, NIL, SC_ERR_ENV_MAL},

	/* Other illegal names. */
	{{" foo=foo", NULL}, NIL, NIL, NIL, SC_ERR_ENV_MAL},
	{{"1foo=foo", NULL}, NIL, NIL, NIL, SC_ERR_ENV_MAL},
	{{"*=foo", NULL}, NIL, NIL, NIL, SC_ERR_ENV_MAL},
	{{"FOO =foo", NULL}, NIL, NIL, NIL, SC_ERR_ENV_MAL},
	{{"$(foo)=foo", NULL}, NIL, NIL, NIL, SC_ERR_ENV_MAL},
	{{"`foo`=foo", NULL}, NIL, NIL, NIL, SC_ERR_ENV_MAL},

	/* Simple tests. */
	{{"foo=bar", NULL}, {"foo", NULL}, NIL, {"foo=bar", NULL}, SC_OK},
	{{"foo=bar", NULL}, NIL, NIL, NIL, SC_OK},
	{{"foo=bar", NULL}, {"foo", NULL}, {"foo", NULL}, NIL, SC_OK},
	{{"foo=foo", "bar=bar", "baz=baz", NULL}, {"*", NULL}, NIL,
	 {"foo=foo", "bar=bar", "baz=baz", NULL}, SC_OK},
	{{"foo=foo", "bar=bar", "baz=baz", NULL}, {"f*", "b*", NULL}, NIL,
	 {"foo=foo", "bar=bar", "baz=baz", NULL}, SC_OK},
	{{"foo=foo", "bar=bar", "baz=baz", NULL}, {"f*", NULL}, NIL,
	 {"foo=foo", "bar", "baz", NULL}, SC_OK},
	{{"foo=foo", "bar=bar", "baz=baz", NULL}, {"*", NULL}, {"*", NULL},
	 {"foo", "bar", "baz", NULL}, SC_OK},
	{{"foo=foo", "bar=bar", "baz=baz", NULL}, {"*", NULL}, {"f*", NULL},
	 {"foo", "bar=bar", "baz=baz", NULL}, SC_OK},
	{{"foo=foo", "bar=bar", "baz=baz", NULL}, {"f*", NULL}, {"b*", NULL},
	 {"foo=foo", "bar", "baz", NULL}, SC_OK},
	
	/* Odd, but legal, values. */
	{{"empty=", "assign==", "space= ", "tab=\t", "lf=\n", NULL},
	 {"*", NULL}, NIL,
	 {"empty=", "assign==", "space= ", "tab=\t", "lf=\n", NULL}, SC_OK},

	/* Terminator. */
	{NIL, NIL, NIL, NIL, SC_OK}
};


/*
 * Functions
 */

static enum error
env_init(char *const vars[SC_ENV_MAX])
{
	size_t n = 0;

	while (vars[n]) n++;
	environ = calloc(n + 1, sizeof(char *));
	if (!environ) return SC_ERR_SYS;
	for (size_t i = 0; i < n; i++) environ[i] = vars[i];

	return SC_OK;
}


/*
 * Main
 */

int
main (void) {
	(void) memset((void *) huge, 'x', STR_MAX);

	for (int i = 0; tests[i].env_vars[0]; i++) {
		const struct tcase t = tests[i];
		const char *vars[SC_ENV_MAX];
		enum error ret;

		req(env_init(t.env_vars) == SC_OK,
		    "failed to initialise the environment.");
		req(env_clear(&vars) == SC_OK,
		    "failed to clear the environment.");

		ret = env_restore(vars, t.env_keep, t.env_toss);

		req(ret == t.ret,
		    "test %d: env_restore returned %u, not %u.",
		    i + 1, ret, t.ret);

		for (int j = 0; t.env_clean[j]; j++) {
			const char *v = t.env_clean[j];
			char name[STR_MAX];
			char *exp;
			char *val;

			req(str_split(v, "=", &name, &exp) == SC_OK,
			    "failed to split variable %s.", v);

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
