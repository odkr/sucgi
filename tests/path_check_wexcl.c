/*
 * Test path_check_wexcl.
 */

#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "../err.h"
#include "../path.h"

#include "../tools/lib.h"


int
main (int argc, char **argv)
{
	/* RATS: ignore */
	char cur[STR_MAX];	/* Current directory. */
	const char *parent;	/* Parent directory. */
	const char *fname;	/* Filename. */
	uid_t uid;		/* User ID. */
	enum error rc;		/* Return code. */

	if (argc != 4) {
		die("usage: path_check_wexcl UID SUPER SUB");
	}
	if (str_to_id(argv[1], &uid) != OK) {
		die("path_check_wexcl: could not parse UID.");
	}
	
	parent = argv[2];
	fname = argv[3];

	rc = path_check_wexcl(uid, parent, fname, &cur);
	switch (rc) {
		case OK:
			break;
		case ERR_SYS:
			croak("open %s: %s.", cur, strerror(errno));
		case ERR_PATH_WEXCL:
		        croak("%s: writable by UIDs other than %llu.",
			      cur, (uint64_t) uid);
		default:
			croak("unexpected return code %u.", rc);
	}

	(void) puts(cur);

	return EXIT_SUCCESS;
}
