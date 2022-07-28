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
#include <limits.h>
#include <grp.h>
#include <pwd.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/types.h>

#include "err.h"
#include "str.h"
#include "utils.h"


void
drop_privs(const struct passwd *user)
{
	const uid_t uid = user->pw_uid;
	const gid_t gid = user->pw_gid;
	gid_t groups[] = {gid};
	int groups_init = -1;

	if (uid == 0) {
		fail("%s is the superuser.", user->pw_name);
	}
	if (gid == 0) {
		fail("%s's primary group is the supergroup.", user->pw_name);
	}

	if (setgroups(1, groups) != 0) {
		fail("group clean-up: %s.", strerror(errno));
	}
	
#if defined(__APPLE__) && defined(__MACH__)
	groups_init = initgroups(user->pw_name, (int) gid) != 0;
#else
	groups_init = initgroups(user->pw_name, gid) != 0;
#endif /* defined(__APPLE__) && defined(__MACH__) */
	if (groups_init != 0) {
		fail("group initialisation: %s.", strerror(errno));
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
#include <stdio.h>
void
run_script(const char *const script, const struct pair pairs[])
{
	char *suffix = NULL;		/* Filename suffix. */

	assert(script);
	suffix = strrchr(script, '.');
	if (!suffix) fail("%s: no filename suffix.", script);

	for (int i = 0; pairs[i].key; i++) {
		const char *const filetype = pairs[i].key;
		const char *const interpreter = pairs[i].value;

		if (!str_eq(suffix, filetype)) continue;

		if (!interpreter) {
			fail("script handler %d: no interpreter.", i + 1);
		}
		if (interpreter[0] == '\0') {
			fail("script handler %d: path is empty.", i + 1);
		}

		// suCGI's whole point is to do this safely.
		// flawfinder: ignore.
		execlp(interpreter, interpreter, script, NULL);
		fail("%s: exec %s: %s.", script, interpreter, strerror(errno));
	}

	fail("filename suffix %s: no interpreter registered.", suffix);
}
