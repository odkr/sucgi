/*
 * Run a programme under the given UID and GID.
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "../str.h"
#include "../utils.h"


/*
 * Convert a string to a user ID.
 * Aborts the programme if conversion fails.
 */
void
str_to_id(char *s, id_t *id) {
	unsigned long n;
	if (str_to_ulong(s, &n) != OK) {
		die("run-as: %s is not a number.", s);
	}
	*id = (id_t) n;
}

int
main (int argc, char **argv)
{
	char **args = calloc((size_t) (argc - 3), sizeof(char *)); 
	uid_t uid = 0;
	gid_t gid = 0;

	if (argc < 4) die("usage: run-as UID GID PROG [ARG [ARG [...]]]");
	if (!args) die("run-as: %s.", strerror(errno));

	str_to_id(argv[1], &uid);
	str_to_id(argv[2], &gid);
	/* cppcheck-suppress nullPointerRedundantCheck;
	   Flawfinder: ignore */
	(void) memcpy(args, &argv[3],
	              /* argc - 2 to copy the terminating NULL. */
	              (size_t) (argc - 2) * sizeof(char *));

	if (setgroups(1, (gid_t[1]) {gid}) != 0) {
		die("setgroups %lu: %s.",
		    (unsigned long) gid, strerror(errno));
	}

	/*
	 * The real UID and GID need to be set, too. Or else the
	 * user may call seteuid(2) to gain webserver priviliges.
	 */
	if (setgid(gid) != 0) {
		die("run-as: setgid %lu: %s.",
		    (unsigned long) gid, strerror(errno));
	}
	if (setuid(uid) != 0) {
		die("run-as: setuid %lu: %s.",
		    (unsigned long) uid, strerror(errno));
	}
	if (setegid(gid) != 0) {
		die("run-as: setegid %lu: %s.",
		    (unsigned long) gid, strerror(errno));
	}
	if (seteuid(uid) != 0) {
		die("run-as: seteuid %lu: %s.",
		    (unsigned long) uid, strerror(errno));
	}

	if (getuid() != uid) die("run-as: failed to set real UID.");
	if (getgid() != gid) die("run-as: failed to set real GID.");
	if (geteuid() != uid) die("run-as: failed to set effective UID.");
	if (getegid() != gid) die("run-as: failed to set effective GID.");

	if (setegid(0) == 0) die("run-as: could reset effective GID to 0.");
	if (seteuid(0) == 0) die("run-as: could reset effective UID to 0.");
	if (setgid(0) == 0) die("run-as: could reset real GID to 0.");
	if (setuid(0) == 0) die("run-as: could reset real UID to 0.");

	// Flawfinder: ignore
	execvp(args[0], args);

	die("run-as: exec %s: %s.", args[0], strerror(errno));
}
