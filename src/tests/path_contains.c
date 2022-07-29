/*
 * Test str_starts_w.
 */

#include <assert.h>
#include <stdlib.h>

#include "../path.h"

int
main (void) {
	assert(path_contains("/", "/foo"));
	assert(path_contains("/foo", "/foo/bar"));

	assert(!path_contains("/foo", "/bar"));
	assert(!path_contains("/bar", "/foo"));
	assert(!path_contains("/foo", "/foobar"));
	assert(!path_contains("/", "foo"));
	assert(!path_contains("/foo", "/"));
	assert(!path_contains("/foo", "/foo"));
	assert(!path_contains("/", "/"));

	assert(path_contains(".", "./foo"));
	assert(path_contains("./foo", "./foo/bar"));

	assert(!path_contains("./bar", "./foo"));
	assert(!path_contains("./foo", "."));
	assert(!path_contains("./foo", "./"));
	assert(!path_contains("./foo", "./foo"));
	assert(!path_contains(".", "."));
	assert(!path_contains(".", ".foo"));
	assert(!path_contains(".f", ".foo"));
	assert(!path_contains(".foo", ".foo"));

	assert(path_contains("foo", "foo/bar"));
	assert(!path_contains("foo", "foo"));
	assert(!path_contains("bar", "foo"));

	assert(!path_contains("", "/"));
	assert(!path_contains("", "."));
	assert(!path_contains("", "foo"));
	assert(!path_contains("/", ""));
	assert(!path_contains(".", ""));
	assert(!path_contains("foo", ""));
	assert(!path_contains("", ""));	
	
	return EXIT_SUCCESS;
}
