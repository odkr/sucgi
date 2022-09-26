/*
 * Test check.
 */

#include <stdlib.h>

#include "../err.h"
#include "../tools/lib.h"


/* try wrapper that should always fail. */
static enum error
try_ok (void)
{
	try(SC_OK);
	
	return SC_ERR_SYS;
}

/* try wrapper that should always fail. */
static enum error
try_err (void)
{
	try(SC_ERR_SYS);

	/* This point should not be reached. */
	return SC_OK;
}

int
main (void) {
	if (try_ok() != SC_ERR_SYS) die("try: did not ignore SC_OK");
	if (try_err() != SC_ERR_SYS) die("try: ignored error.");

	return EXIT_SUCCESS;
}
