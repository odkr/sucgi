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
	assert(str_fnmatchn("foo", (cchar[]) {"foo", NULL}, 0));
	assert(str_fnmatchn("foo", (cchar[]) {"f*", NULL}, 0));
	assert(str_fnmatchn("foo", (cchar[]) {"b", "f*", NULL}, 0));
	assert(str_fnmatchn("foo", (cchar[]) {"f*", "b", NULL}, 0));
	assert(str_fnmatchn("foo", (cchar[]) {"f*", "", NULL}, 0));
	assert(str_fnmatchn("foo", (cchar[]) {"", "f*", NULL}, 0));
	assert(str_fnmatchn(".", (cchar[]) {".", "*", NULL}, 0));

	assert(!str_fnmatchn("foo", (cchar[]) {"bar", NULL}, 0));
	assert(!str_fnmatchn("foo", (cchar[]) {"b*", NULL}, 0));
	assert(!str_fnmatchn("foo", (cchar[]) {"b*", "z*", NULL}, 0));
	assert(!str_fnmatchn("foo", (cchar[]) {"b*", "", NULL}, 0));
	assert(!str_fnmatchn("foo", (cchar[]) {"", NULL}, 0));
	assert(!str_fnmatchn(".", (cchar[]) {"*", NULL}, FNM_PERIOD));

	/* All good. */
	return EXIT_SUCCESS;
}
