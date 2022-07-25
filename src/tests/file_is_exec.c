/*
 * Test file_is_exec.
 */

#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include "../file.h"
#include "utils.h"

int
main (int argc, char **argv)
{
	struct stat fstatus;
	const char *fname = NULL;

	switch (argc) {
		case 2:
			fname = argv[1];
			break;
		default:
	 		die("usage: file_is_exec FNAME");
	}

	if (stat(fname, &fstatus) != 0) {
		die("file_is_exec: stat %s: %s.", fname, strerror(errno));
	}

	if (file_is_exec(&fstatus)) return EXIT_SUCCESS;
	return EXIT_FAILURE;
}
