/*
 * Test priv_drop.
 */

#include <errno.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../err.h"
#include "../priv.h"
#include "../tools/lib.h"

int
main (int argc, char **argv)
{
	const char *logname;	/* Login name. */
	struct passwd *user;	/* passwd entry of the given user. */
	enum error rc;		/* A return code. */

	if (argc != 2) die("usage: priv_drop LOGNAME");
	
	logname = argv[1];
	errno = 0;
	user = getpwnam(logname);
	if (user == NULL) {
		croak("getpwnam %s: %s.", logname,
                      (errno == 0) ? "no such user" : strerror(errno));
	}

	rc = priv_drop(user->pw_uid, user->pw_gid, 1,
	               (gid_t[1]) {user->pw_gid});
	switch (rc) {
		case OK:
			break;
		case ERR_SYS:
			croak("failed to set IDs or groups: %s.",
			     strerror(errno));
		case ERR_PRIV:
			croak("could resume privileges.");
		default:
			croak("unexpected return code %u.", rc);
	}

	/* RATS: ignore */
	(void) printf("euid=%llu egid=%llu ruid=%llu rgid=%llu\n",
	              (unsigned long long) geteuid(),
	              (unsigned long long) getegid(),
	              (unsigned long long) getuid(),
	              (unsigned long long) getgid());

	return EXIT_SUCCESS;
}
