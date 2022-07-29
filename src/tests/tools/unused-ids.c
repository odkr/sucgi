/*
 * Get an unused user ID and group ID.
 */

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <grp.h>
#include <pwd.h>

#define MAX 65536

int
main (void)
{
	struct passwd *pwd = NULL;
	struct group *grp = NULL;
	uid_t uid = 1;
	gid_t gid = 1;

	for (; uid < MAX; uid++) {
		pwd = getpwuid(uid);
		if (!pwd) {
			if (errno) {
				errno = 0;
			} else {
				break;
			}
		}
	}
	if (MAX == uid) die("unused-ids: cannot find an unused user ID.");

	for (; gid < MAX; gid++) {
		grp = getgrgid(gid);
		if (!grp) {
			if (errno) {
				errno = 0;
			} else {
				break;
			}
		}
	}
	if (MAX == gid) die("unused-ids: cannot find an unused group ID.");

	printf("%lu:%lu\n", (unsigned long) uid, (unsigned long) gid);

	exit(EXIT_SUCCESS);
}
