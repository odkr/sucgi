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


/*
 * Convert a string to an ID.
 * Aborts the programme if conversion fails.
 */
void
str_to_id(char *s, id_t *id) {
	unsigned long long n;
	if (str_to_ullong(s, &n) != OK) {
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

	if (argc != 3) die("usage: file_is_wexcl UID GID FNAME");
	str_to_id(argv[1], &uid);
	fname = argv[2];

	if (stat(fname, &fstatus) != 0) {
		die("file_is_wexcl: stat %s: %s.", fname, strerror(errno));
	}

	if (file_is_wexcl(uid, &fstatus)) return EXIT_SUCCESS;
	return EXIT_FAILURE;
}
