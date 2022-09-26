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
#include <wordexp.h>

#include "config.h"
#include "env.h"
#include "err.h"
#include "file.h"
#include "gids.h"
#include "path.h"
#include "priv.h"
#include "scpt.h"
#include "str.h"


/*
 * Test builds
 */

#if !defined(NDEBUG) && defined(TESTING) && TESTING

#undef DOC_ROOT_FLAGS
#define DOC_ROOT_FLAGS FNM_PERIOD

#undef DOC_ROOT_PATTERN
#define DOC_ROOT_PATTERN "/*/*"

#undef MIN_UID
#define MIN_UID 500U

#undef MAX_UID
#define MAX_UID 30000U

#undef MIN_GID
#define MIN_GID 1U

#undef MAX_GID
#define MAX_GID 30000U

#undef SCRIPT_HANDLERS
#define SCRIPT_HANDLERS {{".sh", "sh"}, {NULL, NULL}}

#endif /* !defined(NDEBUG) && defined(TESTING) && TESTING */


/*
 * Configuration
 */

#if !defined DOC_ROOT_FLAGS
#define DOC_ROOT_FLAGS (FNM_PATHNAME | FNM_PERIOD)
#endif

#if !defined(DOC_ROOT_BASE)
#error DOC_ROOT_BASE has not been set.
#endif /* !defined(DOC_ROOT_BASE) */

#if !defined(DOC_ROOT_PATTERN)
#error DOC_ROOT_PATTERN has not been set.
#endif /* !defined(DOC_ROOT_PATTERN) */

#if MIN_UID <= 0 
#error MIN_UID must be greater than 0.
#endif /* MIN_UID <= 0 */

#if MAX_UID < MIN_UID
#error MAX_UID is smaller than MIN_UID.
#endif /* MAX_UID <= MIN_UID */

#if MIN_GID <= 0
#error MIN_GID must be greater than 0.
#endif /* MIN_GID <= 0 */

#if MAX_GID < MIN_GID
#error MAX_GID is smaller than MIN_GID.
#endif /* MAX_GID <= MIN_GID */

#if !defined(SECURE_PATH)
#error SECURE_PATH has not been set.
#endif

#if !defined(SCRIPT_HANDLERS)
#error SCRIPT_HANDLERS has not been set.
#endif


/*
 * Main
 */

int
main(void) {
	errno = 0;

	/*
	 * Words of wisdom from the authors of suEXEC:
	 *
	 * > While cleaning the environment, the environment should be clean.
	 * > (E.g. malloc() may get the name of a file for writing debugging
	 * > info. Bad news if MALLOC_DEBUG_FILE is set to /etc/passwd.)
	 */

	const char *vars[SC_ENV_MAX];	/* Backup of environ(2). */
	enum error rc;			/* Return code. */

	rc = env_clear(&vars);
	switch (rc) {
		case SC_OK:
			break;
		case SC_ERR_ENV_MAX:
			error("too many environment variables.");
		default:
			error("%s:%d: env_clear returned %u.",
			      __FILE__, __LINE__ - 7, rc);
	}
	
	assert(!*environ);


	/*
	 * Drop privileges temporarily.
	 */

	uid_t proc_ruid;		/* Process' real user ID. */
	gid_t proc_rgid;		/* Process' real group ID. */

	proc_ruid = getuid();
	proc_rgid = getgid();

	if (setegid(proc_rgid) != 0) {
		error("setegid %llu: %s.",
		      (uint64_t) proc_rgid, strerror(errno));
	}
	if (seteuid(proc_ruid) != 0) {
		error("seteuid %llu: %s.",
		      (uint64_t) proc_ruid, strerror(errno));
	}

	assert(geteuid() == proc_ruid);
	assert(getegid() == proc_rgid);


	/*
	 * Restore the environment variables needed by CGI scripts.
	 */

	rc = env_restore(vars, env_keep, env_toss);
	switch (rc) {
		case SC_OK:
			break;
		case SC_ERR_SYS:
			error("setenv: %s.", strerror(errno));
		case SC_ERR_ENV_MAL:
			error("an environment variable is malformed.");
		case SC_ERR_ENV_LEN:
			error("an environment variable is too long.");
		default:
			error("%s:%d: env_restore returned %u.",
			      __FILE__, __LINE__ - 12, rc);
	}


	/*
	 * Check whether $DOCUMENT_ROOT makes sense.
	 */

	const char *doc_root;		/* $DOCUMENT_ROOT itself. */
	const char *doc_root_base;	/* $DOCUMENT_ROOT base directory. */
	int doc_root_fd;		/* $DOCUMENT_ROOT file descriptor. */

	doc_root_base = realpath(DOC_ROOT_BASE, NULL);
	if (!doc_root_base) {
		error("realpath %s: %s.", DOC_ROOT_BASE, strerror(errno));
	}

	assert(doc_root_base);
	assert(*doc_root_base != '\0');
	assert(strcmp(realpath(doc_root_base, NULL), doc_root_base) == 0);

	rc = env_file_openat(doc_root_base, "DOCUMENT_ROOT",
	                     O_RDONLY | O_CLOEXEC | O_DIRECTORY,
			     &doc_root, &doc_root_fd);

	switch (rc) {
		case SC_OK:
			break;
		case SC_ERR_SYS:
			error("$DOCUMENT_ROOT: %s.", strerror(errno));
		case SC_ERR_ENV_LEN:
			error("$DOCUMENT_ROOT: path is too long.");
		case SC_ERR_ENV_MAL:
			error("$DOCUMENT_ROOT: not in %s.", DOC_ROOT_BASE);
		case SC_ERR_ENV_NIL:
			error("$DOCUMENT_ROOT: unset or empty.");
		default:
			error("%s:%d: env_file_openat returned %u.",
			      __FILE__, __LINE__ - 17, rc);
	}

	if (close(doc_root_fd) != 0) {
		error("close $DOCUMENT_ROOT: %s.", strerror(errno));
	}

#if 0
	rc = env_get_fname("DOCUMENT_ROOT", S_IFDIR,
	                   &doc_root, &doc_root_stat);

	switch (rc) {
		case SC_OK:
			break;
		case SC_ERR_SYS:
			error("$DOCUMENT_ROOT: %s.", strerror(errno));
		case SC_ERR_FTYPE:
			error("$DOCUMENT_ROOT: not a directory.");
		case SC_ERR_ENV_LEN:
			error("$DOCUMENT_ROOT: resolved path is too long.");
		case SC_ERR_ENV_NIL:
			error("$DOCUMENT_ROOT: unset or empty.");
		default:
			error("%s:%d: env_get_fname returned %u.",
			      __FILE__, __LINE__ - 14, rc);
	}
#endif
	assert(doc_root);
	assert(*doc_root != '\0');
	assert(doc_root_fd > -1);
	assert(strnlen(doc_root, STR_MAX) < STR_MAX);
	assert(strncmp(realpath(doc_root, NULL), doc_root, STR_MAX) == 0);

#if 0
	/*
	 * TODO: we check later whether the doc root is within $HOME.
	 * if we checked that the doc root matches "~/<doc_root>",
	 * say public_html, then this would be as good, and simpler
	 * to set up.
	 */

	if (fnmatch((const char *) DOC_ROOT_PATTERN, doc_root,
	            DOC_ROOT_FLAGS) != 0)
	{
		error("$DOCUMENT_ROOT: does not match %s.",
		      (const char *) DOC_ROOT_PATTERN);
	}
#endif

	/*
	 * Check if $PATH_TRANSLATED makes sense.
	 */
#if 0
	rc = env_get_fname("PATH_TRANSLATED", S_IFREG,
	                   &path_trans, &path_stat);
	switch (rc) {
		case SC_OK:
			break;
		case SC_ERR_SYS:
			error("$PATH_TRANSLATED: %s.", strerror(errno));
		case SC_ERR_FTYPE:
			error("$PATH_TRANSLATED: not a regular file.");
		case SC_ERR_STR_LEN:
			error("$PATH_TRANSLATED: resolved path is too long.");
		case SC_ERR_ENV_NIL:
			error("$PATH_TRANSLATED: unset or empty.");
		default:
			error("%s:%d: env_get_fname returned %u.",
			      __FILE__, __LINE__ - 20, rc);
	}
#endif

	const char *path_trans;		/* $PATH_TRANSLATED. */
	int path_fd;			/* $PATH_TRANSLATED file descriptor. */
	struct stat path_stat;		/* $PATH_TRANSLATED metadata. */

	rc = env_file_openat(doc_root, "PATH_TRANSLATED",
	                     O_RDONLY | O_CLOEXEC,
			     &path_trans, &path_fd);

	switch (rc) {
		case SC_OK:
			break;
		case SC_ERR_SYS:
			error("$PATH_TRANSLATED: %s.", strerror(errno));
		case SC_ERR_ENV_LEN:
			error("$PATH_TRANSLATED: path is too long.");
		case SC_ERR_ENV_MAL:
			error("$PATH_TRANSLATED: not in $DOCUMENT_ROOT.");
		case SC_ERR_ENV_NIL:
			error("$PATH_TRANSLATED: unset or empty.");
		default:
			error("%s:%d: env_file_openat returned %u.",
			      __FILE__, __LINE__ - 17, rc);
	}

	assert(path_trans);
	assert(*path_trans != '\0');
	assert(strnlen(path_trans, STR_MAX) < STR_MAX);
	assert(strncmp(realpath(path_trans, NULL), path_trans, STR_MAX) == 0);
	
	if (fstat(path_fd, &path_stat) != 0) {
		error("fstat $PATH_TRANSLATED: %s.", strerror(errno));
	}
	if (!(path_stat.st_mode & S_IFREG)) {
		error("$PATH_TRANSLATED: not a regular file.");
	}

#if 0
	if (!path_contains(doc_root, path_trans)) {
		error("$PATH_TRANSLATED: not in document root %s.", doc_root);
	}
#endif

	/*
	 * Check if $PATH_TRANSLATED is owned by a regular user.
	 */

	struct passwd *owner;		/* $PATH_TRANSLATED's owner. */
	gid_t owner_gids[NGROUPS_MAX];	/* Groups the owner is in. */
	int owner_ngids;		/* Number of those groups. */

	if (path_stat.st_uid < MIN_UID || path_stat.st_uid > MAX_UID) {
		error("%s: owned by privileged UID %llu.",
		      path_trans, (uint64_t) path_stat.st_uid);
	}

	owner = getpwuid(path_stat.st_uid);
	if (!owner) {
		char *err = (errno == 0) ? "no such user" : strerror(errno);
		error("getpwuid %llu: %s.", (uint64_t) path_stat.st_uid, err);
	}

	/* Paranoia is a virtue. */
	if (path_stat.st_uid != owner->pw_uid) {
		error("getpwuid %llu: returned wrong user %s.",
		      (uint64_t) path_stat.st_uid, owner->pw_name);
	}

	rc = gids_get_list(owner->pw_name, owner->pw_gid,
	                   &owner_gids, &owner_ngids);
	switch (rc) {
		case SC_OK:
			break;
		case SC_ERR_SYS:
			error("getgrent: %s.", strerror(errno));
		case SC_ERR_GIDS_MAX:
			error("%s: in too many groups.", owner->pw_name);
		default:
			error("%s:%d: gids_get_list returned %u.",
			      __FILE__, __LINE__ - 11, rc);
	}

	assert(owner_ngids > 0);

	for (int i = 0; i < owner_ngids; i++) {
		const gid_t gid = owner_gids[i];
		
		if (gid < MIN_GID || gid > MAX_GID) {
			error("%s: member of privileged group %llu.",
			      owner->pw_name, (uint64_t) gid);
		}
	}


	/*
	 * Drop privileges for good.
	 */

	if (seteuid(0) != 0) error("seteuid 0: %s.", strerror(errno));

	rc = priv_drop(owner->pw_uid, owner->pw_gid, owner_ngids, owner_gids);
	switch (rc) {
		case SC_OK:
			break;
		case SC_ERR_SYS:
			error("failed to drop privileges: %s.",
			      strerror(errno));
		case SC_ERR_PRIV:
			error("could resume privileges.");
		default:
			error("%s:%d: priv_drop returned %u.",
			      __FILE__, __LINE__ - 11, rc);
	}

	assert(geteuid() == owner->pw_uid);
	assert(getegid() == owner->pw_gid);
	assert(getuid() == owner->pw_uid);
	assert(getgid() == owner->pw_gid);


	/*
	 * Guard against system operators making a mistake like:
	 *
	 *	cd /home/ismith/public_html/wp-plugins
	 *	cp -a /home/doe/public_html/wp-plugins/acme ./acme
	 *	chown -R jsmith:jsmith acme
	 *
	 * Also a second line of defence against users
	 * attempting to break out of their home directory.
	 */

	if (!path_contains(owner->pw_dir, doc_root)) {
		error("$DOCUMENT_ROOT: not in %s.", owner->pw_dir);
	}


	/*
	 * FIXME: Factor out and add documentation.
	 */

	wordexp_t doc_root_exp;		/* Expected $DOCUMENT_ROOT. */
	int wordexp_rc;			/* wordexp return code. */

	wordexp_rc = wordexp((const char *) {DOC_ROOT_PATH},
	                     &doc_root_exp, WRDE_NOCMD | WRDE_UNDEF);
	switch (wordexp_rc) {
		case 0:
			break;
		default:
			error("%s:%d: wordexp returned %d.",
			      __FILE__, __LINE__ - 7, wordexp_rc);
	}

	if (doc_root_exp.we_wordc < 1) {
		error("FIXME!");
	}	
	if (strcmp(doc_root_exp.we_wordv[0], doc_root) != 0) {
		error("FIXME!");
	}


	/*
	 * There should be no need to run a hidden file or a file that resides
	 * in a hidden directory. So if $PATH_TRANSLATED does contain a hidden
	 * file or directory, this probably indicates a configuration error.
	 */

	/* path_trans is guaranteed to be canonical. */
	if (strstr(path_trans, "/.")) {
		error("%s: path contains hidden files.", path_trans);
	}


	/*
	 * Although the set-user-ID and the set-group-ID on execute bits
	 * should be harmless, it would be odd for them to be set for a
	 * file that is owned by a regular user, So if either of them is
	 * set, this probably indicates a configuration error.
	 */

	if (path_stat.st_mode & S_ISUID) {
		error("%s: set-user-ID on execute bit set.", path_trans);
	}
	if (path_stat.st_mode & S_ISGID) {
		error("%s: set-group-ID on execute bit set.", path_trans);
	}


	/*
	 * If the webserver is (mis-)configured to allow visitors to change
	 * user files and a user (accidentally) set the permissions of a
	 * web-accessible script to be writable by the webserver or even
	 * the world, then visitors can run arbitrary code as that user.
	 *
	 * Also ensures that the document root and the CGI scripts are
	 * both owned by the same UID, namely, owner->pw_uid.
	 */

	char path_cur[STR_MAX];		/* Sub-path of $PATH_TRANSLATED. */

	rc = path_check_wexcl(owner->pw_uid, owner->pw_dir,
	                      path_trans, &path_cur);
	switch (rc) {
		case SC_OK:
			break;
		case SC_ERR_SYS:
			error("open %s: %s.", path_cur, strerror(errno));
		case SC_ERR_PATH_WEXCL:
		        error("%s: writable by users other than %s.",
			      path_cur, owner->pw_name);
		default:
			error("%s:%d: path_check_wexcl returned %u.",
			      __FILE__, __LINE__ - 11, rc);
	}


	/*
	 * Set up a safer environment.
	 */

	if (setenv("DOCUMENT_ROOT", doc_root, true) != 0) {
		error("setenv DOCUMENT_ROOT: %s.", strerror(errno));
	}

	if (setenv("PATH", (const char *const) SECURE_PATH, true) != 0) {
		error("setenv PATH: %s.", strerror(errno));
	}

	if (setenv("PATH_TRANSLATED", path_trans, true) != 0) {
		error("setenv PATH_TRANSLATED: %s.", strerror(errno));
	}

	if (setenv("USER_NAME", owner->pw_name, true) != 0) {
		error("setenv USER_NAME: %s.", strerror(errno));
	}

	if (chdir(doc_root) != 0) {
		error("chdir %s: %s.", doc_root, strerror(errno));
	}

	umask(umask(0) | UMASK);


	/*
	 * Run the programme.
	 */

	const char *path_handler;	/* Handler for $PATH_TRANSLATED. */

	if (file_is_exec(&path_stat)) {
		/* RATS: ignore; suCGI's point is to do this safely. */
		(void) execl(path_trans, path_trans, NULL);

		/* If this point is reached, execution has failed. */
		error("exec %s: %s.", path_trans, strerror(errno));
	}

	rc = scpt_get_handler((const struct scpt_ent []) SCRIPT_HANDLERS,
	                      path_trans, &path_handler);
	switch (rc) {
		case SC_OK:
			break;
		case SC_ERR_SCPT_NO_HDL:
			error("%s: no handler registered.", path_trans);
		/* FIXME: This should be impossible and not be checked for. */
		case SC_ERR_SCPT_ONLY_SFX:
			error("%s: filename starts with a dot.", path_trans);
		case SC_ERR_SCPT_NO_SFX:
			error("%s: has no filename suffix.", path_trans);
		default:
			error("%s:%d: scpt_get_handler returned %u.",
			      __FILE__, __LINE__ - 10, rc);
	}

	assert(path_handler);
	assert(*path_handler != '\0');

	/* RATS: ignore; suCGI's point is to do this safely. */
	(void) execlp(path_handler, path_handler, path_trans, NULL);

	/* If this point is reached, execution has failed. */
	error("exec %s %s: %s.", path_handler, path_trans, strerror(errno));
}
