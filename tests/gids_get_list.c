/*
 * Test gids_get_list.
 */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "../err.h"
#include "../gids.h"
#include "../tools/lib.h"


int
main (int argc, char **argv)
{
	const char *logname;		/* Login name. */
	id_t gid;			/* A random GID. */
	gid_t gids[NGROUPS_MAX];	/* List of GIDs. */
	int ngids;			/* Number of GIDs. */
	enum error rc;			/* gids_get_list's return code. */

	if (argc != 3) {
		die("usage: gids_get_list LOGNAME GID");
	} else if (str_to_id(argv[2], &gid) != SC_OK) {
		die("gid_get_list: could not parse GID.");
	}

	logname = argv[1];

	rc = gids_get_list(logname, gid, &gids, &ngids);
	switch (rc) {
		case SC_OK:
			break;
		case SC_ERR_SYS:
			croak("getgrent: %s.", strerror(errno));
		case SC_ERR_GIDS_MAX:
			croak("%s: in too many groups.", logname);
		default:
			croak("unexpected return code %u.", rc);
	}

	for (int i = 0; i < ngids; i++) printf("%llu\n", (uint64_t) gids[i]);

	return EXIT_SUCCESS;
}
