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
#include "../tools/lib.h"


int
main (int argc, char **argv)
{
	struct stat fstatus;
	const char *fname = NULL;

	if (argc != 2) die("usage: file_safe_stat FNAME");

	fname = argv[1];
	if (file_safe_stat(fname, &fstatus) != SC_OK) {
		die("file_safe_stat: open %s: %s.", fname, strerror(errno));
	}

	/* RATS: ignore. */
	(void) printf(
		"uid=%llu gid=%llu mode=%o size=%llu\n",
		(uint64_t) fstatus.st_uid,  (uint64_t) fstatus.st_gid,
			   fstatus.st_mode, (uint64_t) fstatus.st_size
	);

	return EXIT_SUCCESS;
}
