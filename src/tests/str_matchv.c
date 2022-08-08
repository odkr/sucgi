/*
 * Test str_match.
 */

#include <assert.h>
#include <fnmatch.h>
#include <stdlib.h>

#include "../str.h"

int
main (void) {
	assert(str_matchv("foo", (const char *const[]) {"foo", NULL}, 0));
	assert(str_matchv("foo", (const char *const[]) {"f*", NULL}, 0));
	assert(str_matchv("foo", (const char *const[]) {"b", "f*", NULL}, 0));
	assert(str_matchv("foo", (const char *const[]) {"f*", "b", NULL}, 0));
	assert(str_matchv("foo", (const char *const[]) {"f*", "", NULL}, 0));
	assert(str_matchv("foo", (const char *const[]) {"", "f*", NULL}, 0));
	assert(str_matchv(".",   (const char *const[]) {".", "*", NULL}, 0));

	assert(!str_matchv("foo", (const char *const[]) {"bar", NULL}, 0));
	assert(!str_matchv("foo", (const char *const[]) {"b*", NULL}, 0));
	assert(!str_matchv("foo", (const char *const[]) {"b*", "z*", NULL}, 0));
	assert(!str_matchv("foo", (const char *const[]) {"b*", "", NULL}, 0));
	assert(!str_matchv("foo", (const char *const[]) {"", NULL}, 0));
	assert(!str_matchv(".",   (const char *const[]) {"*", NULL}, FNM_PERIOD));

	/* All good. */
	return EXIT_SUCCESS;
}
