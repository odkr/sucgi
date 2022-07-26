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

#if !defined(_FORTIFY_SOURCE)
#define _FORTIFY_SOURCE 3
#endif

#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <limits.h>
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include "config.h"
#include "env.h"
#include "error.h"
#include "file.h"
#include "max.h"
#include "path.h"
#include "priv.h"
#include "script.h"
#include "str.h"
#include "types.h"
#include "userdir.h"


/*
 * Configuration for test builds
 */

#if defined(TESTING) && TESTING

#undef JAIL_DIR
#define JAIL_DIR "/"

#undef USER_DIR
#define USER_DIR "/tmp/check-sucgi/%s"

#undef MIN_UID
#define MIN_UID 500

#undef MAX_UID
#define MAX_UID 30000

#undef MIN_GID
#define MIN_GID 1

#undef MAX_GID
#define MAX_GID 30000

#undef HANDLERS
#define HANDLERS {{".sh", "sh"}, {NULL, NULL}}

#endif /* defined(TESTING) && TESTING */


/*
 * Verification of configuration
 */

#if !defined(JAIL_DIR)
#error JAIL_DIR is undefined.
#endif

#if !defined(USER_DIR)
#error USER_DIR is undefined.
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

#if MAX_UID > INT_MAX
#error MAX_UID is greater than INT_MAX.
#endif

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

#if MAX_GID > INT_MAX
#error MAX_GID is greater than INT_MAX.
#endif

#if !defined(HANDLERS)
#error HANDLERS is undefined.
#endif

#if !defined(PATH)
#error PATH is undefined.
#endif


/*
 * Constants
 */

/* suCGI version. */
#define VERSION "0"

/* Logging options. */
#if defined(LOG_PERROR) && LOG_PERROR
#define LOGGING_OPTS ( LOG_CONS | LOG_PERROR )
#else
#define LOGGING_OPTS ( LOG_CONS )
#endif


/*
 * Macros
 */

/* Raise a compiler error if COND is false. */
#define BUILD_BUG_ON(cond) ((void)sizeof(char[1 - 2*!!(cond)]))


/*
 * Globals
 */

/*
 * Secure environment variables. Array of shell wildcard patterns.
 *
 * Variables that match none of the given patterns are discarded.
 * See fnmatch(3) for the syntax. The array must be NULL-terminated.
 *
 * The list below has been adopted from:
 *      - RFC 3876
 *        <https://datatracker.ietf.org/doc/html/rfc3875>
 *      - Kira Matrejek, CGI Programming 101, chap. 3
 *        <http://www.cgi101.com/book/ch3/text.html>
 *      - Apache's suEXEC
 *        <https://github.com/apache/httpd/blob/trunk/support/suexec.c>
 *      - the Apache v2.4 documentation
 *        <https://httpd.apache.org/docs/2.4/expr.html>
 *      - the mod_ssl documentation
 *        <https://httpd.apache.org/docs/2.4/mod/mod_ssl.html>
 *
 * The list must include DOCUMENT_ROOT and PATH_TRANSLATED.
 * HOME, PATH, and USER_NAME are set regardless.
 *
 * There should be no need to adapt this list.
 */
static const char *const sec_vars[] = {
	"AUTH_TYPE",
	"CONTENT_LENGTH",
	"CONTENT_TYPE",
	"CONTEXT_DOCUMENT_ROOT",
	"CONTEXT_PREFIX",
	"DATE_GMT",
	"DATE_LOCAL",
	"DOCUMENT_NAME",
	"DOCUMENT_PATH_INFO",
	"DOCUMENT_ROOT",
	"DOCUMENT_URI",
	"GATEWAY_INTERFACE",
	"HANDLER",
	"HTTP_ACCEPT",
	"HTTP_COOKIE",
	"HTTP_FORWARDED",
	"HTTP_HOST",
	"HTTP_PROXY_CONNECTION",
	"HTTP_REFERER",
	"HTTP_USER_AGENT",
	"HTTP2",
	"HTTPS",
	"IS_SUBREQ",
	"IPV6",
	"LAST_MODIFIED",
	"PATH_INFO",
	"PATH_TRANSLATED",
	"QUERY_STRING",
	"QUERY_STRING_UNESCAPED",
	"REMOTE_ADDR",
	"REMOTE_HOST",
	"REMOTE_IDENT",
	"REMOTE_PORT",
	"REMOTE_USER",
	"REDIRECT_ERROR_NOTES",
	"REDIRECT_HANDLER",
	"REDIRECT_QUERY_STRING",
	"REDIRECT_REMOTE_USER",
	"REDIRECT_SCRIPT_FILENAME",
	"REDIRECT_STATUS REDIRECT_URL",
	"REQUEST_LOG_ID",
	"REQUEST_METHOD",
	"REQUEST_SCHEME",
	"REQUEST_STATUS",
	"REQUEST_URI",
	"SCRIPT_FILENAME",
	"SCRIPT_NAME",
	"SCRIPT_URI",
	"SCRIPT_URL",
	"SERVER_ADMIN",
	"SERVER_NAME",
	"SERVER_ADDR",
	"SERVER_PORT",
	"SERVER_PROTOCOL",
	"SERVER_SIGNATURE",
	"SERVER_SOFTWARE",
	"SSL_CIPHER",
	"SSL_CIPHER_EXPORT",
	"SSL_CIPHER_USEKEYSIZE",
	"SSL_CIPHER_ALGKEYSIZE",
	"SSL_CLIENT_M_VERSION",
	"SSL_CLIENT_M_SERIAL",
	"SSL_CLIENT_S_DN",
	"SSL_CLIENT_S_DN_*",
	"SSL_CLIENT_SAN_Email_*",
	"SSL_CLIENT_SAN_DNS_*",
	"SSL_CLIENT_SAN_OTHER_msUPN_*",
	"SSL_CLIENT_I_DN",
	"SSL_CLIENT_I_DN_*",
	"SSL_CLIENT_V_START",
	"SSL_CLIENT_V_END",
	"SSL_CLIENT_V_REMAIN",
	"SSL_CLIENT_A_SIG",
	"SSL_CLIENT_A_KEY",
	"SSL_CLIENT_CERT",
	"SSL_CLIENT_CERT_CHAIN_*",
	"SSL_CLIENT_CERT_RFC4523_CEA",
	"SSL_CLIENT_VERIFY",
	"SSL_COMPRESS_METHOD",
	"SSL_PROTOCOL",
	"SSL_SECURE_RENEG",
	"SSL_SERVER_M_VERSION",
	"SSL_SERVER_M_SERIAL",
	"SSL_SERVER_S_DN",
	"SSL_SERVER_SAN_Email_*",
	"SSL_SERVER_SAN_DNS_*",
	"SSL_SERVER_SAN_OTHER_dnsSRV_*",
	"SSL_SERVER_S_DN_*",
	"SSL_SERVER_I_DN",
	"SSL_SERVER_I_DN_*",
	"SSL_SERVER_V_START",
	"SSL_SERVER_V_END",
	"SSL_SERVER_A_SIG",
	"SSL_SERVER_A_KEY",
	"SSL_SERVER_CERT",
	"SSL_SESSION_ID",
	"SSL_SESSION_RESUMED",
	"SSL_SRP_USER",
	"SSL_SRP_USERINFO",
	"SSL_TLS_SNI",
	"SSL_VERSION_INTERFACE",
	"SSL_VERSION_LIBRARY",
	"UNIQUE_ID",
	"USER_NAME",
	"THE_REQUEST",
	"TIME_YEAR",
	"TIME_MON",
	"TIME_DAY",
	"TIME_HOUR",
	"TIME_MIN",
	"TIME_SEC",
	"TIME_WDAY",
	"TIME",
	"TZ",
	NULL	/* Array terminator. DO NOT REMOVE. */
};


/*
 * Prototypes
 */

/* Print help and exit. */
__attribute__((noreturn))
static void help(void);

/* Print build configuration and exit. */
__attribute__((noreturn))
static void config(void);

/* Print version and exit. */
__attribute__((noreturn))
static void version(void);

/* Print usage information to stderr and error out. */
__attribute__((noreturn))
static void usage(void);


/*
 * Functions
 */

static void
help(void)
{
	(void) puts(
"suCGI - run CGI scripts with the permissions of their owner\n\n"
"Usage:  sucgi\n"
"        sucgi [-C|-V|-h]\n\n"
"Options:\n"
"    -C  Print build configuration.\n"
"    -V  Print version and license.\n"
"    -h  Print this help screen."
	);
	exit(EXIT_SUCCESS);
}

static void
config(void)
{
	struct pair hdb[] = HANDLERS;

	(void) printf("JAIL_DIR=%s\n", JAIL_DIR);
	(void) printf("USER_DIR=%s\n", USER_DIR);

	(void) printf("MIN_UID=%d\n", MIN_UID);
	(void) printf("MAX_UID=%d\n", MAX_UID);
	(void) printf("MIN_GID=%d\n", MIN_GID);
	(void) printf("MAX_GID=%d\n", MAX_GID);

	(void) printf("HANDLERS=");
	for (const struct pair *h = hdb; h->key; ++h) {
		if (h != hdb)
			(void) printf(",");
		(void) printf("%s:%s", h->key, h->value);
	}
	(void) printf("\n");

	(void) printf("PATH=%s\n", PATH);
	(void) printf("UMASK=0%o\n", UMASK);

	(void) printf("MAX_NGROUPS=%u\n", MAX_NGROUPS);
	(void) printf("MAX_FNAME=%zu\n", MAX_FNAME);

	exit(EXIT_SUCCESS);
}

static void
version(void)
{
	(void) puts(
"suCGI v" VERSION "\n"
"Copyright 2022 Odin Kroeger.\n"
"Released under the GNU General Public License.\n"
"This programme comes with ABSOLUTELY NO WARRANTY."
	);
	exit(EXIT_SUCCESS);
}

static void
usage(void)
{
	(void) fputs("usage: sucgi [-c|-V|-h]\n", stderr);
	exit(EXIT_FAILURE);
}


/*
 * Main
 */

int
main(int argc, char **argv) {
	/*
	 * Check whether configuration strings are within bounds.
	 */

	BUILD_BUG_ON(sizeof(JAIL_DIR) <= 1);
	BUILD_BUG_ON(sizeof(JAIL_DIR) >= MAX_FNAME);
	BUILD_BUG_ON(sizeof(USER_DIR) <= 1);
	BUILD_BUG_ON(sizeof(USER_DIR) >= MAX_FNAME);
	BUILD_BUG_ON(sizeof(PATH) <= 1);
	BUILD_BUG_ON(sizeof(PATH) >= MAX_FNAME);


	/*
	 * Words of wisdom from the authors of suEXEC:
	 *
	 * > While cleaning the environment, the environment should be clean.
	 * > (E.g. malloc() may get the name of a file for writing debugging
	 * > info. Bad news if MALLOC_DEBUG_FILE is set to /etc/passwd.)
	 */

	char *const *vars;		/* Backup of the environment. */
	char *null;			/* NULL pointer. */

	vars = environ;
	null = NULL;
	environ = &null;

	assert(*environ == NULL);


	/*
	 * Set up logging.
	 */

	openlog("sucgi", LOGGING_OPTS, LOG_USER);
	atexit(closelog);


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
	 * Parse arguments.
	 */

	switch (argc) {
	case 1:
		break;
	case 2:
		/* Some getopt implementations are insecure. */
		if      (strncmp(argv[1], "-h", 3) == 0)
			help();
		else if (strncmp(argv[1], "-C", 3) == 0)
			config();
		else if (strncmp(argv[1], "-V", 3) == 0)
			version();
		__attribute__((fallthrough));
	default:
		usage();
	}


	/*
	 * Restore the environment variables needed by CGI scripts.
	 */

	/* RATS: ignore; env_restore respects MAX_VARNAME. */
	char var_name[MAX_VARNAME];	/* Name of last variable. */
	enum retval rc;			/* Return value. */

	rc = env_restore(vars, sec_vars, var_name);
	switch (rc) {
	case OK:
		break;
	case ERR_ENV:
		error("setenv: %m.");
	case ERR_CNV:
		error("encountered malformed environment variable.");
	case ERR_ILL:
		error("bad characters in variable name %s.", var_name);
	case ERR_LEN:
		error("$%s is too long.", var_name);
	default:
		/* Should be unreachable. */
		error("%d: env_restore returned %u.", __LINE__, rc);
	}


	/*
	 * Verify the jail directory.
	 */

	const char *jail_dir;		/* Jail directory. */

	errno = 0;
	/* RATS: ignore; the length of JAIL_DIR is checked above. */
	jail_dir = realpath(JAIL_DIR, NULL);
	if (!jail_dir)
		error("realpath %s: %m.", JAIL_DIR);

	assert(jail_dir);
	assert(*jail_dir != '\0');
        assert(strnlen(jail_dir, MAX_FNAME) < MAX_FNAME);


	/*
	 * Get the document root.
	 */

	const char *doc_root;		/* Document root. */
	int doc_fd;			/* -- " -- file descriptor. */

	rc = env_file_open(jail_dir, "DOCUMENT_ROOT", O_RDONLY | O_DIRECTORY,
	                   &doc_root, &doc_fd);
	switch (rc) {
	case OK:
		break;
	case ERR_ENV:
		error("getenv: %m.");
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
		error("%d: env_file_open returned %u.", __LINE__, rc);
	}

	assert(doc_root);
	assert(*doc_root != '\0');
	assert(doc_fd > -1);
	assert(strnlen(doc_root, MAX_FNAME) < MAX_FNAME);
	/* RATS: ignore; not a permission check. */
	assert(access(doc_root, F_OK) == 0);
	/* RATS: ignore; the length of doc_root is checked above. */
	assert(strncmp(realpath(doc_root, NULL), doc_root, MAX_FNAME) == 0);
	assert(doc_fd > -1);

	if (close(doc_fd) != 0)
		error("close %s: %m.", doc_root);


	/*
	 * Get the script.
	 */

	const char *script;		/* Path to script. */
	int script_fd;			/* Script file descriptor. */
	struct stat script_stat;	/* -- " -- filesystem metadata. */

	rc = env_file_open(doc_root, "PATH_TRANSLATED", O_RDONLY,
	                   &script, &script_fd);
   	switch (rc) {
	case OK:
		break;
	case ERR_ENV:
		error("getenv: %m.");
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
		error("%d: env_file_open returned %u.", __LINE__, rc);
   	}

	assert(script);
	assert(*script != '\0');
	assert(strnlen(script, MAX_FNAME) < MAX_FNAME);
	/* RATS: ignore; not a permission check. */
	assert(access(script, F_OK) == 0);
	/* RATS: ignore; the length of script is checked above. */
	assert(strncmp(realpath(script, NULL), script, MAX_FNAME) == 0);
	assert(script_fd > -1);

	errno = 0;
	if (fstat(script_fd, &script_stat) != 0)
		error("fstat %s: %m.", script);
	if ((script_stat.st_mode & S_IFREG) == 0)
		error("script %s is not a regular file.", script);


	/*
	 * Check if the script is owned by a regular user.
	 */

	struct passwd *owner;		/* Script owner. */

	errno = 0;
	owner = getpwuid(script_stat.st_uid);
	if (!owner) {
		if (errno == 0)
			error("script %s is owned by unallocated UID %llu.",
			      script, (unsigned long long) script_stat.st_uid);
		else
			error("getpwuid %llu: %m.",
			      (unsigned long long) script_stat.st_uid);
	}

	assert(owner->pw_uid == script_stat.st_uid);

	if (owner->pw_uid < MIN_UID || owner->pw_uid > MAX_UID)
		error("script %s is owned by privileged user %s.",
		      script, owner->pw_name);


	/*
	 * Check if the owner is a member of a privileged group.
	 */

	gid_t groups[MAX_NGROUPS];	/* Groups the owner is a member of. */
	int ngroups;			/* Number of those groups. */
	long maxgroups;			/* Maximum number of groups. */

/*
 * Some older getgrouplist implementations assume that GIDs are of the type
 * int, rather than type gid_t. However, gid_t is typically an alias for
 * unsigned int, so that both types are of the same size. Moreover, suCGI
 * requires that GIDs are smaller than INT_MAX. So this is a distinction
 * without a difference. The most straightforward way to deal with older
 * getgrouplist implementations is, therefore, to tell the compiler to
 * ignore the signedness incongruency.
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wpointer-sign"
	ngroups = MAX_NGROUPS;
	if (getgrouplist(owner->pw_name, owner->pw_gid, groups, &ngroups) < 0)
		error("user %s belongs to too many groups.", owner->pw_name);
#pragma GCC diagnostic pop

	for (int i = 0; i < ngroups; ++i) {
		gid_t gid = groups[i];

		if (gid < MIN_GID || gid > MAX_GID)
			error("user %s belongs to privileged group %llu.",
			      owner->pw_name, (unsigned long long) gid);
	}

	maxgroups = sysconf(_SC_NGROUPS_MAX);
	if (-1L < maxgroups && maxgroups < ngroups) {
		syslog(LOG_INFO, "user %s belongs to %d groups.",
		       owner->pw_name, ngroups);
		syslog(LOG_NOTICE, "can only set %ld groups for user %s.",
		       maxgroups, owner->pw_name);
		ngroups = (int) maxgroups;
	}


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
	if (setenv("PATH", PATH, true) != 0)
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
	umask(UMASK);


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

	char *user_dir;		/* Resolved user directory. */

	rc = userdir_resolve(USER_DIR, owner, &user_dir);

	switch (rc) {
	case OK:
		break;
	case ERR_RES:
		error("realpath %s: %m.", user_dir);
	case ERR_PRN:
		error("snprintf: %m.");
	case ERR_LEN:
		error("expanded user directory is too long.");
	default:
		/* Should be unreachable. */
		error("%d: path_check_format returned %u.", __LINE__, rc);
	}

	assert(user_dir);
	assert(*user_dir != '\0');
	assert(strnlen(user_dir, MAX_FNAME) < MAX_FNAME);
	/* RATS: ignore; not a permission check. */
	assert(access(user_dir, F_OK) == 0);
	/* RATS: ignore; the length of script is checked above. */
	assert(strncmp(realpath(user_dir, NULL), user_dir, MAX_FNAME) == 0);

	if (strncmp(doc_root, user_dir, MAX_FNAME) != 0)
		error("document root %s is not %s's user directory.",
		      doc_root, owner->pw_name);


	/*
	 * There should be no need to run hidden files or a files that reside
	 * in hidden directories. So if the path to the script does refer to
	 * a hidden file, this probably indicates a configuration error.
	 */

	/* script is guaranteed to be canonical. */
	if (strstr(script, "/.") != NULL)
		error("path %s contains hidden files.", script);


	/*
	 * Although the set-user-ID and the set-group-ID on execute bits
	 * should be harmless, it would be odd for them to be set for a
	 * file that is owned by a regular user. So if either of them is
	 * set, this probably indicates a configuration error.
	 */

	if ((script_stat.st_mode & S_ISUID) != 0)
		error("script %s's set-user-ID bit is set.", script);
	if ((script_stat.st_mode & S_ISGID) != 0)
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

	/* RATS: ignore; path_check_wexcl respects MAX_FNAME. */
	char script_cur[MAX_FNAME];	/* Sub-path of script path. */
	const char *base_dir;		/* Base directory. */

	if (*USER_DIR == '/')
		base_dir = doc_root;
	else
		base_dir = owner->pw_dir;

	rc = path_check_wexcl(owner->pw_uid, script, base_dir, script_cur);
	switch (rc) {
	case OK:
		break;
	case ERR_OPEN:
		error("open %s: %m.", script_cur);
	case ERR_CLOSE:
		error("close %s: %m.", script_cur);
	case ERR_STAT:
		error("fstat %s: %m.", script_cur);
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

	/* RATS: ignore; script_get_handler respects MAX_FNAME. */
	char handler[MAX_FNAME];		/* Interpreter. */
	const struct pair db[] = HANDLERS;	/* Database. */

	if (file_is_exe(script_stat)) {
		errno = 0;
		/* RATS: ignore; suCGI's whole point is to do this right. */
		(void) execl(script, script, NULL);

		/* If this point is reached, execution has failed. */
		error("execl %s: %m.", script);
	}

	rc = script_get_handler(db, script, handler);
	switch (rc) {
	case OK:
		break;
	case ERR_ILL:
		error("script %s has no filename suffix.", script);
	case FAIL:
		error("no handler for %s's filename suffix.", script);
	default:
		/* Should be unreachable. */
		error("%d: script_get_handler returned %u.", __LINE__, rc);
	}

	assert(*handler != '\0');

	errno = 0;
	/* RATS: ignore; suCGI's whole point is to do this right. */
	(void) execlp(handler, handler, script, NULL);

	/* If this point is reached, execution has failed. */
	error("execlp %s %s: %m.", handler, script);
}
