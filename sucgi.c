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
#include <wordexp.h>

#include "config.h"
#include "env.h"
#include "error.h"
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

#undef JAIL_DIR
#define JAIL_DIR "/"

#undef DOC_ROOT
#define DOC_ROOT "$DOCUMENT_ROOT"

#undef MIN_UID
#define MIN_UID 500U

#undef MAX_UID
#define MAX_UID 30000U

#undef MIN_GID
#define MIN_GID 1U

#undef MAX_GID
#define MAX_GID 30000U

#undef HANDLERS
#define HANDLERS {{".sh", "sh"}, {NULL, NULL}}

#endif /* !defined(NDEBUG) && defined(TESTING) && TESTING */


/*
 * Configuration
 */

#if !defined(JAIL_DIR)
#error JAIL_DIR has not been set.
#endif /* !defined(JAIL_DIR) */

#if !defined(DOC_ROOT)
#error DOC_ROOT has not been set.
#endif /* !defined(DOC_ROOT) */

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

#if !defined(PATH)
#error PATH has not been set.
#endif

#if !defined(HANDLERS)
#error HANDLERS has not been set.
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

	/* RATS: ignore; env_clear respects ENV_MAX. */
	const char *vars[ENV_MAX];	/* Backup of environ(2). */
	enum error rc;			/* Return code. */

	rc = env_clear(&vars);
	switch (rc) {
		case OK:
			break;
		case ERR_ENV_MAX:
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
		error("setegid %llu: %m.", (unsigned long long) proc_rgid);
	}
	if (seteuid(proc_ruid) != 0) {
		error("seteuid %llu: %m.", (unsigned long long) proc_ruid);
	}

	assert(geteuid() == proc_ruid);
	assert(getegid() == proc_rgid);


	/*
	 * Restore the environment variables needed by CGI scripts.
	 */

	rc = env_restore(vars, env_vars_safe);
	switch (rc) {
		case OK:
			break;
		case ERR_SYS:
			error("setenv: %m.");
		case ERR_ENV_MAL:
			error("an environment variable is malformed.");
		case ERR_ENV_LEN:
			error("an environment variable is too long.");
		default:
			error("%s:%d: env_restore returned %u.",
			      __FILE__, __LINE__ - 12, rc);
	}


	/*
	 * Check whether $DOCUMENT_ROOT makes sense.
	 */

	const char *jail_dir;		/* Jail directory. */
	const char *doc_root;		/* $DOCUMENT_ROOT. */
	int doc_fd;			/* -- " -- file descriptor. */

        if (strnlen(JAIL_DIR, STR_MAX) >= STR_MAX) {
                error("path to jail directory is too long.");
        }

        /* RATS: ignore; this use of realpath should be safe. */
	jail_dir = realpath(JAIL_DIR, NULL);
	if (!jail_dir) {
		error("realpath %s: %m.", JAIL_DIR);
	}

	assert(jail_dir);
	assert(*jail_dir != '\0');
        assert(strnlen(jail_dir, STR_MAX) < STR_MAX);
        /* RATS: ignore; this use of realpath should be safe. */
	assert(strncmp(realpath(jail_dir, NULL), jail_dir, STR_MAX) == 0);

	rc = env_file_open(jail_dir, "DOCUMENT_ROOT",
	                   O_RDONLY | O_CLOEXEC | O_DIRECTORY,
			   &doc_root, &doc_fd);
	switch (rc) {
		case OK:
			break;
		case ERR_SYS:
			error("open $DOCUMENT_ROOT: %m.");
		case ERR_ENV_LEN:
			error("$DOCUMENT_ROOT: path too long.");
		case ERR_ENV_MAL:
			error("$DOCUMENT_ROOT: not within %s.", JAIL_DIR);
		case ERR_ENV_NIL:
			error("$DOCUMENT_ROOT: unset or empty.");
		default:
			error("%s:%d: env_file_open returned %u.",
			      __FILE__, __LINE__ - 17, rc);
	}

	assert(doc_root);
	assert(*doc_root != '\0');
	assert(doc_fd > -1);
	assert(strnlen(doc_root, STR_MAX) < STR_MAX);
        /* RATS: ignore; this use of realpath should be safe. */
	assert(strncmp(realpath(doc_root, NULL), doc_root, STR_MAX) == 0);

	if (close(doc_fd) != 0) {
		error("close $DOCUMENT_ROOT: %m.");
	}


	/*
	 * Check if $PATH_TRANSLATED makes sense.
	 */

	const char *path_trans;		/* $PATH_TRANSLATED. */
	int path_fd;			/* -- " -- file descriptor. */
	struct stat path_stat;		/* -- " -- filesystem metadata. */

	rc = env_file_open(doc_root, "PATH_TRANSLATED", O_RDONLY | O_CLOEXEC,
			   &path_trans, &path_fd);
	switch (rc) {
		case OK:
			break;
		case ERR_SYS:
			error("open $PATH_TRANSLATED: %m.");
		case ERR_ENV_LEN:
			error("$PATH_TRANSLATED: path too long.");
		case ERR_ENV_MAL:
			error("$PATH_TRANSLATED: not within $DOCUMENT_ROOT.");
		case ERR_ENV_NIL:
			error("$PATH_TRANSLATED: unset or empty.");
		default:
			error("%s:%d: env_file_open returned %u.",
			      __FILE__, __LINE__ - 17, rc);
	}

	assert(path_trans);
	assert(*path_trans != '\0');
	assert(strnlen(path_trans, STR_MAX) < STR_MAX);
        /* RATS: ignore; this use of realpath should be safe. */
	assert(strncmp(realpath(path_trans, NULL), path_trans, STR_MAX) == 0);
	
	if (fstat(path_fd, &path_stat) != 0) {
		error("stat $PATH_TRANSLATED: %m.");
	}
	if (!(path_stat.st_mode & S_IFREG)) {
		error("$PATH_TRANSLATED: not a regular file.");
	}


	/*
	 * Check if $PATH_TRANSLATED is owned by a regular user.
	 */

	struct passwd *owner;		/* $PATH_TRANSLATED's owner. */
	gid_t gids[NGROUPS_MAX];	/* Groups they are a member of. */
	int ngids;			/* Number of those groups. */

	if (path_stat.st_uid < MIN_UID || path_stat.st_uid > MAX_UID) {
		error("%s: owned by privileged UID %llu.",
		      path_trans, (unsigned long long) path_stat.st_uid);
	}

	owner = getpwuid(path_stat.st_uid);
	if (!owner) {
		if (errno == 0) {
			error("getpwuid %llu: no such user.",
			      (unsigned long long) path_stat.st_uid);
		} else {
			error("getpwuid %llu: %m.",
			      (unsigned long long) path_stat.st_uid);
		}
	}

	/* Paranoia is a virtue. */
	if (path_stat.st_uid != owner->pw_uid) {
		error("getpwuid %llu: returned wrong user %s.",
		      (unsigned long long) path_stat.st_uid, owner->pw_name);
	}

	rc = gids_get_list(owner->pw_name, owner->pw_gid, &gids, &ngids);
	switch (rc) {
		case OK:
			break;
		case ERR_SYS:
			error("getgrent: %m.");
		case ERR_GIDS_MAX:
			error("%s: in too many groups.", owner->pw_name);
		default:
			error("%s:%d: gids_get_list returned %u.",
			      __FILE__, __LINE__ - 11, rc);
	}

	assert(ngids > 0);

	for (int i = 0; i < ngids; i++) {
		const gid_t gid = gids[i];

		if (gid < MIN_GID || gid > MAX_GID) {
			error("%s: member of privileged group %llu.",
			      owner->pw_name, (unsigned long long) gid);
		}
	}


	/*
	 * Drop privileges for good.
	 */

	if (seteuid(0) != 0) {
		error("seteuid 0: %m.");
	}

	rc = priv_drop(owner->pw_uid, owner->pw_gid, ngids, gids);
	switch (rc) {
		case OK:
			break;
		case ERR_SYS:
			error("set id: %m.");
		case ERR_PRIV:
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
	 * Set up a safer environment.
	 */

	if (setenv("DOCUMENT_ROOT", doc_root, true) != 0) {
		error("setenv DOCUMENT_ROOT: %m.");
	}

	if (setenv("HOME", owner->pw_dir, true) != 0) {
		error("setenv HOME: %m.");
	}

	if (setenv("PATH", PATH, true) != 0) {
		error("setenv PATH: %m.");
	}

	if (setenv("PATH_TRANSLATED", path_trans, true) != 0) {
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
	 * Guard against system operators making a mistake like:
	 *
	 *	cd /home/ismith/public_html/wp-plugins
	 *	cp -a /home/doe/public_html/wp-plugins/acme ./acme
	 *	chown -R jsmith:jsmith acme
	 *
	 * Also a second line of defence against a user
	 * breaking out of their home directory.
	 */

	if (!path_contains(owner->pw_dir, doc_root)) {
		error("$DOCUMENT_ROOT: not within %s.", owner->pw_dir);
	}


	/*
	 * Verify $DOCUMENT_ROOT.
	 * FIXME: Not unit-tested.
	 */

	wordexp_t doc_exp;		/* Expected $DOCUMENT_ROOT. */
	int wordexp_rc;			/* wordexp return code. */

	wordexp_rc = wordexp((const char *) {DOC_ROOT},
	                     &doc_exp, WRDE_NOCMD | WRDE_UNDEF);
	switch (wordexp_rc) {
		case 0:
			break;
		case WRDE_BADCHAR:
			error("%s: unquoted meta-character.", DOC_ROOT);
		case WRDE_BADVAL:
			error("%s: refers to undefined variable.", DOC_ROOT);
		case WRDE_CMDSUB:
      			error("%s: command substitution.", DOC_ROOT);
		case WRDE_NOSPACE:
			error("%s: not enough memory to expand.", DOC_ROOT);
		case WRDE_SYNTAX:
			error("%s: syntax error.", DOC_ROOT);
		default:
			error("%s:%d: wordexp returned %d.",
			      __FILE__, __LINE__ - 8, wordexp_rc);
	}

	if (doc_exp.we_wordc < 1) {
		error("%s: failed to shell-expand.", DOC_ROOT);
	}	
	if (strncmp(doc_exp.we_wordv[0], doc_root, STR_MAX) != 0) {
		error("$DOCUMENT_ROOT: not %s.", doc_exp.we_wordv[0]);
	}


	/*
	 * There should be no need to run hidden files or a files that reside
	 * in hidden directories. So if $PATH_TRANSLATED does contain a hidden
	 * file or directory, this probably indicates a configuration error.
	 */

	/* path_trans is guaranteed to be canonical. */
	if (strstr(path_trans, "/.")) {
		error("%s: path contains hidden files.", path_trans);
	}


	/*
	 * Although the set-user-ID and the set-group-ID on execute bits
	 * should be harmless, it would be odd for them to be set for a
	 * file that is owned by a regular user. So if either of them is
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
	 * owned by the same UID, namely, owner->pw_uid.
	 */

	/* RATS: ignore; path_check_wexcl respects STR_MAX. */
	char path_cur[STR_MAX];		/* Sub-path of $PATH_TRANSLATED. */

	rc = path_check_wexcl(owner->pw_uid, owner->pw_dir,
	                      path_trans, &path_cur);
	switch (rc) {
		case OK:
			break;
		case ERR_SYS:
			error("open %s: %m.", path_cur);
		case ERR_PATH_WEXCL:
		        error("%s: writable by users other than %s.",
			      path_cur, owner->pw_name);
		default:
			error("%s:%d: path_check_wexcl returned %u.",
			      __FILE__, __LINE__ - 11, rc);
	}


	/*
	 * Run the programme.
	 */


	if (!file_is_exec(path_stat)) {
		const char *handler;	/* Handler for $PATH_TRANSLATED. */

		rc = scpt_get_handler((const struct scpt_ent []) HANDLERS,
		                      path_trans, &handler);
		switch (rc) {
			case OK:
				break;
			case ERR_SCPT_NO_HDL:
				error("%s: no handler registered.",
				      path_trans);
			case ERR_SCPT_NO_SFX:
				error("%s: has no filename suffix.",
				      path_trans);
			default:
				error("%s:%d: scpt_get_handler returned %u.",
				      __FILE__, __LINE__ - 7, rc);
		}

		assert(handler);
		assert(*handler != '\0');

		/* RATS: ignore; suCGI's point is to do this safely. */
		(void) execlp(handler, handler, path_trans, NULL);

		/* If this point is reached, execution has failed. */
		error("exec %s %s: %m.", handler, path_trans);
	}

	/* RATS: ignore; suCGI's point is to do this safely. */
	(void) execl(path_trans, path_trans, NULL);

	/* If this point is reached, execution has failed. */
	error("exec %s: %m.", path_trans);
}
