/* 
 * Test path_check_wexcl.
 */

#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "../err.h"
#include "../path.h"
#include "utils.h"

void
str_to_id_f (char *s, id_t *id) {
	unsigned long n;
	if (str_to_ulong(s, &n) != OK) {
		die("path_check_wexcl: %d is not a number.", s);
	}
	*id = (id_t) n;
}

int
main (int argc, char **argv)
{
	const char *fname = NULL;
	const char *stop = NULL;
	uid_t uid = 0;
	gid_t gid = 0;

	errno = 0;

	switch (argc) {
		case 5:
			str_to_id_f(argv[1], &uid);
			str_to_id_f(argv[2], &gid);
			fname = argv[3];
			stop = argv[4];
			break;
		default:
			die("usage: path_check_wexcl UID GID FNAME STOP");
			break;
	}

	return (int) path_check_wexcl(uid, gid, fname, stop);
}
