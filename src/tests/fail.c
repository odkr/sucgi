/*
 * Test error.
 */

#include <assert.h>
#include <stdlib.h>
#include <syslog.h>

#include "../err.h"

int main (void) {
	fail("foo.");

	/* This point should not be reached. */
	return EXIT_SUCCESS;
}
