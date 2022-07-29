/*
 * Run CGI programmes under the UID and GID of their owner.
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

#include <errno.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "env.h"
#include "err.h"
#include "file.h"
#include "path.h"
#include "str.h"
#include "utils.h"
#include "config.h"


/*
 * Configuration
 */

#if !defined(DOC_ROOT)
#define DOC_ROOT "/home/*/public_html"
#endif

#if !defined(SECURE_PATH)
#define SECURE_PATH "/usr/bin:/bin"
#endif

#if !defined(MIN_UID)
#define MIN_UID 1000
#elif MIN_UID < 0 || (defined(UID_MIN) && MIN_UID < UID_MIN)
#error MIN_UID is too small.
#endif

#if !defined(MAX_UID)
#define MAX_UID 30000
#elif (defined(UID_MAX) && MIN_UID > UID_MAX)
#error MAX_UID is too large.
#endif

#if MAX_UID <= MIN_UID
/* flawfinder: ignore (this is not a function ¯\_(ツ)_/¯). */
#error MAX_UID is smaller than or equal to MIN_UID.
#endif

#if !defined(SCRIPT_HANDLERS)
#define SCRIPT_HANDLERS {{.key = ".php", .value = "php"}, {NULL, NULL}}
#endif


/*
 * Test suite
 */

#if defined(TESTING) && TESTING

#undef DOC_ROOT
#define DOC_ROOT "/*"

#undef FNM_PATHNAME
#define FNM_PATHNAME 0

#undef MIN_UID
#define MIN_UID 500

#undef SCRIPT_HANDLERS
#define SCRIPT_HANDLERS {{.key = ".sh", .value = "sh"}, {NULL, NULL}}

#endif /* !defined(TESTING) && TESTING */


/*
 * Main
 */

int
main (void) {
	struct passwd *owner = NULL;	/* Programme owner. */
	struct stat fstatus;		/* Programme's filesystem status. */
	char *doc_root = NULL;		/* $DOCUMENT_ROOT. */
	char *prog = NULL;		/* $PATH_TRANSLATED. */
	error rc = ERR;			/* A return code. */


	/*
	 * Secure the environment
	 */

	rc = env_sanitise(env_keep, env_toss);
	switch (rc) {
		case OK:
			break;
		case ERR_SYS:
			fail("environment clean-up: %s.", strerror(errno));
		case ERR_ENV_MAX:
			fail("too many environment variables.");
		case ERR_VAR_INVALID:
			fail("ill-formed environment variable.");
		default:
			fail("%s:%d: env_sanitise returned %u.",
			     __FILE__, __LINE__ - 12, rc);
	}

	if (setenv("PATH", SECURE_PATH, 1) != 0) {
		fail("setenv PATH: %s", strerror(errno));
	}


	/*
	 * Check whether DOCUMENT_ROOT makes sense.
	 */

	rc = env_get_fname("DOCUMENT_ROOT", S_IFDIR, &doc_root, NULL);
	switch (rc) {
		case OK:
			break;
		case ERR_SYS:
			fail("$DOCUMENT_ROOT: %s.", strerror(errno));
		case ERR_FNAME_LEN:
			fail("$DOCUMENT_ROOT: filename too long.");
		case ERR_FTYPE:
			fail("$DOCUMENT_ROOT: not a directory.");
		case ERR_STR_LEN:
			fail("$DOCUMENT_ROOT: path too long.");
		case ERR_VAR_UNDEF:
			fail("DOCUMENT_ROOT: not set.");
		case ERR_VAR_EMPTY:
			fail("DOCUMENT_ROOT: is the empty string.");
		default:
			fail("%s:%d: env_get_fname returned %u.",
			      __FILE__, __LINE__ - 18, rc);
	}

	if (fnmatch(DOC_ROOT, doc_root, FNM_PATHNAME | FNM_PERIOD) != 0) {
		fail("$DOCUMENT_ROOT: does not match %s.", DOC_ROOT);
	}


	/*
	 * Check if PATH_TRANSLATED makes sense.
	 */

	rc = env_get_fname("PATH_TRANSLATED", S_IFREG, &prog, &fstatus);
	switch (rc) {
		case OK:
			break;
		case ERR_SYS:
			fail("$PATH_TRANSLATED: %s.", strerror(errno));
		case ERR_FNAME_LEN:
			fail("$PATH_TRANSLATED: filename too long.");
		case ERR_FTYPE:
			fail("$PATH_TRANSLATED: not a regular file.");
		case ERR_STR_LEN:
			fail("$PATH_TRANSLATED: path too long.");
		case ERR_VAR_UNDEF:
			fail("PATH_TRANSLATED: not set.");
		case ERR_VAR_EMPTY:
			fail("PATH_TRANSLATED: is the empty string.");
		default:
			fail("%s:%d: env_get_fname returned %u.",
			      __FILE__, __LINE__ - 19, rc);
	}

	if (!path_contains(doc_root, prog)) {
		fail("$PATH_TRANSLATED: not in document root %s.", doc_root);
	}

	if (fstatus.st_uid == 0) {
		fail("%s: owned by the superuser.", prog);
	}
	if (fstatus.st_gid == 0) {
		fail("%s: owned by the supergroup.", prog);
	}

	/* NB: The test suite does not check whether this check works. */
	if (MAX_UID < fstatus.st_uid || fstatus.st_uid < MIN_UID) {
		fail("%s: owned by non-regular UID %lu.",
		     prog, (unsigned long) fstatus.st_uid);
	}

	/*
	 * There is no need to check the GID that owns the programme.
	 * suCGI sets the process' effective GID to that of the programme
	 * owner's primary group, not to that of the programme file.
	 */

	/* suCGI does not aim to be thread-safe. */
	/* cppcheck-suppress getpwuidCalled */
	owner = getpwuid(fstatus.st_uid);
	/* NB: The test suite does not check whether this check works. */
	if (!owner) {
		fail("%s: getpwuid %lu: %s.", prog,
		     (unsigned long) fstatus.st_uid, strerror(errno));
	}


	/*
	 * Drop privileges.
	 */

	change_identity(owner);


	/*
	 * Guard against system operators making a mistake like:
	 *
	 *	cd /home/jsmith/public_html/wp-plugins
	 *	cp -a /home/doe/public_html/wp-plugins/acme ./acme
	 *	chown -R smith:smith acme
	 */

	/* It is tested above whether owner is NULL. */
	/* cppcheck-suppress nullPointerRedundantCheck */
	if (!path_contains(owner->pw_dir, doc_root)) {
		fail("document root %s is not in %s's home directory.",
		     doc_root, owner->pw_name);
	}


	/*
	 * If the webserver is (mis-)configured to allow visitors to change
	 * user files and a user (accidentally) has set the permissions of a
	 * web-accessible script to be world-writable, then visitors could
	 * run arbitrary code as that user.
	 */

	rc = path_check_wexcl(owner->pw_uid, prog, owner->pw_dir);
	switch (rc) {
		case OK:
			break;
		case ERR_SYS:
			fail("%s: %s.", prog, strerror(errno));
		case ERR_NOT_EXCLW:
		        fail("%s: can be altered by users other than %s.",
			     prog, owner->pw_name);
		default:
			fail("%s:%d: path_check_wexcl returned %u.",
			     __FILE__, __LINE__ - 11, rc);
	}


	/*
	 * Run the programme.
	 */

	if (!file_is_exec(&fstatus)) {
		/* run_script never returns. */
		run_script(prog, (struct pair []) SCRIPT_HANDLERS);
	} else {
		/* flawfinder: ignore (suCGI's point is to do this safely). */
		execl(prog, prog, NULL);
	}

	/* If this point is reached, execution has failed. */
	fail("exec %s: %s.", prog, strerror(errno));
}
