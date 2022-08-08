/*
 * Test file_safe_stat.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "../err.h"
#include "../file.h"
#include "utils.h"


int
main (int argc, char **argv)
{
	struct stat fstatus;
	const char *fname = NULL;

	if (argc != 2) die("usage: file_safe_stat FNAME");
	fname = argv[1];

	if (file_safe_stat(fname, &fstatus) == OK) {
		/* Flawfinder: ignore. */
		(void) printf(
			"file_safe_stat: "
			"inode %lu, UID %lu, GID %lu, mode %o, size %lub.\n",
			(unsigned long) fstatus.st_ino,
			(unsigned long) fstatus.st_uid,
			(unsigned long) fstatus.st_gid,
			                fstatus.st_mode,
			(unsigned long) fstatus.st_size
		);
	} else {
		die("file_safe_stat: %s: %s.", fname, strerror(errno));
	}

	return EXIT_SUCCESS;
}
