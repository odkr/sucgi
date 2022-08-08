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

#define MAX_ARGS 255U

int
main (int argc, char **argv)
{
	char **args = calloc(MAX_ARGS, sizeof(char *)); 
	uid_t uid = 0;
	gid_t gid = 0;

	if (!args) die("runas: %s.", strerror(errno));
	if (argc < 4) die("usage: runas UID GID PROG [ARG [ARG [...]]]");
	if ((unsigned long) argc > MAX_ARGS) die("runas: too many operands.");
	if (str_to_id(argv[1], &uid) != OK) die("runas: cannot parse UID.");
	if (str_to_id(argv[2], &gid) != OK) die("runas: cannot parse GID.");

	/* RATS: ignore */
	(void) memcpy(args, &argv[3],
	              /* argc - 2 to copy the terminating NULL. */
		      (size_t) (argc - 2) * sizeof(char *));

	if (setgroups(1, (gid_t[1]) {gid}) != 0) {
		die("runas: setgroups %llu: %s.",
		    (uint64_t) gid, strerror(errno));
	}

	/* This is paranoid, but better be safe than sorry. */
	if (setgid(gid) != 0) {
		die("runas: setgid %llu: %s.",
		    (uint64_t) gid, strerror(errno));
	}
	if (setuid(uid) != 0) {
		die("runas: setuid %llu: %s.",
		    (uint64_t) uid, strerror(errno));
	}

	if (setgid(0) != -1) die("runas: setgid 0: succeeded.");
	if (setuid(0) != -1) die("runas: setuid 0: succeeded.");

	/* RATS: ignore */
	(void) execvp(args[0], args);

	die("runas: exec %s: %s.", args[0], strerror(errno));
}
