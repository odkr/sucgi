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
	char **args = calloc((size_t) (argc - 2), sizeof(char *)); 
	uid_t uid = 0;
	gid_t gid = 0;
	gid_t groups[1] = {0};

	if (argc < 4) die("usage: run-as UID GID PROG [ARG [ARG [...]]]");
	if (!args) die("run-as: %s.", strerror(errno));

	str_to_id(argv[1], &uid);
	str_to_id(argv[2], &gid);
	(void) memcpy(args, &argv[2], (size_t) (argc - 2) * sizeof(char *));

	if (0 == uid) die("run-as: UID is 0.");
	if (0 == gid) die("run-as: GID is 0.");
	
	if (setgroups(1, groups) != 0) {
		die("run-as: group clean-up: %s.", strerror(errno));
	}
	if (setgid(gid) != 0) {
		die("run-as: failed to set real GID: %s", strerror(errno));
	}
	if (setuid(uid) != 0) {
		die("run-as: failed to set real UID: %s.", strerror(errno));
	}
	if (setegid(gid) != 0) {
		die("run-as: failed to set effective GID: %s",
		    strerror(errno));
	}
	if (seteuid(uid) != 0) {
		die("run-as: failed to set effective UID: %s.",
		    strerror(errno));
	}

	if (getuid() != uid) {
		die("run-as: real UID did not change.");
	}
	if (getgid() != gid) {
		die("run-as: real GID did not change.");
	}
	if (geteuid() != uid) {
		die("run-as: effective UID did not change.");
	}
	if (getegid() != gid) {
		die("run-as: effective GID did not change.");
	}

	if (0 == setegid(0)) {
		die("run-as: could re-set process' effective GID to 0.");
	}
	if (0 == seteuid(0)) {
		die("run-as: could re-set process' effective UID to 0.");
	}
	if (0 == setgid(0)) {
		die("run-as: could re-set process' real GID to 0.");
	}
	if (0 == setuid(0)) {
		die("run-as: could re-set process' real UID to 0.");
	}

	execvp(args[0], args);

	die("exec %s: %s.", args[0], strerror(errno));
}
