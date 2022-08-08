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

	if (argc != 2) die("usage: drop_privs USERNAME");
	user = argv[1];

	errno = 0;
	pwd = getpwnam(user);
	if (!pwd) {
		char *err = (0 == errno) ? "no such user" : strerror(errno);
		die("drop_privs: getpwnam %s: %s.", user, err);
	}

	drop_privs(pwd);

	/* RATS: ignore */
	(void) printf("effective: %lu:%lu; real: %lu:%lu.\n",
	              (unsigned long) geteuid(), (unsigned long) getegid(),
	              (unsigned long) getuid(),  (unsigned long) getgid());

	return EXIT_SUCCESS;
}
