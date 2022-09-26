/*
 * Print the login name associated with a user ID.
 */

#include <errno.h>
#include <pwd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lib.h"


int
main (int argc, char **argv)
{
	struct passwd *pwd = NULL;	/* User record. */
	uid_t uid = 0;			/* UID. */

	if (argc != 2) die("usage: getlogname UID.");
	if (str_to_id(argv[1], &uid) != SC_OK) {
		die("getlogname: cannot parse UID.");
	}

	errno = 0;
	pwd = getpwuid(uid);
	if (!pwd) {
		if (errno == 0) {
			die("getlogname: getpwuid %llu: no such user.",
			    (uint64_t) uid);
		} else {
			die("getlogname: getpwuid %llu: %s.",
			    (uint64_t) uid, strerror(errno));
		}
	}

	/* RATS: ignore */
	(void) puts(pwd->pw_name);

	return EXIT_SUCCESS;
}
