/*
 * Test scpt_get_handler.
 */

#include <stdio.h>
#include <string.h>

#include "../err.h"
#include "../scpt.h"
#include "../tools/lib.h"


/* Test case. */
struct tcase {
	const char *scpt;
	const char *handler;
	const enum error ret;
};


/* Tests. */
const struct tcase tests[] = {
	/* Simple errors. */
	{"file", NULL, SC_ERR_SCPT_NO_SFX},
	{".", NULL, SC_ERR_SCPT_ONLY_SFX},
	{".sh", NULL, SC_ERR_SCPT_ONLY_SFX},
	{".py", NULL, SC_ERR_SCPT_ONLY_SFX},
	{"file.null", NULL, SC_ERR_SCPT_NO_HDL},
	{"file.empty", NULL, SC_ERR_SCPT_NO_HDL},
	{"file.py", NULL, SC_ERR_SCPT_NO_HDL},
	{"file.post", NULL, SC_ERR_SCPT_NO_HDL},

	/* Empty string shenanigans. */
	{" ", NULL, SC_ERR_SCPT_NO_SFX},
	{". ", NULL, SC_ERR_SCPT_ONLY_SFX},
	{".sh ", NULL, SC_ERR_SCPT_ONLY_SFX},
	{".py ",NULL, SC_ERR_SCPT_ONLY_SFX},
	{" .null", NULL, SC_ERR_SCPT_NO_HDL},
	{" .empty", NULL, SC_ERR_SCPT_NO_HDL},
	{" .py", NULL, SC_ERR_SCPT_NO_HDL},
	{" .post", NULL, SC_ERR_SCPT_NO_HDL},
	{" . ", NULL, SC_ERR_SCPT_NO_HDL},

	/* Simple test. */
	{"file.sh", "sh", SC_OK},
	{"file.", "dot", SC_OK},

	/* Terminator. */
	{NULL, NULL, SC_OK}
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
			const struct tcase t = tests[i];
			const char *prefix = prefixes[j];
			const char *handler = NULL;
			char scpt[STR_MAX] = {0};
			enum error ret;	
			int n;
			
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
