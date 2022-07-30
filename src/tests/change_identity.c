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
		char *err = (errno > 0) ? strerror(errno) : "no such user";
		die("change_identity: getpwnam %s: %s.", user, err);
	}

	change_identity(pwd);

	/* flawfinder: ignore */
	printf("effective: %lu:%lu; real: %lu:%lu.\n",
	       (unsigned long) geteuid(), (unsigned long) getegid(),
	       (unsigned long) getuid(), (unsigned long) getgid());

	/*  FIMXME: bad doc:
	 * If we don't switch back to root, testing fails when coverage
	 * results are generated as root
	 */
	pwd = getpwuid(0);
	if (!pwd) {
		char *err = (errno > 0) ? strerror(errno) : "no such user";
		die("change_identity: getpwuid 0: %s.", err);
	}

	return EXIT_SUCCESS;
}
