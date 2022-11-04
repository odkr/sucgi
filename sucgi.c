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
#endif

#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <limits.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "env.h"
#include "error.h"
#include "file.h"
#include "gids.h"
#include "path.h"
#include "priv.h"
#include "script.h"
#include "str.h"
#include "sysdefs.h"
#include "types.h"


/*
 * Constants
 */

/* Programme version. */
#define VERSION "0"

/* Maximum number of groups that users are likely to be a member of. */
#define PRELIM_NGROUPS 32


/*
 * Macros
 */

/* Raise a compiler error if COND is false. */
#define BUILD_BUG_ON(cond) ((void)sizeof(char[1 - 2*!!(cond)]))


/*
 * Configuration for test builds
 */

#if !defined(NDEBUG) && defined(TESTING) && TESTING

#undef JAIL_DIR
#define JAIL_DIR "/"

#undef USER_DIR 
#define USER_DIR "/tmp/check-sucgi/%s"

#undef FORCE_HOME
#define FORCE_HOME false

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

#undef PRELIM_NGROUPS
#define PRELIM_NGROUPS 0

#endif /* !defined(NDEBUG) && defined(TESTING) && TESTING */


/*
 * Verification of configuration
 */

#if !defined(JAIL_DIR)
#error JAIL_DIR is undefined.
#endif

#if !defined(USER_DIR)
#error USER_DIR is undefined.
#endif

#if !defined(FORCE_HOME)
#error FORCE_HOME is undefined.
#endif

#if !defined(MIN_UID)
#error MIN_UID is undefined.
#endif

#if !defined(MAX_UID)
#error MIN_UID is undefined.
#endif

#if MIN_UID <= 0 
#error MIN_UID must be greater than 0.
#endif

#if MAX_UID < MIN_UID
#error MAX_UID is smaller than MIN_UID.
#endif

#if defined(UID_MAX)
#if MAX_UID > UID_MAX
#error MAX_UID is greater than UID_MAX.
#endif
#else /* !defined(MAX_UID) */
#if MAX_UID > UINT_MAX
#error MAX_UID is greater than UINT_MAX.
#endif
#endif /* defined(MAX_UID) */

#if !defined(MIN_GID)
#error MIN_GID is undefined.
#endif

#if !defined(MAX_GID)
#error MIN_GID is undefined.
#endif

#if MIN_GID <= 0
#error MIN_GID must be greater than 0.
#endif

#if MAX_GID < MIN_GID
#error MAX_GID is smaller than MIN_GID.
#endif

#if defined(GID_MAX)
#if MAX_GID > GID_MAX
#error MAX_GID is greater than GID_MAX.
#endif
#else /* !defined(MAX_GID) */
#if MAX_GID > UINT_MAX
#error MAX_GID is greater than UINT_MAX.
#endif
#endif /* defined(MAX_GID) */

#if !defined(SEC_VARS)
#error SEC_VARS is undefined.
#endif

#if !defined(HANDLERS)
#error HANDLERS is undefined.
#endif

#if !defined(SEC_PATH)
#error SEC_PATH is undefined.
#endif


/*
 * Main
 */

int
main(int argc, char **argv) {
	/*
	 * Check whether configuration strings are within bounds.
	 */

	BUILD_BUG_ON(sizeof(JAIL_DIR) <= 1);
	BUILD_BUG_ON(sizeof(JAIL_DIR) >= PATH_SIZE);
	BUILD_BUG_ON(sizeof(USER_DIR) <= 1);
	BUILD_BUG_ON(sizeof(USER_DIR) >= PATH_SIZE);
	BUILD_BUG_ON(sizeof(SEC_PATH) <= 1);
	BUILD_BUG_ON(sizeof(SEC_PATH) >= PATH_SIZE);

	
	/*
	 * Words of wisdom from the authors of suEXEC:
	 *
	 * > While cleaning the environment, the environment should be clean.
	 * > (E.g. malloc() may get the name of a file for writing debugging
	 * > info. Bad news if MALLOC_DEBUG_FILE is set to /etc/passwd.)
	 */

	/* RATS: ignore; writes to env respect MAX_NVARS. */
	const char *env[MAX_NVARS];	/* Backup of environment. */
	enum retcode rc;		/* Return code. */

	rc = env_clear(MAX_NVARS, env);
	switch (rc) {
	case OK:
		break;
	case ERR_LEN:
		error("too many environment variables.");
	default:
		/* Should be unreachable. */
		error("%d: env_clear returned %u.", __LINE__, rc);
	}
	
	assert(!*environ);


	/*
	 * Drop privileges temporarily.
	 */

	errno = 0;
	if (setegid(getgid()) != 0)
		/* Should be unreachable. */
		error("setegid: %m.");
	errno = 0;
	if (seteuid(getuid()) != 0)
		/* Should be unreachable. */
		error("seteuid: %m.");


	/*
	 * Process arguments.
	 */

	for (int i = 1; i < argc; i++) {
		if (strncmp(argv[i], "-h", 3) == 0) {
			(void) puts(
"suCGI - run CGI scripts with the permissions of their owner\n\n"
"Usage:  sucgi\n"
"        sucgi [-c|-h]\n\n"
"Options:\n"
"    -h  Print this help screen.\n"
"    -c  Print build configuration.\n"
"    -V  Print version and license."
			       );
			return EXIT_SUCCESS;
		} else if (strncmp(argv[i], "-c", 3) == 0) {
			struct pair hdb[] = HANDLERS;

			(void) printf("JAIL_DIR=%s\n", JAIL_DIR);
			(void) printf("USER_DIR=%s\n", USER_DIR);
			(void) printf("FORCE_HOME=%d\n", FORCE_HOME);

			(void) printf("MIN_UID=%u\n", MIN_UID);
			(void) printf("MAX_UID=%u\n", MAX_UID);
			(void) printf("MIN_GID=%u\n", MIN_GID);
			(void) printf("MAX_GID=%u\n", MAX_GID);

			(void) printf("HANDLERS=");
			for (int j = 0; hdb[j].key; j++) {
				struct pair hdl = hdb[j];
				
				if (j > 0)
					(void) printf(",");
				(void) printf("%s:%s", hdl.key, hdl.value);
			}
			(void) printf("\n");

			(void) printf("SEC_PATH=%s\n", SEC_PATH);
			(void) printf("UMASK=0%o\n", UMASK);

			(void) printf("MAX_NVARS=%u\n", MAX_NVARS);
			(void) printf("PATH_SIZE=%d\n", PATH_SIZE);

			return EXIT_SUCCESS;
		} else if (strncmp(argv[i], "-V", 3) == 0) {
			(void) puts(
"suCGI v" VERSION "\n"
"Copyright 2022 Odin Kroeger.\n"
"Released under the GNU General Public License.\n"
"This programme comes with ABSOLUTELY NO WARRANTY."
			       );
			return EXIT_SUCCESS;
		} else {
			(void) fputs("usage: sucgi [-h|-c]\n", stderr);
			return EXIT_FAILURE;
		}
	}


	/*
	 * Restore the environment variables needed by CGI scripts.
	 */
	
	const char *const sec_vars[] = SEC_VARS;	/* Secure variables. */

	rc = env_restore(env, sec_vars);
	switch (rc) {
	case OK:
		break;
	case ERR_ENV:
		error("setenv: %m.");
	case ERR_ILL:
		error("encountered malformed environment variable.");
	case ERR_LEN:
		error("encountered suspiciously long environment variable.");
	default:
		/* Should be unreachable. */
		error("%d: env_restore returned %u.", __LINE__, rc);
	}


	/*
	 * Get the document root.
	 */

	const char *jail_dir;		/* Jail directory. */
	const char *doc_root;		/* Document root. */
	int doc_fd;			/* -- " -- file descriptor. */

	errno = 0;
	/* RATS: ignore; the length of JAIL_DIR is checked above. */
	jail_dir = realpath(JAIL_DIR, NULL);
	if (!jail_dir)
		error("realpath %s: %m.", JAIL_DIR);

	assert(jail_dir);
	assert(*jail_dir);
        assert(strnlen(jail_dir, PATH_SIZE) < PATH_SIZE);

	rc = env_fopen(jail_dir, "DOCUMENT_ROOT", O_RDONLY | O_DIRECTORY,
	               &doc_root, &doc_fd);
	switch (rc) {
	case OK:
		break;
	case ERR_ENV:
		error("getenv: %m.");
	case ERR_MEM:
		error("strndup: %m.");
	case ERR_RES:
		error("realpath %s: %m.", doc_root);
	case ERR_OPEN:
		error("open %s: %m.", doc_root);
	case ERR_LEN:
		error("path to document root is too long.");
	case ERR_ILL:
		error("document root %s not within jail.", doc_root);
	case ERR_NIL:
		error("$DOCUMENT_ROOT unset or empty.");
	default:
		/* Should be unreachable. */
		error("%d: env_fopen returned %u.", __LINE__, rc);
	}

	if (close(doc_fd) != 0)
		error("close %s: %m.", doc_root);

	assert(doc_root);
	assert(*doc_root);
	assert(doc_fd > -1);
	assert(strnlen(doc_root, PATH_SIZE) < PATH_SIZE);
	/* RATS: ignore; not a permission check. */
	assert(access(doc_root, F_OK) == 0);
	/* RATS: ignore; the length of doc_root is checked above. */
	assert(strncmp(realpath(doc_root, NULL), doc_root, PATH_SIZE) == 0);


	/*
	 * Get the script.
	 */

	const char *script;		/* Path to script. */
	int script_fd;			/* Script file descriptor. */
	struct stat script_stat;	/* -- " -- filesystem metadata. */

	rc = env_fopen(doc_root, "PATH_TRANSLATED", O_RDONLY,
	               &script, &script_fd);
   	switch (rc) {
	case OK:
		break;
	case ERR_ENV:
		error("getenv: %m.");
	case ERR_MEM:
		error("calloc: %m.");
	case ERR_RES:
		error("realpath %s: %m.", script);
	case ERR_OPEN:
		error("open %s: %m.", script);
	case ERR_LEN:
		error("path to script is too long.");
	case ERR_ILL:
		error("script %s not within document root.", script);
	case ERR_NIL:
		error("$PATH_TRANSLATED unset or empty.");
	default:
		/* Should be unreachable. */
		error("%d: env_fopen returned %u.", __LINE__, rc);
   	}

	assert(script);
	assert(*script);
	assert(strnlen(script, PATH_SIZE) < PATH_SIZE);
	/* RATS: ignore; not a permission check. */
	assert(access(script, F_OK) == 0);
	/* RATS: ignore; the length of script is checked above. */
	assert(strncmp(realpath(script, NULL), script, PATH_SIZE) == 0);

	errno = 0;
	if (fstat(script_fd, &script_stat) != 0)
		error("stat %s: %m.", script);
	if (!(script_stat.st_mode & S_IFREG))
		error("script %s is not a regular file.", script);


	/*
	 * Check if the script is owned by a regular user.
	 */

	struct passwd *owner;		/* Script owner. */
	gid_t *groups;			/* Groups they are a member of. */
	int ngroups;			/* Number of groups. */

	errno = 0;
	owner = getpwuid(script_stat.st_uid);
	if (!owner) {
		if (errno == 0)
			error("script %s is owned by unallocated UID %llu.",
			      script, (long long unsigned) script_stat.st_uid);
		else
			error("getpwuid %llu: %m.",
			      (long long unsigned) script_stat.st_uid);
	}

	assert(owner->pw_uid == script_stat.st_uid);

	if (owner->pw_uid < MIN_UID || owner->pw_uid > MAX_UID)
		error("script %s is owned by privileged user %s.",
		      script, owner->pw_name);

	ngroups = PRELIM_NGROUPS;
	groups = malloc((size_t) ngroups * sizeof(*groups));
	if (!groups)
		error("malloc: %m.");

	rc = gids_get(owner->pw_name, owner->pw_gid, groups, &ngroups);

	assert(ngroups > 0);

	if (rc == ERR_LEN) {
		size_t gid_size = sizeof(*groups);

		/* Check for overflow. */
		if ((size_t) ngroups > SIZE_MAX/gid_size)
			error("user %s belongs to too many groups.",
		              owner->pw_name);

		/* RATS: ignore; garbage irrelevant, alignment correct. */
		groups = realloc(groups, (size_t) ngroups * gid_size);
		if (!groups)
			error("realloc: %m");
		
		rc = gids_get(owner->pw_name, owner->pw_gid, groups, &ngroups);
	}

	switch (rc) {
	case OK:
		break;
	case ERR_GETGR:
		error("getgrent: %m.");
	default:
		/* Should be unreachable. */
		error("%d: gids_get returned %u.", __LINE__, rc);
	}

	assert(ngroups > 0);

	for (int i = 0; i < ngroups; i++) {
		gid_t gid = groups[i];

		if (MAX_GID < gid || gid < MIN_GID)
			error("user %s belongs to privileged group %llu.",
			      owner->pw_name, (long long unsigned) gid);
	}

	assert(owner->pw_gid >= MIN_GID);
	assert(owner->pw_gid <= MAX_GID);	


	/*
	 * Drop privileges for good.
	 */

	errno = 0;
	if (seteuid(0) != 0)
		error("seteuid: %m.");

	rc = priv_drop(owner->pw_uid, owner->pw_gid, ngroups, groups);
	switch (rc) {
	case OK:
		break;
	case ERR_SETGRPS:
		error("setgroups: %m.");
	case ERR_SETGID:
		error("setgid: %m.");
	case ERR_SETUID:
		error("setuid: %m.");
	case FAIL:
		error("could resume superuser privileges.");
	default:
		/* Should be unreachable. */
		error("%d: priv_drop returned %u.", __LINE__, rc);
	}

	assert(geteuid() == owner->pw_uid);
	assert(getegid() == owner->pw_gid);
	assert(getuid() == owner->pw_uid);
	assert(getgid() == owner->pw_gid);


	/*
	 * Set up a safer environment.
	 */

	errno = 0;
	if (setenv("DOCUMENT_ROOT", doc_root, true) != 0)
		error("setenv: %m.");

	errno = 0;
	if (setenv("HOME", owner->pw_dir, true) != 0)
		error("setenv: %m.");

	errno = 0;
	if (setenv("PATH", SEC_PATH, true) != 0)
		error("setenv: %m.");

	errno = 0;
	if (setenv("PATH_TRANSLATED", script, true) != 0)
		error("setenv: %m.");

	errno = 0;
	if (setenv("USER_NAME", owner->pw_name, true) != 0)
		error("setenv: %m.");

	errno = 0;
	if (chdir(doc_root) != 0)
		error("chdir %s: %m.", doc_root);

	/* RATS: ignore; the umask is the administrator's responsibility. */
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

	/* RATS: ignore; path_check_format respects PATH_SIZE. */
	char user_dir[PATH_SIZE];	/* The user directory. */

	if (FORCE_HOME && !path_is_subdir(doc_root, owner->pw_dir))
		error("document root %s not within %s's home directory.",
		      doc_root, owner->pw_name);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wformat-extra-args"
	rc = path_check_format(doc_root, user_dir, USER_DIR,
	                       owner->pw_dir, owner->pw_name);
#pragma GCC diagnostic pop
	switch (rc) {
	case OK:
		break;
	case ERR_RES:
		error("realpath %s: %m.", user_dir);
	case ERR_PRN:
		error("vsnprintf: %m.");
	case ERR_LEN:
		error("expanded user directory is too long.");
	case FAIL:
		error("document root %s is not %s's user directory.",
		      doc_root, owner->pw_name);
	default:
		/* Should be unreachable. */
		error("%d: path_check_format returned %u.", __LINE__, rc);
	}


	/*
	 * There should be no need to run hidden files or a files that reside
	 * in hidden directories. So if the path to the script does refer to
	 * a hidden file, this probably indicates a configuration error.
	 */

	/* script is guaranteed to be canonical. */
	if (strstr(script, "/."))
		error("path %s contains hidden files.", script);


	/*
	 * Although the set-user-ID and the set-group-ID on execute bits
	 * should be harmless, it would be odd for them to be set for a
	 * file that is owned by a regular user. So if either of them is
	 * set, this probably indicates a configuration error.
	 */

	if (script_stat.st_mode & S_ISUID)
		error("script %s's set-user-ID bit is set.", script);
	if (script_stat.st_mode & S_ISGID)
		error("script %s's set-group-ID bit is set.", script);


	/*
	 * If the webserver is (mis-)configured to allow visitors to change
	 * user files and a user (accidentally) set the permissions of a
	 * web-accessible script to be writable by the webserver or even
	 * the world, then visitors can run arbitrary code as that user.
	 *
	 * Also ensures that the document root and the CGI scripts are
	 * owned by the same UID, namely, owner->pw_uid.
	 */

	/* RATS: ignore; path_check_wexcl respects PATH_SIZE. */
	char script_cur[PATH_SIZE];	/* Sub-path of script path. */
	const char *base_dir;		/* Base directory. */

	if (FORCE_HOME)
		base_dir = owner->pw_dir;
	else
		base_dir = doc_root;

	rc = path_check_wexcl(owner->pw_uid, script, base_dir, script_cur);
	switch (rc) {
	case OK:
		break;
	case ERR_OPEN:
		error("open %s: %m.", script_cur);
	case ERR_CLOSE:
		error("close %s: %m.", script_cur);
	case ERR_STAT:
		error("stat %s: %m.", script_cur);
	case FAIL:
	        error("%s is writable by users other than %s.",
		      script_cur, owner->pw_name);
	default:
		/* Should be unreachable. */
		error("%d: path_check_wexcl returned %u.", __LINE__, rc);
	}


	/*
	 * Run the script.
	 */

	if (!file_is_exe(script_stat)) {
		/* RATS: ignore; script_get_inter respects PATH_SIZE. */
		char interp[PATH_SIZE];			/* Interpreter. */
		const struct pair db[] = HANDLERS;	/* Database. */

		rc = script_get_inter(db, script, interp);
		switch (rc) {
		case OK:
			break;
		case ERR_ILL:
			error("%s has no filename suffix.", script);
		case FAIL:
			error("no interpreter registered for %s.", script);
		default:
			/* Should be unreachable. */
			error("%d: script_get_inter returned %u.", __LINE__, rc);
		}

		assert(*interp);

		errno = 0;
		/* RATS: ignore; suCGI's whole point is to do this securely. */
		(void) execlp(interp, interp, script, NULL);

		/* If this point is reached, execution has failed. */
		error("execlp %s %s: %m.", interp, script);
	}

	errno = 0;
	/* RATS: ignore; suCGI's whole point is to do this securely. */
	(void) execl(script, script, NULL);

	/* If this point is reached, execution has failed. */
	error("execl %s: %m.", script);
}
