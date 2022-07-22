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
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>

#include "config.h"
#include "env.h"
#include "err.h"
#include "file.h"
#include "path.h"
#include "str.h"
#include "utils.h"


/*
 * System
 */

#if PATH_MAX == -1
#define PATH_MAX STR_MAX_LEN + 1
#endif


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

#if MAX_UID < MIN_UID
#error MAX_UID is smaller than MIN_UID.
#endif

#if MIN_UID > MAX_UID
#error MIN_UID is greater than MAX_UID.
#endif

#if !defined(SCRIPT_HANDLERS)
#define SCRIPT_HANDLERS	{	\
	".php=php",		\
	NULL			\
}
#endif

#if !defined(WWW_USER)
#define WWW_USER "www-data"
#endif


/*
 * Test suite
 */

#if TESTING

#undef DOC_ROOT
#define DOC_ROOT "/*"

#undef MIN_UID
#define MIN_UID 500

#undef SCRIPT_HANDLERS
#define SCRIPT_HANDLERS {".sh=sh", NULL}

#endif /* TESTING. */


/*
 * Main
 */

int
main (void) {
	/* passwd database entry of the programme's owner. */
	struct passwd *user = NULL;
	/* Filesystem status of $DOCUMENT_ROOT. */
	struct stat *doc_root_st = NULL;
	/* Filesystem status of $PATH_TRANSLATED, that is, the programme. */
	struct stat *path_trans_st = NULL;
	/* $DOCUMENT_ROOT. */
	char *doc_root = NULL;
	/* $PATH_TRANSLATED. */
	char *path_trans = NULL;
	/* A backup of the environment. */
	char **vars = NULL;
	/* A return code. */
	enum code rc = ERR;

	errno = 0;


	/*
	 * Words of wisdom from the authors of suexec.c:
	 *
	 * > While cleaning the environment, the environment should be clean.
	 * > (E.g. malloc() may get the name of a file for writing debugging
	 * > info. Bad news if MALLOC_DEBUG_FILE is set to /etc/passwd.)
	 */

	rc = env_clear(&vars);
	switch (rc) {
		case OK:
			break;
		case ERR_SYS:
			error("environment clean-up: %s.", strerror(errno));
			break;
		case ERR_ENV_MAX:
			error("too many environment variables.");
			break;
		default:
			error("%s:%d: env_clear returned %d.",
			      __FILE__, __LINE__ - 12, rc);
	}


	/*
	 * Check if run by the webserver.
	 */

	/*
	 * Guard against a system operator leaving suCGI world-executable.
	 * NB: The test suite does not check whether this check works.
	 */
#if !TESTING
	{
		uid_t uid = getuid();
		if (uid != 0) {
			// suCGI does not aim to be thread-safe.
			// cppcheck-suppress getpwuidCalled
			struct passwd *pwd = getpwuid(uid);
			if (!pwd) {
				error("lookup of UID %lu: %s.",
		                      (unsigned long) uid, strerror(errno));
			}
			// It is checked above whether pwd is a null pointer.
			// cppcheck-suppress nullPointerRedundantCheck
			if (!str_eq(pwd->pw_name, WWW_USER)) {
				error("user %s: not permitted to run suCGI.",
				      pwd->pw_name);
			}
		}
	}
#endif /* !TESTING. */


	/*
	 * Re-populate the environment.
	 */

	rc = env_restore((const char *const *) vars, env_keep, env_toss);
	switch (rc) {
		case OK:
			break;
		case ERR_SYS:
			error("environment restoration: %s.",
			      strerror(errno));
			break;
		case ERR_STR_LEN:
			error("environment variable too long.");
			break;
		case ERR_VAR_INVALID:
			error("ill-formed environment variable.");
			break;
		default:
			error("%s:%d: env_restore returned %d.",
			      __FILE__, __LINE__ - 17, rc);
	}

	if (setenv("PATH", SECURE_PATH, 1) != 0) {
		error("setenv PATH: %s", strerror(errno));
	}


	/*
	 * Check whether DOCUMENT_ROOT makes sense.
	 */

	rc = env_get_fname("DOCUMENT_ROOT", &doc_root, &doc_root_st);
	switch (rc) {
		case OK:
			break;
		case ERR_SYS:
			error("$DOCUMENT_ROOT: %s.", strerror(errno));
			break;
		case ERR_STR_LEN:
			error("$DOCUMENT_ROOT: too long.");
			break;
		case ERR_VAR_UNDEF:
			error("DOCUMENT_ROOT: not set.");
			break;
		case ERR_VAR_EMPTY:
			error("DOCUMENT_ROOT: is the empty string.");
			break;
		default:
			error("%s:%d: env_get_fname returned %d.",
			      __FILE__, __LINE__ - 18, rc);
	}

	if (!S_ISDIR(doc_root_st->st_mode)) {
		error("$DOCUMENT_ROOT: not a directory.");
	}

	if (fnmatch(DOC_ROOT, doc_root, FNM_PERIOD) != 0) {
		error("$DOCUMENT_ROOT: does not match %s.", DOC_ROOT);
	}


	/*
	 * Check if PATH_TRANSLATED makes sense.
	 */

	rc = env_get_fname("PATH_TRANSLATED", &path_trans, &path_trans_st);
	switch (rc) {
		case OK:
			break;
		case ERR_SYS:
			error("$PATH_TRANSLATED: %s.", strerror(errno));
			break;
		case ERR_STR_LEN:
			error("$PATH_TRANSLATED: too long.");
			break;
		case ERR_VAR_UNDEF:
			error("PATH_TRANSLATED: not set.");
			break;
		case ERR_VAR_EMPTY:
			error("PATH_TRANSLATED: is the empty string.");
			break;
		default:
			error("%s:%d: path_check_len returned %d.",
			      __FILE__, __LINE__ - 19, rc);
	}

	if (!path_contains(doc_root, path_trans)) {
		error("$PATH_TRANSLATED: not in document root %s.", doc_root);
	}

	if (!S_ISREG(path_trans_st->st_mode)) {
		error("$PATH_TRANSLATED: not a regular file.");
	}


	/*
	 * Verify the CGI programme's UID and GID.
	 */

	if (path_trans_st->st_uid == 0) {
		error("%s: owned by the superuser.", path_trans);
	}
	if (path_trans_st->st_gid == 0) {
		error("%s: owned by the supergroup.", path_trans);
	}

	/* NB: The test suite does not check whether this check works. */
	if (path_trans_st->st_uid < MIN_UID ||
	    path_trans_st->st_uid > MAX_UID)
	{
		error("%s: owned by non-regular UID %lu.",
		      path_trans, (unsigned long) path_trans_st->st_uid);
	}

	/*
	 * There is no need to check the GID that owns the programme.
	 * suCGI sets the process' effective GID to that of the programme
	 * owner's primary group, not to that of the programme file.
	 */

	// suCGI does not aim to be thread-safe.
	// cppcheck-suppress getpwuidCalled
	user = getpwuid(path_trans_st->st_uid);
	/* NB: The test suite does not check whether this check works. */
	if (!user) {
		error("%s: lookup of UID %lu: %s.", path_trans,
		      (unsigned long) path_trans_st->st_uid, strerror(errno));
	}


	/*
	 * Drop privileges.
	 */

	/* NB: This whole section is not checked by the test suite. */
	{
		// It is checked above whether user is a null pointer.
		// cppcheck-suppress nullPointerRedundantCheck
		uid_t uid = user->pw_uid;
		// It is checked above whether user is a null pointer.
		// cppcheck-suppress nullPointerRedundantCheck
		gid_t gid = user->pw_gid;

		const gid_t groups[] = {gid};

		if (setgroups(1, groups) != 0) {
			error("supplementary group clean-up: %s.",
			      strerror(errno));
		}

		/* 
		 * Darwin's initgroups expects the GID to be given as int.
		 * This will trigger a (harmless) compiler warning.
		 */
		if (initgroups(user->pw_name, gid) != 0) {
			error("supplementary group initialisation: %s.",
			      strerror(errno));
		}

		/*
		 * The real UID and GID need to be set, too. Or else the
		 * user may call seteuid(2) to gain webserver priviliges. 
		 */
		if (setgid(gid) != 0) {
			error("failed to set real GID: %s",
			      strerror(errno));
		}
		if (setuid(uid) != 0) {
			error("failed to set real UID: %s.",
			      strerror(errno));
		}
		if (setegid(gid) != 0) {
			error("failed to set effective GID: %s",
			      strerror(errno));
		}
		if (seteuid(uid) != 0) {
			error("failed to set effective UID: %s.",
			      strerror(errno));
		}

		if (getuid() != uid) {
			error("real UID did not change to %d.", uid);
		}
		if (getgid() != gid) {
			error("real GID did not change to %d.", gid);
		}
		if (geteuid() != uid) {
			error("effective UID did not change to %d.", uid);
		}
		if (getegid() != gid) {
			error("effective GID did not change to %d.", gid);
		}

#if !TESTING
		if (setegid(0) == 0) {
			error("could re-set process' effective GID to 0.");
		}
		if (seteuid(0) == 0) {
			error("could re-set process' effective UID to 0.");
		}
		if (setgid(0) == 0) {
			error("could re-set process' real GID to 0.");
		}
		if (setuid(0) == 0) {
			error("could re-set process' real UID to 0.");
		}
#endif /* !TESTING. */
	}


	/*
	 * Guard against system operators making a mistake like:
	 *
	 *	cd /home/jsmith/public_html/wp-plugins
	 *	cp -a /home/doe/public_html/wp-plugins/acme ./acme
	 *	chown -R smith:smith acme
	 */
	if (!path_contains(user->pw_dir, doc_root)) {
		error("document root %s is not in %s's home directory.",
		      doc_root, user->pw_name);
	}


	/*
	 * Guard against permission errors of various sorts.
	 */

	rc = path_check_wexcl(user->pw_uid, user->pw_gid,
	                      path_trans, user->pw_dir);
	switch (rc) {
		case OK:
			break;
		case ERR_SYS:
			error("%s: %s.", path_trans, strerror(errno));
			break;
		case ERR_NOT_EXCLW:
		        error("%s: can be altered by users other than %s.",
			      path_trans, user->pw_name);
			break;
		default:
			error("%s:%d: path_check_wexcl returned %d.",
			      __FILE__, __LINE__ - 13, rc);
	}


	/*
	 * Run the programme.
	 */

	if (file_is_exec(path_trans_st)) {
		const char *handlers[] = SCRIPT_HANDLERS;
		/* run_script only returns if prog could not be executed. */
		run_script(path_trans, handlers);
	} else {
		// suCGI's whole point is to do this safely.
		// flawfinder: ignore.
		execl(path_trans, path_trans, NULL);
	}

	/* If this point is reached, execution has failed. */
	error("failed to execute %s: %s.", path_trans, strerror(errno));
}
