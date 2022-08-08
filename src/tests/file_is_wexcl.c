/*
 * Test file_is_wexcl.
 */

#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "../err.h"
#include "../file.h"

#include "utils.h"
#include "str.h"


int
main (int argc, char **argv)
{
	struct stat fstatus;
	const char *fname = NULL;
	id_t uid = 0;

	if (argc != 3) {
		die("usage: file_is_wexcl UID GID FNAME");
	}
	if (str_to_id(argv[1], &uid) != OK) {
		die("file_check_wexcl: cannot parse UID.");
	}
	fname = argv[2];

	/* RATS: ignore */
	if (stat(fname, &fstatus) != 0) {
		die("file_is_wexcl: stat %s: %s.", fname, strerror(errno));
	}

	if (file_is_wexcl(uid, &fstatus)) return EXIT_SUCCESS;
	return EXIT_FAILURE;
}
