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
change_identity(const struct passwd *const user)
{
	uid_t uid = 0;		/* user's UID. */
	gid_t gid = 0;		/* user's GID. */
	char *name = NULL;	/* user's name. */

	assert(user && user->pw_name);
	name = user->pw_name;
	uid = user->pw_uid;
	gid = user->pw_gid;

	if (0 == uid) fail("%s is the superuser.", name);
	if (0 == gid) fail("%s's primary group is the supergroup.", name);

	if (setgroups(1, (gid_t[1]) {gid}) != 0) {
		fail("setgroups %lu: %s.",
		     (unsigned long) gid, strerror(errno));
	}
	if (initgroups(name, (dummy_gid) gid) != 0) {
		fail("initgroups %s: %s.", name, strerror(errno));
	}

	/*
	 * The real UID and GID need to be set, too. Or else the
	 * user can call seteuid(2) to gain webserver priviliges.
	 */
	if (setgid(gid) != 0) {
		fail("setgid %lu: %s.", (unsigned long) gid, strerror(errno));
	}
	if (setuid(uid) != 0) {
		fail("setuid %lu: %s.", (unsigned long) uid, strerror(errno));
	}
	if (setegid(gid) != 0) {
		fail("setegid %lu: %s.", (unsigned long) gid, strerror(errno));
	}
	if (seteuid(uid) != 0) {
		fail("seteuid %lu: %s.", (unsigned long) uid, strerror(errno));
	}

	if (getuid() != uid) fail("failed to set real UID.");
	if (getgid() != gid) fail("failed to set real GID.");
	if (geteuid() != uid) fail("failed to set effective UID.");
	if (getegid() != gid) fail("failed to set effective GID.");

	if (setegid(0) == 0) fail("could reset effective GID to 0.");
	if (seteuid(0) == 0) fail("could reset effective UID to 0.");
	if (setgid(0) == 0) fail("could reset real GID to 0.");
	if (setuid(0) == 0) fail("could reset real UID to 0.");
}

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
		} else if (str_eq(interpreter, "")) {
			fail("script handler %d: path is empty.", i + 1);
		}

		/* Flawfinder: ignore (suCGI's point is to do this safely). */
		execlp(interpreter, interpreter, script, NULL);
		fail("exec %s %s: %s.", interpreter, script, strerror(errno));
	}

	fail("filename suffix %s: no interpreter registered.", suffix);
}
