/*
 * Environment handling for suCGI.
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
#include <ctype.h>
#include <errno.h>
#include <fnmatch.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "env.h"
#include "err.h"
#include "file.h"
#include "path.h"
#include "str.h"


/*
 * Constants
 */

/* Characters allowed in environment variable names. */
#define VAR_NAME_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789" \
                       "abcdefghijklmnopqrstuvwxyz"

/*
 * Globals
 */

/*
 * Environment variables to keep.
 *
 * Array of shell wildcard patterns. See fnmatch(3) for the syntax.
 * Variables are only kept if their name matches one of these patterns.
 * The array must be NULL-terminated. $PATH is always set to SECURE_PATH.
 * Patterns should be shorter than PATH_MAX bytes.
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
 * There should be no need for changes.
 */
const char *const env_keep[] = {
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
 * Environment variables to toss even if they match a pattern in env_keep.
 *
 * Array of shell wildcard patterns. See fnmatch(3) for the syntax.
 * Variables are thrown out if their name matches one of these patterns.
 * The array must be NULL-terminated. $PATH is always set to SECURE_PATH.
 * Patterns should be shorter than PATH_MAX bytes.
 *
 * The list below has been adopted from Apache's suEXEC.
 * There should be no need for changes.
 */
const char *const env_toss[] = {
	"IFS",
	"HTTP_PROXY",
	"PATH",
	NULL	/* Array terminator. DO NOT REMOVE. */
};


/*
 * Functions
 */

enum error
env_clear(const char *(*const vars)[SC_ENV_MAX])
{
	static char *var;	/* First environment variable. */
	char **env;		/* Copy of the environ pointer. */

	var = NULL;
	env = environ;
	environ = &var;

	if (!vars) return SC_OK;
	for (int n = 0; n < SC_ENV_MAX; n++) {
		(*vars)[n] = env[n];
		if (!env[n]) return SC_OK;
	}

	return SC_ERR_ENV_MAX;
}

/* Note in docs: jail must be canonical, and fname may have been assigned a value;
 * this is useful for error messages, i.e., if it ain't realpath that fails,
 * then we know we are dealing with a canonical path. */
enum error
env_file_openat(const char *const jail, const char *const varname,
                const int flags, const char **const fname, int *const fd)
{
	const char *value;	/* Unchecked variable value. */

	assert(*jail != '\0');
	assert(*varname != '\0');
	assert(strcmp(jail, realpath(jail, NULL)) == 0);
	assert(flags != 0);
	assert(fname);
	assert(fd);

	/* RATS: ignore; value is checked below. */
	value = getenv(varname);
	if (!value || *value == '\0') return SC_ERR_ENV_NIL; 
	if (strnlen(value, STR_MAX) >= STR_MAX) return SC_ERR_ENV_LEN;
	*fname = realpath(value, NULL);
	if (!*fname) return SC_ERR_SYS;
	if (strnlen(*fname, STR_MAX) >= STR_MAX) return SC_ERR_ENV_LEN;
	if (!path_contains(jail, *fname)) return SC_ERR_ENV_MAL;
	try(file_safe_open(*fname, flags, fd));

	return SC_OK;
}

enum error
env_get_fname(const char *name, const mode_t ftype,
              const char **const fname, struct stat *const fstatus)
{
	const char *value;	/* Unchecked variable value. */

	assert(name[0] != '\0');
	assert(ftype != 0);

	/* RATS: ignore; value is checked below. */
	value = getenv(name);
	if (!value || value[0] == '\0') return SC_ERR_ENV_NIL;
	if (strnlen(value, STR_MAX) >= STR_MAX) return SC_ERR_ENV_LEN;
	*fname = realpath(value, NULL);
	if (!*fname) return SC_ERR_SYS;	
	if (strnlen(*fname, STR_MAX) >= STR_MAX) return SC_ERR_ENV_LEN;
	try(file_safe_stat(*fname, fstatus));
	if ((fstatus->st_mode & S_IFMT) != ftype) return SC_ERR_FTYPE;

	return SC_OK;
}

bool
env_name_valid(const char *const s)
{
	/* Check if the name is the empty string or starts with a digit. */
	if (*s == '\0' || isdigit(*s)) return false;
	/* Check if first non-valid character is NUL. */
	return (s[strspn(s, VAR_NAME_CHARS)] == '\0');
}

enum error
env_restore(const char *vars[],
            const char *const keep[],
	    const char *const toss[])
{
	assert(*vars);

	for (int i = 0; vars[i]; i++) {
		/* RATS: ignore; str_split respects STR_MAX. */
		char name[STR_MAX];	/* Variable name. */
		char *value;		/* Variable value. */
		size_t len;		/* Variable length. */

		len = strnlen(vars[i], STR_MAX);
		if (len >= STR_MAX) return SC_ERR_ENV_LEN;
		if (len == 0) return SC_ERR_ENV_MAL;

		try(str_split(vars[i], "=", &name, &value));
		/* keep may contain wildcards, so name has to be checked. */
		if (!env_name_valid(name)) return SC_ERR_ENV_MAL;
		if (!value) return SC_ERR_ENV_MAL;

		if (str_matchv(name, keep, 0) && !str_matchv(name, toss, 0)) {
			if (setenv(name, value, true) != 0) return SC_ERR_SYS;
		}
	}

	return SC_OK;
}
