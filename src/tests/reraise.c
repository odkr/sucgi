/* 
 * Test reraise.
 */

#include <assert.h>
#include <stdlib.h>

#include "../err.h"

/* reraise wrapper that should always fail. */
enum code
reraise_ok (void)
{
	reraise(OK);
	/* This point should be reached. */
	return ERR_SYS;
}

/* reraise wrapper that should always fail. */
enum code
reraise_err (void)
{
	reraise(ERR_SYS);
	/* This point should not be reached. */
	return OK;
}

int
main (void) {
	assert(reraise_ok() != OK);
	assert(reraise_err() != OK);
	return EXIT_SUCCESS;
}
