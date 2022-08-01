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
	mode_t ftype = 0;
	char *name = NULL;
	char *fname = NULL;
	char *fspec = NULL;
	error rc;

	switch (argc) {
		case 3:
			name = argv[1];
			fspec = argv[2];
			break;
		default:
	 		die("usage: env_get_fname VAR FTYPE");
	}

	if (fspec[1] != '\0') {
		die("env_get_fname: filetype must be a single character.");
	}

	switch(fspec[0]) {
		case 'f':
			ftype = S_IFREG;
			break;
		case 'd':
			ftype = S_IFDIR;
			break;
		case '\0':
			die("env_get_fname: filetype is empty.");
		default:
			die("env_get_fname: filetype must be 'd' or 'f'.");
	}

	rc = env_get_fname(name, ftype, &fname, &fstatus);

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
		    name, fspec, rc);
	}

	return EXIT_SUCCESS;
}
