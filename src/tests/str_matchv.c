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
	assert(str_matchv("foo", (cchar[]) {"foo", NULL}, 0));
	assert(str_matchv("foo", (cchar[]) {"f*", NULL}, 0));
	assert(str_matchv("foo", (cchar[]) {"b", "f*", NULL}, 0));
	assert(str_matchv("foo", (cchar[]) {"f*", "b", NULL}, 0));
	assert(str_matchv("foo", (cchar[]) {"f*", "", NULL}, 0));
	assert(str_matchv("foo", (cchar[]) {"", "f*", NULL}, 0));
	assert(str_matchv(".", (cchar[]) {".", "*", NULL}, 0));

	assert(!str_matchv("foo", (cchar[]) {"bar", NULL}, 0));
	assert(!str_matchv("foo", (cchar[]) {"b*", NULL}, 0));
	assert(!str_matchv("foo", (cchar[]) {"b*", "z*", NULL}, 0));
	assert(!str_matchv("foo", (cchar[]) {"b*", "", NULL}, 0));
	assert(!str_matchv("foo", (cchar[]) {"", NULL}, 0));
	assert(!str_matchv(".", (cchar[]) {"*", NULL}, FNM_PERIOD));

	/* All good. */
	return EXIT_SUCCESS;
}
