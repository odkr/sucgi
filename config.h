/*
 * suCGI configuration.
 *
 * Copyright 2022 and 2023 Odin Kroeger.
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

/* This is config.h's include guard. Leave it alone. It's on duty. */
#if !defined(CONFIG_H)
#define CONFIG_H

#include <sys/stat.h>
#include <limits.h>
#include <syslog.h>


/*
 * Just in case your C is rusty:
 *
 *  - #define statements are terminated with a linefeed, not a semicolon.
 *  - If a #define statement should span multiple lines,
 *    the linefeed must be escaped with a backslash.
 *  - Strings are given in double quotes ("..."), not single quotes ('...').
 */


/*
 * The document root of user websites. Filename pattern.
 * CGI scripts are only run if they reside within their owner's document root.
 *
 * This constant mirrors Apache's UserDir directive, save for that a "%s"
 * printf conversion specifier is used instead of an "*". That is:
 *
 * (1) If USER_DIR is an absolute filename and contains a single "%s",
 *     that "%s" is replaced with the user's login name.
 *     For example: "/srv/web/%s/html" -> "/srv/web/jdoe/html".
 *     There must be at most one format specifier, and it must be "%s".
 *     printf's escaping rules apply.
 *
 * (2) If USER_DIR is an absolute filename but does *not* contain a "%s",
 *     a "/" and the user's login name are appended to the filename.
 *     For example: "/srv/web" -> "/srv/web/jdoe".
 *     printf's escaping rules do not apply.
 *
 * (3) If USER_DIR is a relative filename,
 *     the filename is prefixed with the user's home directory and a "/".
 *     For example: "public_html" -> "/home/jdoe/public_html.
 *     Format specifiers carry no special meaning and are used as they are.
 *     printf's escaping rules do not apply.
 */
/* #define USER_DIR "public_html" */


/*
 * Range of user IDs that may be assigned to non-system users. Integers.
 *
 * Most systems reserve a range of user IDs for non-system users, usually
 * that range starts at 100, 500, or 1,000 and ends at 30,000 or 60,000.
 * You MUST set [START_UID .. STOP_UID] to a subset of that range.
 *
 * Only CGI scripts owned by non-system users can be executed with suCGI.
 */
/* #define START_UID 1000 */
/* #define STOP_UID 30000 */


/*
 * Range of group IDs that may be assigned to non-system groups. Integers.
 *
 * Many systems reserve a range of group IDs for non-system groups, usually
 * that range starts at 100, 500, or 1,000 and ends at 30,000 or 60,000.
 * You MUST set [START_GID .. STOP_GID] to a subset of that range.
 *
 * On systems that do NOT reserve a range of group IDs for non-systems groups,
 * exclude as many system groups as feasible.
 *
 * Only CGI scripts owned by users who are NOT a member of
 * any system group can be executed with suCGI.
 */
/* #define START_GID 1000 */
/* #define STOP_GID 30000 */


/*
 * Environment variables to keep. Array of POSIX extended regular expressions.
 * Only variables the names of which match at least one expression are kept.
 *
 * The default list of permitted variables has been adopted from:
 *
 *     - RFC 3876 "The Common Gateway Interface (CGI) Version 1.1"
 *       <https://datatracker.ietf.org/doc/html/rfc3875>
 *     - Kira Matrejek, CGI Programming 101, chap. 3
 *       <http://www.cgi101.com/book/ch3/text.html>
 *     - Apache's suEXEC
 *       <https://github.com/apache/httpd/blob/trunk/support/suexec.c>
 *     - the Apache v2.4 variable documentation
 *       <https://httpd.apache.org/docs/2.4/expr.html#vars>
 *     - the Apache v2.4 mod_ssl documentation
 *       <https://httpd.apache.org/docs/2.4/mod/mod_ssl.html>
 *
 * The array MUST contain a regular expression that matches "PATH_TRANSLATED".
 *
 * Some variables are set by suCGI:
 *     - DOCUMENT_ROOT
 *     - HOME
 *     - PATH
 *     - SCRIPT_FILENAME
 *     - USER_NAME
 *
 * These variables will be set regardless of whether they match a pattern.
 *
 * There should be no need to adapt this list.
 */
/*
#define SAFE_ENV_VARS { \
    "^AUTH_TYPE$", \
    "^CONTENT_LENGTH$", \
    "^CONTENT_TYPE$", \
    "^CONTEXT_DOCUMENT_ROOT$", \
    "^CONTEXT_PREFIX$", \
    "^DATE_GMT$", \
    "^DATE_LOCAL$", \
    "^DOCUMENT_NAME$", \
    "^DOCUMENT_PATH_INFO$", \
    "^DOCUMENT_URI$", \
    "^GATEWAY_INTERFACE$", \
    "^HANDLER$", \
    "^HTTP_ACCEPT$", \
    "^HTTP_COOKIE$", \
    "^HTTP_FORWARDED$", \
    "^HTTP_HOST$", \
    "^HTTP_PROXY_CONNECTION$", \
    "^HTTP_REFERER$", \
    "^HTTP_USER_AGENT$", \
    "^HTTP2$", \
    "^HTTPS$", \
    "^IS_SUBREQ$", \
    "^IPV6$", \
    "^LAST_MODIFIED$", \
    "^PATH_INFO$", \
    "^PATH_TRANSLATED$", \
    "^QUERY_STRING$", \
    "^QUERY_STRING_UNESCAPED$", \
    "^REMOTE_ADDR$", \
    "^REMOTE_HOST$", \
    "^REMOTE_IDENT$", \
    "^REMOTE_PORT$", \
    "^REMOTE_USER$", \
    "^REDIRECT_ERROR_NOTES$", \
    "^REDIRECT_HANDLER$", \
    "^REDIRECT_QUERY_STRING$", \
    "^REDIRECT_REMOTE_USER$", \
    "^REDIRECT_SCRIPT_FILENAME$", \
    "^REDIRECT_STATUS$", \
    "^REDIRECT_URL$", \
    "^REQUEST_LOG_ID$", \
    "^REQUEST_METHOD$", \
    "^REQUEST_SCHEME$", \
    "^REQUEST_STATUS$", \
    "^REQUEST_URI$", \
    "^SCRIPT_FILENAME$", \
    "^SCRIPT_NAME$", \
    "^SCRIPT_URI$", \
    "^SCRIPT_URL$", \
    "^SERVER_ADMIN$", \
    "^SERVER_NAME$", \
    "^SERVER_ADDR$", \
    "^SERVER_PORT$", \
    "^SERVER_PROTOCOL$", \
    "^SERVER_SIGNATURE$", \
    "^SERVER_SOFTWARE$", \
    "^SSL_CIPHER$", \
    "^SSL_CIPHER_EXPORT$", \
    "^SSL_CIPHER_USEKEYSIZE$", \
    "^SSL_CIPHER_ALGKEYSIZE$", \
    "^SSL_CLIENT_M_VERSION$", \
    "^SSL_CLIENT_M_SERIAL$", \
    "^SSL_CLIENT_S_DN$", \
    "^SSL_CLIENT_S_DN_(C|ST|L|O|OU|CN|T|I|G|S|D|UID|Email)$", \
    "^SSL_CLIENT_S_DN_(C|ST|L|O|OU|CN|T|I|G|S|D|UID|Email)_(0|[1-9][0-9]*)$", \
    "^SSL_CLIENT_SAN_Email_(0|[1-9][0-9]*)$", \
    "^SSL_CLIENT_SAN_DNS_(0|[1-9][0-9]*)$", \
    "^SSL_CLIENT_SAN_OTHER_msUPN_(0|[1-9][0-9]*)$", \
    "^SSL_CLIENT_I_DN$", \
    "^SSL_CLIENT_I_DN_(C|ST|L|O|OU|CN|T|I|G|S|D|UID|Email)$", \
    "^SSL_CLIENT_I_DN_(C|ST|L|O|OU|CN|T|I|G|S|D|UID|Email)_(0|[1-9][0-9]*)$", \
    "^SSL_CLIENT_V_START$", \
    "^SSL_CLIENT_V_END$", \
    "^SSL_CLIENT_V_REMAIN$", \
    "^SSL_CLIENT_A_SIG$", \
    "^SSL_CLIENT_A_KEY$", \
    "^SSL_CLIENT_CERT$", \
    "^SSL_CLIENT_CERT_CHAIN_(0|[1-9][0-9]*)$", \
    "^SSL_CLIENT_CERT_RFC4523_CEA$", \
    "^SSL_CLIENT_VERIFY$", \
    "^SSL_COMPRESS_METHOD$", \
    "^SSL_PROTOCOL$", \
    "^SSL_SECURE_RENEG$", \
    "^SSL_SERVER_M_VERSION$", \
    "^SSL_SERVER_M_SERIAL$", \
    "^SSL_SERVER_S_DN_(C|ST|L|O|OU|CN|T|I|G|S|D|UID|Email)$", \
    "^SSL_SERVER_S_DN_(C|ST|L|O|OU|CN|T|I|G|S|D|UID|Email)_(0|[1-9][0-9]*)$", \
    "^SSL_SERVER_SAN_Email_(0|[1-9][0-9]*)$", \
    "^SSL_SERVER_SAN_DNS_(0|[1-9][0-9]*)$", \
    "^SSL_SERVER_SAN_OTHER_dnsSRV_(0|[1-9][0-9]*)$", \
    "^SSL_SERVER_I_DN_(C|ST|L|O|OU|CN|T|I|G|S|D|UID|Email)$", \
    "^SSL_SERVER_I_DN_(C|ST|L|O|OU|CN|T|I|G|S|D|UID|Email)_(0|[1-9][0-9]*)$", \
    "^SSL_SERVER_V_START$", \
    "^SSL_SERVER_V_END$", \
    "^SSL_SERVER_A_SIG$", \
    "^SSL_SERVER_A_KEY$", \
    "^SSL_SERVER_CERT$", \
    "^SSL_SESSION_ID$", \
    "^SSL_SESSION_RESUMED$", \
    "^SSL_SRP_USER$", \
    "^SSL_SRP_USERINFO$", \
    "^SSL_TLS_SNI$", \
    "^SSL_VERSION_INTERFACE$", \
    "^SSL_VERSION_LIBRARY$", \
    "^UNIQUE_ID$", \
    "^USER_NAME$", \
    "^THE_REQUEST$", \
    "^TIME_YEAR$", \
    "^TIME_MON$", \
    "^TIME_DAY$", \
    "^TIME_HOUR$", \
    "^TIME_MIN$", \
    "^TIME_SEC$", \
    "^TIME_WDAY$", \
    "^TIME$", \
    "^TZ$" \
*/


/*
 * Handlers to run CGI scripts with.
 * Array of filename suffix-handler pairs.
 *
 * The filename suffix must include the leading dot (e.g., ".php").
 * The handler is looked up in $PATH if its name is relative (e.g., "php").
 * Keep in mind that $PATH is set to PATH (see below).
 *
 * If no handler can be found, suCGI will execute the CGI script itself.
 */
/*
#define HANDLERS { \
    {".php", "php"}, \
}
*/


/*
 * Secure $PATH. String literal; colon-separated list of directories.
 */
/*
#define PATH "/usr/bin:/bin"
 */


/*
 * Secure file permission mask. Unsigned integer.
 */
/*
#define UMASK (S_ISUID | S_ISGID | S_ISVTX | S_IRWXG | S_IRWXO)
 */


/*
 * Facility to log to. Syslog constant.
 * See syslog(3) for details.
 */
/*
#define SYSLOG_FACILITY LOG_AUTH
 */


/*
 * Priorities to log. Syslog constant.
 * See syslog(3) for details.
 */
/*
#define SYSLOG_MASK LOG_UPTO(LOG_ERR)
 */


/*
 * Syslog options. Syslog constant.
 * See syslog(3) for details.
 */
/*
#define SYSLOG_OPTS (LOG_CONS | LOG_PERROR)
 */


#endif /* !defined(CONFIG_H) */
