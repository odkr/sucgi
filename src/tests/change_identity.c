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

	/* 
	 * Creating coverage reports as root would fail
	 * if the EUID and EGID were not reset to 0.
	 */

	if (seteuid(0) != 0) {
		die("change_identity: seteuid 0: %s.", strerror(errno));
	}
	if (setegid(0) != 0) {
		die("change_identity: setegid 0: %s.", strerror(errno));
	}

	return EXIT_SUCCESS;
}
