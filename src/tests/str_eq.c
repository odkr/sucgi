/* 
 * Test str_eq.
 */

#include <assert.h>
#include <stdlib.h>

#include "../str.h"

int
main (void) {
	assert(str_eq("", ""));
	assert(str_eq("foo", "foo"));
	assert(!str_eq("foo", ""));
	assert(!str_eq("foo", "bar"));

	return EXIT_SUCCESS;
}
