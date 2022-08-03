/*
 * Test check.
 */

#include <assert.h>
#include <stdlib.h>

#include "../err.h"

/* check wrapper that should always fail. */
error
check_ok (void)
{
	check(OK);
	/* This point should be reached. */
	return ERR_SYS;
}

/* check wrapper that should always fail. */
error
check_err (void)
{
	check(ERR_SYS);
	/* This point should not be reached. */
	return OK;
}

int
main (void) {
	assert(check_ok() != OK);
	assert(check_err() != OK);
	return EXIT_SUCCESS;
}
