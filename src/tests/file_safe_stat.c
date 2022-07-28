/*
 * Test file_safe_stat.
 */

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#include "../err.h"
#include "../file.h"
#include "utils.h"


int
main (int argc, char **argv)
{
	const char *fname = NULL;

	switch (argc) {
		case 2:
			fname = argv[1];
			break;
		default:
	 		die("usage: file_safe_stat FNAME");
	}

	return (int) file_safe_stat(fname, NULL);
	/* FIXME: test actually get stats! */
}
