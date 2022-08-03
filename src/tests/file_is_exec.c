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

	if (argc != 2) die("usage: file_is_exec FNAME");
	fname = argv[1];

	if (stat(fname, &fstatus) != 0) {
		die("file_is_exec: stat %s: %s.", fname, strerror(errno));
	}

	if (file_is_exec(&fstatus)) return EXIT_SUCCESS;
	return EXIT_FAILURE;
}
