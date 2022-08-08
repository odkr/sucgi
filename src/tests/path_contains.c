/*
 * Test path_contains.
 */

#include <assert.h>
#include <stdlib.h>

#include "../path.h"

int
main (void) {

	/*
	 * Absolute paths.
	 */

	assert(path_contains("/", "/foo"));
	assert(path_contains("/foo", "/foo/bar"));
	assert(!path_contains("/foo", "/bar"));
	assert(!path_contains("/bar", "/foo"));
	assert(!path_contains("/foo", "/foobar"));
	assert(!path_contains("/", "foo"));
	assert(!path_contains("/foo", "/"));
	assert(!path_contains("/foo", "/foo"));
	assert(!path_contains("/", "/"));


	/*
	 * Relative paths
	 */

	assert(path_contains("foo", "foo/bar"));
	assert(!path_contains("foo", "foo"));
	assert(!path_contains("bar", "foo"));


	/*
	 * Paths with leading dot
	 */

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


	/*
	 * All good
	 */

	return EXIT_SUCCESS;
}
