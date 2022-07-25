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
			die("usage: drop_privs USER");
	}

	// cppcheck-suppress getpwnamCalled
	pwd = getpwnam(user);
	if (!pwd) {
		die("drop_privs: lookup of user %s: %s.",
		    user, strerror(errno));
	}

	drop_privs(pwd);

	// flawfinder: ignore
	printf("effective: %lu:%lu; real: %lu:%lu.\n", 
	       (unsigned long) geteuid(), (unsigned long) getegid(),
	       (unsigned long) getuid(), (unsigned long) getgid());

	return EXIT_SUCCESS;
}
