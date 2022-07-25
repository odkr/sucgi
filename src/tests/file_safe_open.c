/*
 * Test file_safe_open.
 */

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../err.h"
#include "../file.h"
#include "utils.h"


int
main (int argc, char **argv)
{
	int fd;
	const char *fname = NULL;
	error rc = ERR;

	switch (argc) {
		case 2:
			fname = argv[1];
			break;
		default:
	 		die("usage: file_safe_open FNAME");
	}

	rc = file_safe_open(fname, O_RDONLY | O_CLOEXEC, &fd);
	if (rc != OK) return EXIT_FAILURE;
	close(fd);

	return EXIT_SUCCESS;
}
