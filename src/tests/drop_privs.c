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

	/* cppcheck-suppress getpwnamCalled */
	pwd = getpwnam(user);
	if (!pwd) {
		char *err = (errno > 0) ? strerror(errno) : "no such user";
		die("drop_privs: getpwnam %s: %s.", user, err);
	}

	drop_privs(pwd);

	/* cppcheck-suppress invalidPrintfArgType_uint */
	/* Flawfinder: ignore */
	printf("effective: %llu:%llu; real: %llu:%llu.\n",
	       (uint64_t) geteuid(), (uint64_t) getegid(),
	       (uint64_t) getuid(), (uint64_t) getgid());

	return EXIT_SUCCESS;
}