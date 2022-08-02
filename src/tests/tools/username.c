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

/*
 * Convert a string to a user ID.
 * Aborts the programme if conversion fails.
 */
void
str_to_uid(char *s, uid_t *uid) {
	unsigned long n;
	if (str_to_ulong(s, &n) != OK) {
		die("username: %s is not a number.", s);
	}
	*uid = (uid_t) n;
}

int
main (int argc, char **argv)
{
	struct passwd *pwd = NULL;
	uid_t uid = 0;

	if (argc != 2) die("usage: username UID.");
	str_to_uid(argv[1], &uid);

	/* cppcheck-suppress getpwuidCalled */
	pwd = getpwuid(uid);
	if (!pwd) {
		if (errno) {
			die("username: getpwuid %llu: %s.",
			    (uint64_t) uid, strerror(errno));
		} else {
			die("username: getpwuid %llu: no such user.",
			    (uint64_t) uid);
		}
	}

	/* Flawfinder: ignore */
	puts(pwd->pw_name);

	exit(EXIT_SUCCESS);
}
