/*
 * Run CGI scripts with the permissions of their owner.
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

#define _ISOC99_SOURCE
#define _POSIX_C_SOURCE 200809L

#if !defined(_FORTIFY_SOURCE)
#define _FORTIFY_SOURCE 3
#endif /* !defined(_FORTIFY_SOURCE) */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "config.h"
#include "macros.h"
#include "env.h"
#include "error.h"
#include "file.h"
#include "gids.h"
#include "path.h"
#include "priv.h"
#include "scpt.h"
#include "str.h"


/*
 * Main
 */

int
main(void) {
	/*
	 * Check whether configuration strings are within bounds.
	 */

	BUILD_BUG_ON(sizeof(JAIL_DIR) <= 1);
	BUILD_BUG_ON(sizeof(JAIL_DIR) >= MAX_STR);
	BUILD_BUG_ON(sizeof(USER_DIR) <= 1);
	BUILD_BUG_ON(sizeof(USER_DIR) >= MAX_STR);
	BUILD_BUG_ON(sizeof(PATH) <= 1);
	BUILD_BUG_ON(sizeof(PATH) >= MAX_STR);

	
	/*
	 * Words of wisdom from the authors of suEXEC:
	 *
	 * > While cleaning the environment, the environment should be clean.
	 * > (E.g. malloc() may get the name of a file for writing debugging
	 * > info. Bad news if MALLOC_DEBUG_FILE is set to /etc/passwd.)
	 */

	/* RATS: ignore; env_clear respects MAX_ENV. */
	const char *vars[MAX_ENV];	/* Backup of environ(2). */
	enum error rc;			/* Return code. */

	rc = env_clear(&vars);
	switch (rc) {
		case OK:
			break;
		case ERR_LEN:
			error("too many environment variables.");
		default:
			error("env_clear returned %u.", rc);
	}
	
	assert(!*environ);


	/*
	 * Drop privileges temporarily.
	 */

	uid_t proc_uid;		/* Process' real user ID. */
	gid_t proc_gid;		/* Process' real group ID. */

	proc_uid = getuid();
	proc_gid = getgid();

	errno = 0;

	if (setegid(proc_gid) != 0) {
		error("setegid %llu: %m.", (long long unsigned) proc_gid);
	}

	if (seteuid(proc_uid) != 0) {
		error("seteuid %llu: %m.", (long long unsigned) proc_uid);
	}

	assert(geteuid() == proc_uid);
	assert(getegid() == proc_gid);


	/*
	 * Restore the environment variables needed by CGI scripts.
	 */

	rc = env_restore(vars, env_vars_safe);
	switch (rc) {
		case OK:
			break;
		case ERR_SETENV:
			error("setenv: %m.");
		case ERR_ILL:
			error("found malformed environment variable.");
		case ERR_LEN:
			error("found too long environment variable.");
		default:
			error("env_restore returned %u.", rc);
	}


	/*
	 * Get the document root.
	 */

	const char *jail_dir;		/* Jail directory. */
	const char *doc_root;		/* Document root. */
	int doc_fd;			/* -- " -- file descriptor. */

	errno = 0;
        /* RATS: ignore; this use of realpath should be safe. */
	jail_dir = realpath(JAIL_DIR, NULL);
	if (!jail_dir) {
		error("realpath %s: %m.", JAIL_DIR);
	}

	assert(jail_dir);
	assert(*jail_dir != '\0');
        assert(strnlen(jail_dir, MAX_STR) < MAX_STR);

	rc = env_file_open(jail_dir, "DOCUMENT_ROOT",
	                   O_RDONLY | O_CLOEXEC | O_DIRECTORY,
			   &doc_root, &doc_fd);
	switch (rc) {
		case OK:
			break;
		case ERR_GETENV:
			error("getenv DOCUMENT_ROOT: %m.");
		case ERR_REALPATH:
			error("realpath %s: %m.", doc_root);
		case ERR_OPEN:
			error("open %s: %m.", doc_root);
		case ERR_LEN:
			error("document root path too long.");
		case ERR_ILL:
			error("document root %s not within jail.", doc_root);
		case ERR_NIL:
			error("$DOCUMENT_ROOT unset or empty.");
		default:
			error("env_file_open returned %u.", rc);
	}

	if (close(doc_fd) != 0) {
		error("close %s: %m.", doc_root);
	}

	assert(doc_root);
	assert(*doc_root != '\0');
	assert(doc_fd > -1);
	assert(strnlen(doc_root, MAX_STR) < MAX_STR);
	/* RATS: ignore; only tests whether doc_root exists. */
	assert(access(doc_root, F_OK) == 0);
        /* RATS: ignore; this use of realpath should be safe. */
	assert(strncmp(realpath(doc_root, NULL), doc_root, MAX_STR) == 0);


	/*
	 * Get the script.
	 */

	const char *script;		/* Path to script. */
	int script_fd;			/* -- " -- file descriptor. */
	struct stat script_stat;	/* -- " -- filesystem metadata. */

	rc = env_file_open(doc_root, "PATH_TRANSLATED", O_RDONLY | O_CLOEXEC,
			   &script, &script_fd);
   	switch (rc) {
   		case OK:
   			break;
   		case ERR_GETENV:
   			error("getenv PATH_TRANSLATED: %m.");
   		case ERR_REALPATH:
   			error("realpath %s: %m.", script);
   		case ERR_OPEN:
   			error("open %s: %m.", script);
   		case ERR_LEN:
   			error("script path too long.");
   		case ERR_ILL:
   			error("script %s not within document root.", script);
   		case ERR_NIL:
   			error("$PATH_TRANSLATED unset or empty.");
   		default:
   			error("env_file_open returned %u.", rc);
   	}

	assert(script);
	assert(*script != '\0');
	assert(strnlen(script, MAX_STR) < MAX_STR);
	/* RATS: ignore; only tests whether doc_root exists. */
	assert(access(doc_root, F_OK) == 0);
        /* RATS: ignore; this use of realpath should be safe. */
	assert(strncmp(realpath(script, NULL), script, MAX_STR) == 0);

	errno = 0;
	if (fstat(script_fd, &script_stat) != 0) {
		error("stat %s: %m.", script);
	}
	if (!(script_stat.st_mode & S_IFREG)) {
		error("script %s is not a regular file.", script);
	}


	/*
	 * Check if the script is owned by a regular user.
	 */

	struct passwd *owner;		/* The script's owner. */
	gid_t owner_gids[MAX_GROUPS];	/* Groups they are a member of. */
	int owner_ngids;		/* Number of those groups. */

	errno = 0;
	owner = getpwuid(script_stat.st_uid);
	if (!owner) {
		if (errno == 0) {
			error("script %s is owned by unallocated UID %llu.",
			      script, (long long unsigned) script_stat.st_uid);
		} else {
			error("getpwuid %llu: %m.",
			      (long long unsigned) script_stat.st_uid);
		}
	}

	assert(owner->pw_uid == script_stat.st_uid);

	if (owner->pw_uid < MIN_UID || owner->pw_uid > MAX_UID) {
		error("script %s is owned by privileged user %s.",
		      script, owner->pw_name);
	}

	rc = gids_get_list(owner->pw_name, owner->pw_gid,
	                   &owner_gids, &owner_ngids);
	switch (rc) {
		case OK:
			break;
		case ERR_GETGRENT:
			error("getgrent: %m.");
		case ERR_LEN:
			error("user %s belongs to too many groups.",
			      owner->pw_name);
		default:
			error("gids_get_list returned %u.", rc);
	}

	assert(owner_ngids > 0);

	for (int i = 0; i < owner_ngids; i++) {
		const gid_t gid = owner_gids[i];

		if (gid < MIN_GID || gid > MAX_GID) {
			error("user %s belongs to privileged group %llu.",
			      owner->pw_name, (long long unsigned) gid);
		}
	}

	assert(owner->pw_gid > MIN_GID);
	assert(owner->pw_gid < MAX_GID);	


	/*
	 * Drop privileges for good.
	 */

	errno = 0;
	if (seteuid(0) != 0) {
		error("seteuid 0: %m.");
	}

	rc = priv_drop(owner->pw_uid, owner->pw_gid, owner_ngids, owner_gids);
	switch (rc) {
		case OK:
			break;
		case ERR_SETGROUPS:
			error("setgroups %llu ...: %m.",
			      (long long unsigned) owner->pw_gid);
		case ERR_SETGID:
			error("setgid %llu: %m.",
			      (long long unsigned) owner->pw_gid);
		case ERR_SETUID:
			error("setuid %llu: %m.",
			      (long long unsigned) owner->pw_uid);
		case FAIL:
			error("could resume superuser privileges.");
		default:
			error("priv_drop returned %u.", rc);
	}

	assert(geteuid() == owner->pw_uid);
	assert(getegid() == owner->pw_gid);
	assert(getuid() == owner->pw_uid);
	assert(getgid() == owner->pw_gid);


	/*
	 * Set up a safer environment.
	 */

	errno = 0;

	if (setenv("DOCUMENT_ROOT", doc_root, true) != 0) {
		error("setenv DOCUMENT_ROOT: %m.");
	}

	if (setenv("HOME", owner->pw_dir, true) != 0) {
		error("setenv HOME: %m.");
	}

	if (setenv("PATH", PATH, true) != 0) {
		error("setenv PATH: %m.");
	}

	if (setenv("PATH_TRANSLATED", script, true) != 0) {
		error("setenv PATH_TRANSLATED: %m.");
	}

	if (setenv("USER_NAME", owner->pw_name, true) != 0) {
		error("setenv USER_NAME: %m.");
	}

	if (chdir(doc_root) != 0) {
		error("chdir %s: %m.", doc_root);
	}

	/* RATS: ignore; the umask is the user's responsibility. */
	umask(umask(0) | UMASK);


	/*
	 * Verify the document root.
	 *
	 * This guards against system operators making a mistake like:
	 *
	 *	cd /home/ismith/public_html/wp-plugins
	 *	cp -a /home/doe/public_html/wp-plugins/acme ./acme
	 *	chown -R jsmith:jsmith acme
	 *
	 * It also makes sure that users cannot break out of their directory.
	 */

	/* RATS: ignore; path_check_format respects MAX_STR. */
	char user_dir[MAX_STR];		/* The user directory. */

#if FORCE_HOME
	if (!path_contains(owner->pw_dir, doc_root)) {
		error("document root %s not within %s's home directory.",
		      doc_root, owner->pw_name);
	}
#endif /* FORCE_HOME */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wformat-extra-args"
	rc = path_check_format(doc_root, &user_dir, USER_DIR,
	                       owner->pw_dir, owner->pw_name);
#pragma GCC diagnostic pop
	switch (rc) {
		case OK:
			break;
		case ERR_REALPATH:
			error("realpath %s: %m.", user_dir);
		case ERR_LEN:
			error("expanded user directory is too long.");
		case FAIL:
			error("document root %s is not %s's user directory.",
			      doc_root, owner->pw_name);
		default:
			error("path_check_format returned %u.", rc);
	}


	/*
	 * There should be no need to run hidden files or a files that reside
	 * in hidden directories. So if the path to the script does refer to
	 * a hidden file, this probably indicates a configuration error.
	 */

	/* script is guaranteed to be canonical. */
	if (strstr(script, "/.")) {
		error("path %s contains hidden files.", script);
	}


	/*
	 * Although the set-user-ID and the set-group-ID on execute bits
	 * should be harmless, it would be odd for them to be set for a
	 * file that is owned by a regular user. So if either of them is
	 * set, this probably indicates a configuration error.
	 */

	if (script_stat.st_mode & S_ISUID) {
		error("script %s's set-user-ID bit is set.", script);
	}
	if (script_stat.st_mode & S_ISGID) {
		error("script %s's set-group-ID bit is set.", script);
	}


	/*
	 * If the webserver is (mis-)configured to allow visitors to change
	 * user files and a user (accidentally) set the permissions of a
	 * web-accessible script to be writable by the webserver or even
	 * the world, then visitors can run arbitrary code as that user.
	 *
	 * Also ensures that the document root and the CGI scripts are
	 * owned by the same UID, namely, owner->pw_uid.
	 */

	/* RATS: ignore; path_check_wexcl respects MAX_STR. */
	char path_cur[MAX_STR];		/* Current sub-path of script path. */

#if FORCE_HOME
#define BASE_DIR owner->pw_dir
#else
#define BASE_DIR doc_root
#endif

	rc = path_check_wexcl(owner->pw_uid, BASE_DIR, script, &path_cur);
	switch (rc) {
		case OK:
			break;
		case ERR_OPEN:
			error("open %s: %m.", path_cur);
		case ERR_CLOSE:
			error("close %s: %m.", path_cur);
		case ERR_STAT:
			error("stat %s: %m.", path_cur);
		case FAIL:
		        error("%s is writable by users other than %s.",
			      path_cur, owner->pw_name);
		default:
			error("path_check_wexcl returned %u.", rc);
	}


	/*
	 * Run the script.
	 */

	/* RATS: ignore; scpt_get_handler respects MAX_STR. */
	char handler[MAX_STR];		/* Script interpreter. */

	if (file_is_exec(script_stat)) {
		errno = 0;
		/* RATS: ignore; suCGI's point is to do this safely. */
		(void) execl(script, script, NULL);

		/* If this point is reached, execution has failed. */
		error("exec %s: %m.", script);
	}

	rc = scpt_get_handler((const struct scpt_ent []) HANDLERS,
	                      script, &handler);
	switch (rc) {
		case OK:
			break;
		case ERR_ILL:
			error("%s has no filename suffix.", script);
		case FAIL:
			error("no handler registered for %s.", script);
		default:
			error("scpt_get_handler returned %u.", rc);
	}

	assert(handler);
	assert(*handler != '\0');

	errno = 0;
	/* RATS: ignore; suCGI's point is to do this safely. */
	(void) execlp(handler, handler, script, NULL);

	/* If this point is reached, execution has failed. */
	error("exec %s %s: %m.", handler, script);
}
