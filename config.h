/*
 * suCGI configuration.
 */

/* This is config.h's include guard. Leave it alone. It's on duty. */
#if !defined(CONFIG_H)
#define CONFIG_H


/*
 * CGI scripts are only executed if they are inside this path. String.
 *
 * Should correspond to the UserDir directive of your Apache configuation
 * (or the equivalent directive of the webserver you use).
 *
 * For example:
 *     - UserDir public_html -> "/home"
 *     - UserDir /srv/web -> "/srv/web"
 */
#define JAIL_DIR "/home"

/*
 * The document root of user websites. String.
 *
 * "%1$s" or "%s" is replaced with the home directory of the script's owner.
 * "%2$s" is replaced with their login name.
 *
 * Should match the UserDir directive of your Apache configuration
 * (or the equivalent directive of the webserver you use).
 *
 * For example:
 *     - UserDir public_html -> "%s/public_html"
 *     - UserDir /srv/web -> "/srv/web/%2$s"
 */
#define USER_DIR "%s/public_html"

/*
 * Must the user directory reside in the user's home directory? Boolean.
 */
#define FORCE_HOME true

/*
 * Smallest UID that may be assigned to a regular user. Unsigned integer.
 * On most systems, this will be 500 (e.g., macOS) or 1,000 (e.g, Debian).
 */
#define MIN_UID 1000U

/*
 * Largest UID that may be assigned to a regular user. Unsigned integer.
 * On most systems, this will be 60,000 (though some use 32,767 for nobody).
 */
#define MAX_UID 30000U

/*
 * Smallest GID that may be assigned to a regular user. Unsigned integer.
 * On most systems, this will be 500 or 1,000 (e.g, Debian).
 * However, some systems (e.g., macOS) make no such distinction.
 */
#define MIN_GID 1000U

/*
 * Largest GID that may be assigned to a regular user. Unsigned integer.
 * On most systems, this will be 30,000 or 60,000 (Debian).
 */
#define MAX_GID 30000U

/*
 * Secure environment variables. Array of shell wildcard patterns.
 *
 * Variables that match none of the given patterns are discarded.
 * See fnmatch(3) for the syntax. The array must be NULL-terminated.
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
 *
 * There should be no need to adapt this list.
 */
#define SEC_VARS {					\
	"AUTH_TYPE",					\
	"CONTENT_LENGTH",				\
	"CONTENT_TYPE",					\
	"CONTEXT_DOCUMENT_ROOT",			\
	"CONTEXT_PREFIX",				\
	"DATE_GMT",					\
	"DATE_LOCAL",					\
	"DOCUMENT_NAME",				\
	"DOCUMENT_PATH_INFO",				\
	"DOCUMENT_ROOT",				\
	"DOCUMENT_URI",					\
	"GATEWAY_INTERFACE",				\
	"HANDLER",					\
	"HTTP_ACCEPT",					\
	"HTTP_COOKIE",					\
	"HTTP_FORWARDED",				\
	"HTTP_HOST",					\
	"HTTP_PROXY_CONNECTION",			\
	"HTTP_REFERER",					\
	"HTTP_USER_AGENT",				\
	"HTTP2",					\
	"HTTPS",					\
	"IS_SUBREQ",					\
	"IPV6",						\
	"LAST_MODIFIED",				\
	"PATH_INFO",					\
	"PATH_TRANSLATED",				\
	"QUERY_STRING",					\
	"QUERY_STRING_UNESCAPED",			\
	"REMOTE_ADDR",					\
	"REMOTE_HOST",					\
	"REMOTE_IDENT",					\
	"REMOTE_PORT",					\
	"REMOTE_USER",					\
	"REDIRECT_ERROR_NOTES",				\
	"REDIRECT_HANDLER",				\
	"REDIRECT_QUERY_STRING",			\
	"REDIRECT_REMOTE_USER",				\
	"REDIRECT_SCRIPT_FILENAME",			\
	"REDIRECT_STATUS REDIRECT_URL",			\
	"REQUEST_LOG_ID",				\
	"REQUEST_METHOD",				\
	"REQUEST_SCHEME",				\
	"REQUEST_STATUS",				\
	"REQUEST_URI",					\
	"SCRIPT_FILENAME",				\
	"SCRIPT_NAME",					\
	"SCRIPT_URI",					\
	"SCRIPT_URL",					\
	"SERVER_ADMIN",					\
	"SERVER_NAME",					\
	"SERVER_ADDR",					\
	"SERVER_PORT",					\
	"SERVER_PROTOCOL",				\
	"SERVER_SIGNATURE",				\
	"SERVER_SOFTWARE",				\
	"SSL_CIPHER",					\
	"SSL_CIPHER_EXPORT",				\
	"SSL_CIPHER_USEKEYSIZE",			\
	"SSL_CIPHER_ALGKEYSIZE",			\
	"SSL_CLIENT_M_VERSION",				\
	"SSL_CLIENT_M_SERIAL",				\
	"SSL_CLIENT_S_DN",				\
	"SSL_CLIENT_S_DN_*",				\
	"SSL_CLIENT_SAN_Email_*",			\
	"SSL_CLIENT_SAN_DNS_*",				\
	"SSL_CLIENT_SAN_OTHER_msUPN_*",			\
	"SSL_CLIENT_I_DN",				\
	"SSL_CLIENT_I_DN_*",				\
	"SSL_CLIENT_V_START",				\
	"SSL_CLIENT_V_END",				\
	"SSL_CLIENT_V_REMAIN",				\
	"SSL_CLIENT_A_SIG",				\
	"SSL_CLIENT_A_KEY",				\
	"SSL_CLIENT_CERT",				\
	"SSL_CLIENT_CERT_CHAIN_*",			\
	"SSL_CLIENT_CERT_RFC4523_CEA",			\
	"SSL_CLIENT_VERIFY",				\
	"SSL_COMPRESS_METHOD",				\
	"SSL_PROTOCOL",					\
	"SSL_SECURE_RENEG",				\
	"SSL_SERVER_M_VERSION",				\
	"SSL_SERVER_M_SERIAL",				\
	"SSL_SERVER_S_DN",				\
	"SSL_SERVER_SAN_Email_*",			\
	"SSL_SERVER_SAN_DNS_*",				\
	"SSL_SERVER_SAN_OTHER_dnsSRV_*",		\
	"SSL_SERVER_S_DN_*",				\
	"SSL_SERVER_I_DN",				\
	"SSL_SERVER_I_DN_*",				\
	"SSL_SERVER_V_START",				\
	"SSL_SERVER_V_END",				\
	"SSL_SERVER_A_SIG",				\
	"SSL_SERVER_A_KEY",				\
	"SSL_SERVER_CERT",				\
	"SSL_SESSION_ID",				\
	"SSL_SESSION_RESUMED",				\
	"SSL_SRP_USER",					\
	"SSL_SRP_USERINFO",				\
	"SSL_TLS_SNI",					\
	"SSL_VERSION_INTERFACE",			\
	"SSL_VERSION_LIBRARY",				\
	"UNIQUE_ID",					\
	"USER_NAME",					\
	"THE_REQUEST",					\
	"TIME_YEAR",					\
	"TIME_MON",					\
	"TIME_DAY",					\
	"TIME_HOUR",					\
	"TIME_MIN",					\
	"TIME_SEC",					\
	"TIME_WDAY",					\
	"TIME",						\
	"TZ",						\
	NULL	/* Array terminator. DO NOT REMOVE. */	\
}

/*
 * Maximum number of environment variables. Unsigned integer.
 * suCGI aborts if the environment contains more variables.
 * Setting this number to anything lower than 128 may break CGI scripts.
 */
#define MAX_NVARS 256U

/*
 * Handlers to run CGI scripts with if their executable bit is unset.
 * Array of filename suffix-interpreter pairs.
 * 
 * The filename suffix must be given including the leading dot (e.g., ".php").
 * The interpreter is looked up in $PATH if its name is relative (e.g., "php"),
 * but keep in mind that $PATH is set to SEC_PATH (see below).
 *
 * The array must be terminated with a pair of NULLs.
 */
#define HANDLERS	{					\
	{".php", "php"},					\
	{NULL, NULL}	/* Array terminator. DO NOT REMOVE. */	\
}

/*
 * A secure $PATH.
 */
#define SEC_PATH "/usr/bin:/bin"

/* 
 * A secure file permission mask. The leading "0" is significant.
 * Added to the current umask.
 */
#define UMASK 077U


#endif /* !defined(CONFIG_H). */
