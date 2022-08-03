/*
 * Find an unallocated regular user and an unallocated regular group ID.
 */

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <grp.h>
#include <pwd.h>

#include "../utils.h"

#define MIN 1000
#define MAX 30000


int
main (void)
{
	struct passwd *pwd = NULL;
	struct group *grp = NULL;
	uid_t uid = MIN;
	gid_t gid = MIN;

	for (; uid < MAX; uid++) {
		/* cppcheck-suppress getpwuidCalled */
		pwd = getpwuid(uid);
		if (!pwd) {
			if (errno) {
				errno = 0;
			} else {
				break;
			}
		}
	}
	if (MAX == uid) die("unallocids: cannot find an unused user ID.");

	for (; gid < MAX; gid++) {
		/* cppcheck-suppress getgrgidCalled */
		grp = getgrgid(gid);
		if (!grp) {
			if (errno) {
				errno = 0;
			} else {
				break;
			}
		}
	}
	if (MAX == gid) die("unallocids: cannot find an unused group ID.");

	// Flawfinder: ignore
	printf("%lu:%lu\n", (unsigned long) uid, (unsigned long) gid);

	exit(EXIT_SUCCESS);
}
