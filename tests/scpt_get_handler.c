/*
 * Test scpt_get_handler.
 */

#include <stdio.h>
#include <string.h>

#include "../err.h"
#include "../scpt.h"
#include "../tools/lib.h"


/* Test case. */
struct signature {
	const char *scpt;
	const char *handler;
	const enum error ret;
};


/* Tests. */
const struct signature tests[] = {
	/* Simple errors. */
	{"file", NULL, ERR_SCPT_NO_SFX},
	{".", NULL, ERR_SCPT_ONLY_SFX},
	{".sh", NULL, ERR_SCPT_ONLY_SFX},
	{".py", NULL, ERR_SCPT_ONLY_SFX},
	{"file.null", NULL, ERR_SCPT_NO_HDL},
	{"file.empty", NULL, ERR_SCPT_NO_HDL},
	{"file.py", NULL, ERR_SCPT_NO_HDL},
	{"file.post", NULL, ERR_SCPT_NO_HDL},

	/* Empty string shenanigans. */
	{" ", NULL, ERR_SCPT_NO_SFX},
	{". ", NULL, ERR_SCPT_ONLY_SFX},
	{".sh ", NULL, ERR_SCPT_ONLY_SFX},
	{".py ",NULL, ERR_SCPT_ONLY_SFX},
	{" .null", NULL, ERR_SCPT_NO_HDL},
	{" .empty", NULL, ERR_SCPT_NO_HDL},
	{" .py", NULL, ERR_SCPT_NO_HDL},
	{" .post", NULL, ERR_SCPT_NO_HDL},
	{" . ", NULL, ERR_SCPT_NO_HDL},

	/* Simple test. */
	{"file.sh", "sh", OK},
	{"file.", "dot", OK},

	/* Terminator. */
	{NULL, NULL, OK}
};

/* Prefixes should make no difference. */
const char *prefixes[] = {
	"", "/", "./", "dir/", "/dir/", " /", NULL
};

/* Script handler database for testing. */
const struct scpt_ent hdb[] = {
	{"", "unreachable"},
	{".", "dot"},
	{".sh", "sh"},
	{".null", NULL},
	{".empty", ""},
	{".pre", "pre"},
	{NULL, NULL},
	{".post", "post"},
};


int
main (void)
{
	for (int i = 0; tests[i].scpt; i++) {
 		for (int j = 0; prefixes[j]; j++) {
			/* RATS: ignore */
			char scpt[STR_MAX] = {0};
			const struct signature t = tests[i];
			const char *prefix = prefixes[j];
			const char *handler = NULL;
			enum error ret;	
			int n;
		
			/* RATS: ignore */
			n = snprintf(scpt, STR_MAX, "%s%s", prefix, t.scpt);
			if (n >= STR_MAX) {
				croak("test %d: input too long.", i);
			}

			ret = scpt_get_handler(hdb, scpt, &handler);

			if (t.ret != ret) {
				croak("scpt_get_handler %s returned %u.\n",
				      scpt, ret);
			}
			if (!(t.handler == handler ||
			      strcmp(t.handler, handler) == 0))
			{
				croak("scpt_get_handler %s: got %s.\n",
				      scpt, handler);
			}
		}
	}

	return EXIT_SUCCESS;
}
