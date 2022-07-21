/* 
 * Test error.
 */

#include <assert.h>
#include <stdlib.h>
#include <syslog.h>

#include "../err.h"

int main (void) {
	error("foo.");

	/* This point should not be reached. */
	return EXIT_SUCCESS;
}
