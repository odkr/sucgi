/*
 * Test env_get_fname.
 */

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "../env.h"
#include "../err.h"
#include "utils.h"


int
main (int argc, char **argv)
{
	struct stat fstatus;
	/* Flawfinder: ignore */
	char fname[STR_MAX] = "";
	char *ftype = NULL;
	char *var = NULL;
	mode_t fmode = 0;
	error rc;

	if (argc != 3) die("usage: env_get_fname VAR FTYPE");
	var = argv[1];
	ftype = argv[2];

	if (strnlen(ftype, STR_MAX) > 1) {
		die("env_get_fname: filetype must be a single character.");
	}

	switch(ftype[0]) {
		case 'f':
			fmode = S_IFREG;
			break;
		case 'd':
			fmode = S_IFDIR;
			break;
		case '\0':
			die("env_get_fname: filetype is empty.");
		default:
			die("env_get_fname: filetype must be 'd' or 'f'.");
	}

	rc = env_get_fname(var, fmode, &fname, &fstatus);

	if (OK == rc) {
		/* Flawfinder: ignore. */
		printf("file_safe_stat: "
		       "inode %lu, UID %lu, GID %lu, mode %o, size %lub.\n",
		       (unsigned long) fstatus.st_ino,
		       (unsigned long) fstatus.st_uid,
		       (unsigned long) fstatus.st_gid,
		       fstatus.st_mode,
		       (unsigned long) fstatus.st_size);
	} else {
		die("env_get_fname: env_get_fname %s %s returned %d.",
		    var, ftype, rc);
	}

	return EXIT_SUCCESS;
}
