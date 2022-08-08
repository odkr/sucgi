/*
 * Print the username associated with a user ID.
 */

#include <errno.h>
#include <pwd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../utils.h"
#include "../str.h"

int
main (int argc, char **argv)
{
	struct passwd *pwd = NULL;
	uid_t uid = 0;

	if (argc != 2) die("usage: username UID.");
	if (str_to_id(argv[1], &uid) != OK) die("username: cannot parse UID.");

	errno = 0;
	pwd = getpwuid(uid);
	if (!pwd) {
		if (0 == errno) {
			die("username: getpwuid %llu: no such user.",
			    (uint64_t) uid);
		} else {
			die("username: getpwuid %llu: %s.",
			    (uint64_t) uid, strerror(errno));
		}
	}

	/* RATS: ignore */
	(void) puts(pwd->pw_name);

	return EXIT_SUCCESS;
}
