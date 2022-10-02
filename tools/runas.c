/*
 * Run a programme under the given UID and GID.
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "lib.h"

#define ARGS_MAX 255

int
main (int argc, char **argv)
{
	uid_t uid = 0;
	gid_t gid = 0;

	if (argc < 4) die("usage: runas UID GID PROG [ARG [ARG [...]]]");
	if (argc > ARGS_MAX) die("runas: too many operands.");
	if (str_to_id(argv[1], &uid) != OK) die("runas: cannot parse UID.");
	if (str_to_id(argv[2], &gid) != OK) die("runas: cannot parse GID.");

	if (setgroups(1, (gid_t[1]) {gid}) != 0) {
		die("runas: setgroups %llu: %s.",
		    (uint64_t) gid, strerror(errno));
	}
	if (setgid(gid) != 0) {
		die("runas: setgid %llu: %s.",
		    (uint64_t) gid, strerror(errno));
	}
	if (setuid(uid) != 0) {
		die("runas: setuid %llu: %s.",
		    (uint64_t) uid, strerror(errno));
	}

	/* RATS: ignore */
	(void) execvp(argv[3], &argv[3]);

	die("runas: exec %s: %s.", argv[3], strerror(errno));
}
