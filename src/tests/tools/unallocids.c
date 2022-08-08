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
		errno = 0;
		pwd = getpwuid(uid);
		if (!pwd) {
			if (0 == errno) {
				break;
			} else {
				errno = 0;
			}
		}
	}
	if (MAX == uid) die("unallocids: cannot find an unused user ID.");

	for (; gid < MAX; gid++) {
		errno = 0;
		grp = getgrgid(gid);
		if (!grp) {
			if (0 == errno) {
				break;
			} else {
				errno = 0;
			}
		}
	}
	if (MAX == gid) die("unallocids: cannot find an unused group ID.");

	// RATS: ignore
	(void) printf("%lu:%lu\n", (unsigned long) uid, (unsigned long) gid);

	return EXIT_SUCCESS;
}
