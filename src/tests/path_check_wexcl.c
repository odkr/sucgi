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
#include "str.h"


/*
 * Convert a string to an ID.
 * Aborts the programme if conversion fails.
 */
void
str_to_id(char *s, id_t *id) {
	unsigned long long n;
	if (str_to_ullong(s, &n) != OK) {
		die("path_check_wexcl: %s is not a number.", s);
	}
	*id = (id_t) n;
}

int
main (int argc, char **argv)
{
	const char *fname = NULL;
	const char *stop = NULL;
	uid_t uid = 0;

	if (argc != 4) die("usage: path_check_wexcl UID GID FNAME STOP");
	str_to_id(argv[1], &uid);
	fname = argv[2];
	stop = argv[3];

	return (int) path_check_wexcl(uid, fname, stop);
}
