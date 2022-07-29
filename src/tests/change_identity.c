/*
 * Test run_script.
 */

#include <errno.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../utils.h"
#include "utils.h"

int
main (int argc, char **argv)
{
	struct passwd *pwd = NULL;
	const char *user = NULL;

	switch (argc) {
		case 2:
			user = argv[1];
			break;
		default:
			die("usage: change_identity USERNAME");
	}

	/* cppcheck-suppress getpwnamCalled */
	pwd = getpwnam(user);
	if (!pwd) {
		die("change_identity: getpwnam %s: %s.",
		    user, strerror(errno));
	}

	change_identity(pwd);

	/* flawfinder: ignore */
	printf("effective: %lu:%lu; real: %lu:%lu.\n",
	       (unsigned long) geteuid(), (unsigned long) getegid(),
	       (unsigned long) getuid(), (unsigned long) getgid());

	return EXIT_SUCCESS;
}