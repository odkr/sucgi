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


void
str_to_id_f (char *s, id_t *id) {
	unsigned long n;
	if (str_to_ulong(s, &n) != OK) {
		die("file_is_wexcl: %s is not a number.", s);
	}
	*id = (id_t) n;
}

int
main (int argc, char **argv)
{
	struct stat fstatus;
	const char *fname = NULL;
	id_t uid = 0;
	id_t gid = 0;

	errno = 0;

	switch (argc) {
		case 4:
			str_to_id_f(argv[1], &uid);
			str_to_id_f(argv[2], &gid);
			fname = argv[3];
			break;
		default:
			die("usage: file_is_wexcl UID GID FNAME");
			break;
	}

	if (stat(fname, &fstatus) != 0) {
		die("file_is_wexcl: stat %s: %s.", fname, strerror(errno));
	}

	if (file_is_wexcl(uid, gid, &fstatus)) return EXIT_SUCCESS;
	return EXIT_FAILURE;
}
