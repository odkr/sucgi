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

	/* cppcheck-suppress invalidPrintfArgType_uint */
	/* Flawfinder: ignore */
	printf("effective: %llu:%llu; real: %llu:%llu.\n",
	       (uint64_t) geteuid(), (uint64_t) getegid(),
	       (uint64_t) getuid(), (uint64_t) getgid());

	return EXIT_SUCCESS;
}
