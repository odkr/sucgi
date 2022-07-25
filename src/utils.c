/*
 * Run scripts for suCGI.
 *
 * Copyright 2022 Odin Kroeger
 * 
 * This file is part of suCGI.
 *
 * suCGI is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * suCGI is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with suCGI. If not, see <https://www.gnu.org/licenses>.
 */

#include <assert.h>
#include <errno.h>
#include <pwd.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

#include "err.h"
#include "str.h"
#include "utils.h"


void
drop_privs(struct passwd *user)
{
	uid_t uid = UID_MAX;
	gid_t gid = GID_MAX;

	assert(user);

	uid = user->pw_uid;
	gid = user->pw_gid;

	{
		const gid_t groups[] = {gid};
		if (setgroups(1, groups) != 0) {
			fail("group clean-up: %s.", strerror(errno));
		}
	}

	{
#if defined(__APPLE__) && defined(__MACH__)
		const int rc = initgroups(user->pw_name, (int) gid) != 0;
#else
		const int rc = initgroups(user->pw_name, gid) != 0;
#endif /* defined(__APPLE__) && defined(__MACH__) */
		if (rc != 0) {
			fail("group initialisation: %s.", strerror(errno));
		}
	}

	/*
	 * The real UID and GID need to be set, too. Or else the
	 * user may call seteuid(2) to gain webserver priviliges. 
	 */
	if (setgid(gid) != 0) {
		fail("failed to set real GID: %s", strerror(errno));
	}
	if (setuid(uid) != 0) {
		fail("failed to set real UID: %s.", strerror(errno));
	}
	if (setegid(gid) != 0) {
		fail("failed to set effective GID: %s", strerror(errno));
	}
	if (seteuid(uid) != 0) {
		fail("failed to set effective UID: %s.", strerror(errno));
	}

	if (getuid() != uid) {
		fail("real UID did not change.");
	}
	if (getgid() != gid) {
		fail("real GID did not change.");
	}
	if (geteuid() != uid) {
		fail("effective UID did not change.");
	}
	if (getegid() != gid) {
		fail("effective GID did not change.");
	}

#if defined(TESTING) && TESTING
	if (setegid(0) == 0) {
		fail("could re-set process' effective GID to 0.");
	}
	if (seteuid(0) == 0) {
		fail("could re-set process' effective UID to 0.");
	}
	if (setgid(0) == 0) {
		fail("could re-set process' real GID to 0.");
	}
	if (setuid(0) == 0) {
		fail("could re-set process' real UID to 0.");
	}
#endif /* defined(TESTING) && TESTING */
}

void
run_script(const char *const script, const char **const pairs)
{
	const char **pair = NULL;	/* Current pair. */
	char *suffix = NULL;		/* Filename suffix. */
	int i = 0;			/* Counter. */

	assert(script);
	assert(pairs);

	suffix = strrchr(script, '.');
	if (!suffix) fail("%s: no filename suffix.", script);

	for (pair = pairs; *pair; pair++) {
		error rc = ERR;		/* Return code. */
		char *inter = NULL;	/* Interpreter .*/
		char *ftype = NULL;	/* Suffix associated with inter. */

		i++;
		rc = str_vsplit(*pair, "=", 2, &ftype, &inter);
		switch (rc) {
			case OK:
				break;
			case ERR_SYS:
				fail("interpreter %d: %s.",
				     i, strerror(errno));
				break;
			default:
				fail("%s:%d: str_vsplit returned %u.",
				      __FILE__, __LINE__ - 10, rc);
		}

		if (ftype[0] == '\0') {
			fail("script type %d: no filename suffix.", i);
		}
		if (ftype[0] != '.' || str_eq(ftype, ".")) {
			fail("script type %d: weird filename suffix.", i);
		} 
		if (!inter || inter[0] == '\0') {
			fail("script type %d: no interpreter given.", i);
		}

		if (str_eq(suffix, ftype)) {
			// suCGI's whole point is to do this safely.
			// flawfinder: ignore.
			execlp(inter, inter, script, NULL);
		};
	}

	fail("filename suffix %s: no interpreter registered.", suffix);
}
