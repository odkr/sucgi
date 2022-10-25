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
#include <unistd.h>

#include "env.h"
#include "error.h"
#include "file.h"
#include "path.h"
#include "str.h"


/*
 * Globals
 */

/*
 * Safe environment variables.
 *
 * Array of shell wildcard patterns. See fnmatch(3) for the syntax.
 * Variables that match none of the given patterns are discarded.
 * Patterns should be shorter than PATH_MAX bytes.
 * The array must be NULL-terminated.
 *
 * The list below has been adopted from:
 *      - RFC 3876
 *	  <https://datatracker.ietf.org/doc/html/rfc3875>
 *      - Kira Matrejek, CGI Programming 101, chap. 3
 *	  <http://www.cgi101.com/book/ch3/text.html>
 *      - Apache's suEXEC
 *	  <https://github.com/apache/httpd/blob/trunk/support/suexec.c>
 *      - the Apache v2.4 documentation
 *	  <https://httpd.apache.org/docs/2.4/expr.html>
 *      - the mod_ssl documentation
 *	  <https://httpd.apache.org/docs/2.4/mod/mod_ssl.html>
 *
 * The list must include DOCUMENT_ROOT and PATH_TRANSLATED.
 * HOME, PATH, and USER_NAME are set regardless.
 */
const char *const env_vars_safe[] = {
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
 * Functions
 */

enum error
env_clear(/* RATS: ignore; vars is bound-checked. */
	  const char *(*const vars)[MAX_ENV])
{
	static char *var;	/* First environment variable. */
	char **env;		/* Copy of the environ pointer. */

	var = NULL;
	env = environ;
	environ = &var;

	if (!vars) {
		return OK;
	}

	for (int n = 0; n < MAX_ENV; n++) {
		(*vars)[n] = env[n];
		if (!env[n]) {
			return OK;
		}
	}

	return ERR_LEN;
}

enum error
env_file_open(const char *const jail, const char *const varname,
	      const int flags, const char **const fname, int *const fd)
{
	const char *value;	/* Unchecked variable value. */
	const char *resolved;	/* Resolved filename. */
	char *unresolved;	/* Unresolved filename. */

	assert(*jail != '\0');
	assert(*varname != '\0');
	assert(strnlen(jail, MAX_STR) < MAX_STR);
	assert(access(jail, F_OK) == 0);
	/* RATS: ignore; this use of realpath should be safe. */
	assert(strncmp(jail, realpath(jail, NULL), MAX_STR) == 0);
	assert(flags != 0);
	assert(fname);
	assert(fd);

	errno = 0;
	unresolved = calloc(MAX_STR, sizeof(unresolved));
	if (!unresolved) {
		return ERR_CALLOC;
	}

	/* RATS: ignore; value is checked below. */
	errno = 0;
	value = getenv(varname);
	if (!value || *value == '\0') {
		if (errno == 0) {
			return ERR_NIL;
		} else {
			return ERR_GETENV;
		}
	}

	try(str_cp(MAX_STR - 1U, value, unresolved));
	*fname = unresolved;
 
	errno = 0;
	/* RATS: ignore; this use of realpath should be safe. */
	resolved = realpath(*fname, NULL);
	if (!resolved) {
		return ERR_REALPATH;
	}

	*fname = resolved;
	free(unresolved);

	if (strnlen(*fname, MAX_STR) >= MAX_STR) {
		return ERR_LEN;
	}
	if (!path_contains(jail, *fname)) {
		return ERR_ILL;
	}

	try(file_safe_open(*fname, flags, fd));

	return OK;
}

bool
env_is_name(const char *const name)
{
	/* Check if the name is the empty string. */
	if (*name == '\0') {
		return false;
	}

	/* Check if the name starts with a digit. */
	if (isdigit(*name)) {
		return false;
	}
	
	/* Check if the first non-valid character is the terminating NUL. */
	return (name[strspn(name, ENV_NAME_CHARS)] == '\0');
}

enum error
env_restore(const char *vars[], const char *const patterns[])
{
	assert(*vars);

	for (int i = 0; vars[i]; i++) {
		/* RATS: ignore; str_split respects MAX_STR. */
		char name[MAX_STR];	/* Variable name. */
		char *value;		/* Variable value. */
		size_t len;		/* Variable length. */

		len = strnlen(vars[i], MAX_STR);
		if (len == 0U) {
			return ERR_ILL;
		}
		if (len >= MAX_STR) {
			return ERR_LEN;
		}

		try(str_split(vars[i], "=", &name, &value));

		/* 
		 * patterns may contain wildcards,
		 * so the name has to be checked.
		 */
		if (!env_is_name(name)) {
			return ERR_ILL;
		}
		if (!value) {
			return ERR_ILL;
		}

		for (int j = 0; patterns[j]; j++) {
			if (fnmatch(patterns[j], name, 0) == 0) {
				errno = 0;
				if (setenv(name, value, true) != 0) {
					return ERR_SETENV;
				}

				break;
			}
		}
	}

	return OK;
}
