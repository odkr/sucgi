/*
 * Test str_match.
 */

#include <assert.h>
#include <fnmatch.h>
#include <stdlib.h>

#include "../str.h"

typedef const char *const cchar;
#include <stdio.h>
int
main (void) {
	assert(str_matchn("foo", (cchar[]) {"foo", NULL}, 0));
	assert(str_matchn("foo", (cchar[]) {"f*", NULL}, 0));
	assert(str_matchn("foo", (cchar[]) {"b", "f*", NULL}, 0));
	assert(str_matchn("foo", (cchar[]) {"f*", "b", NULL}, 0));
	assert(str_matchn("foo", (cchar[]) {"f*", "", NULL}, 0));
	assert(str_matchn("foo", (cchar[]) {"", "f*", NULL}, 0));
	assert(str_matchn(".", (cchar[]) {".", "*", NULL}, 0));

	assert(!str_matchn("foo", (cchar[]) {"bar", NULL}, 0));
	assert(!str_matchn("foo", (cchar[]) {"b*", NULL}, 0));
	assert(!str_matchn("foo", (cchar[]) {"b*", "z*", NULL}, 0));
	assert(!str_matchn("foo", (cchar[]) {"b*", "", NULL}, 0));
	assert(!str_matchn("foo", (cchar[]) {"", NULL}, 0));
	assert(!str_matchn(".", (cchar[]) {"*", NULL}, FNM_PERIOD));

	/* All good. */
	return EXIT_SUCCESS;
}
