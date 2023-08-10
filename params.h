/*
 * Configuration defaults.
 *
 * DO NOT EDIT THIS FILE. Instead override macros in config.h.
 * See config.h for extended descriptions of what each macro does.
 *
 * Copyright 2022 and 2023 Odin Kroeger.
 *
 * This file is part of suCGI.
 *
 * SuCGI is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * SuCGI is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General
 * Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with suCGI. If not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(PARAMS_H)
#define PARAMS_H

#include <sys/stat.h>
#include <limits.h>
#include <syslog.h>

#include "compat.h"
#include "config.h"


/*
 * Testing
 */

#if defined(TESTING) && TESTING

#undef USER_DIR
#define USER_DIR "/tmp/sucgi-check/%s"

#undef SAFE_ENV_VARS
#define SAFE_ENV_VARS { \
    "^GCOV_PREFIX$", \
    "^GCOV_PREFIX_STRIP$", \
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
}

#undef HANDLERS
#define HANDLERS {{".sh", "sh"}, {".empty", ""}}

#undef SYSLOG_FACILITY
#define SYSLOG_FACILITY LOG_USER

#undef SYSLOG_MASK
#define SYSLOG_MASK LOG_UPTO(LOG_DEBUG)

#undef SYSLOG_OPTS
#ifdef LOG_PERROR
#define SYSLOG_OPTS (LOG_CONS | LOG_PERROR)
#else
#define SYSLOG_OPTS LOG_CONS
#endif

#endif /* defined(TESTING) && TESTING */


/*
 * Defaults
 */

/* User directory. String. */
#if !defined(USER_DIR)
#define USER_DIR "public_html"
#endif

/* Lowest user ID that may be assigned to a regular user. */
#if !defined(START_UID)
#if defined(__illumos__) || defined(__sun)
#define START_UID 100
#elif defined(__MACH__)
#define START_UID 500
#else
#define START_UID 1000
#endif
#endif

#if START_UID < 1
#error START_UID must be greater than 0.
#endif

#if defined(MINUID) && START_UID < MINUID
#error START_UID must be greater than MINUID.
#endif


/* Highest user ID that may be assigned to a regular user. */
#if !defined(STOP_UID)
#if defined(__OpenBSD__)
#define STOP_UID 30000
#else
#define STOP_UID 60000
#endif
#endif

#if STOP_UID < START_UID
#error STOP_UID is smaller than START_UID.
#endif

#if STOP_UID > (INT_MAX - 1)
#error STOP_UID is greater than (INT_MAX - 1).
#endif

#if defined(MAXUID) && STOP_UID > MAXUID
#error STOP_UID is greater than MAXUID.
#endif


/* Lowest group ID that may be assigned to a regular group. */
#if !defined(START_GID)
#if defined(__MACH__) || defined(__illumos__) || defined(__sun)
#define START_GID 1
#elif defined(__NetBSD__)
#define START_GID 100
#else
#define START_GID START_UID
#endif
#endif

#if START_GID < 1
#error START_GID must be greater than 0.
#endif

#if defined(MINGID) && START_GID <= MINGID
#error START_GID must be greater than MINGID.
#endif


/* Highest group ID that may be assigned to a regular group. */
#if !defined(STOP_GID)
#define STOP_GID STOP_UID
#endif

#if STOP_GID < START_GID
#error STOP_GID is smaller than START_GID.
#endif

#if STOP_GID > (INT_MAX - 1)
#error STOP_GID is greater than (INT_MAX - 1).
#endif

#if defined(MAXGID) && STOP_GID > STOP_GID
#error STOP_GID is greater than MAXGID.
#endif

#if defined(NOGROUP) && NOGROUP > -1 && STOP_GID >= NOGROUP
#error STOP_GID is greater than or equal to NOGROUP.
#endif


/* Environment variables to keep. */
#if !defined(SAFE_ENV_VARS)
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
}
#endif


/* Handlers to run CGI scripts with. */
#if !defined(HANDLERS)
#define HANDLERS { \
    {".php", "php"}, \
}
#endif


/* Secure $PATH. */
#if !defined(PATH)
#define PATH "/usr/bin:/bin"
#endif


/* Secure file permission mask. */
#if !defined(UMASK)
#define UMASK (S_ISUID | S_ISGID | S_ISVTX | S_IRWXG | S_IRWXO)
#endif


/* Facility to log to. */
#if !defined(SYSLOG_FACILITY)
#define SYSLOG_FACILITY LOG_AUTH
#endif


/* Priorities to log. */
#if !defined(SYSLOG_MASK)
#define SYSLOG_MASK LOG_UPTO(LOG_ERR)
#endif


/* Syslog options. */
#if !defined(SYSLOG_OPTS)
#define SYSLOG_OPTS LOG_CONS
#endif


/*
 * Limits
 */

/*
 * An invalid GID to initialise memory with.
 */

/*FIXME: maybe just set NOGROUP, its non-standard anyway. */
#if defined(NOGROUP)
#define INVALID_GID NOGROUP
#else
#define INVALID_GID MAX_GID_VAL
#endif


/*
 * Maximum length for strings, including the null terminator. Unsigned integer.
 * Upper limit for all other character limits.
 */
#if !defined(MAX_STR_LEN)
#define MAX_STR_LEN 8192U
#endif

#if MAX_STR_LEN < 1
#error MAX_STR_LEN is non-positive.
#endif

#if MAX_STR_LEN > SHRT_MAX
#error MAX_STR_LEN is greater than SHRT_MAX.
#endif


/*
 * Maximum length for error message formats and error messages,
 * including the null terminator. Unsigned integer.
 */
#if !defined(MAX_ERRMSG_LEN)
#define MAX_ERRMSG_LEN 512U
#endif

#if MAX_ERRMSG_LEN < 256
#error MAX_ERRMSG_LEN is too short.
#endif

#if MAX_ERRMSG_LEN > MAX_STR_LEN
#error MAX_ERRMSG_LEN is greater than MAX_STR_LEN.
#endif

#if MAX_ERRMSG_LEN < 1
#error MAX_ERRMSG_LEN is non-positive.
#endif


/*
 * Maximum length for filenames, including the null terminator.
 * Unsigned integer. Longer filenames are rejected.
 */
#if !defined(MAX_FNAME_LEN)
#if defined(PATH_MAX) && PATH_MAX > -1
#if PATH_MAX < MAX_STR_LEN
#define MAX_FNAME_LEN PATH_MAX
#else
#define MAX_FNAME_LEN MAX_STR_LEN
#endif
#elif defined(MAXPATHLEN)
#if MAXPATHLEN < MAX_STR_LEN
#define MAX_FNAME_LEN MAXPATHLEN
#else
#define MAX_FNAME_LEN MAX_STR_LEN
#endif
#else
#define MAX_FNAME_LEN 4096
#endif
#endif

#if MAX_FNAME_LEN < 255
#error MAX_FNAME_LEN is smaller than 255.
#endif

#if MAX_FNAME_LEN < _POSIX_PATH_MAX
#error MAX_FNAME_LEN is smaller than _POSIX_PATH_MAX.
#endif

#if defined(MAXPATHLEN) && MAX_FNAME_LEN > MAXPATHLEN
#error MAX_FNAME_LEN is greater than MAXPATHLEN.
#endif

#if MAX_FNAME_LEN > MAX_STR_LEN
#error MAX_FNAME_LEN is greater than MAX_STR_LEN.
#endif

#if MAX_FNAME_LEN < 1
#error MAX_FNAME_LEN is non-positive.
#endif


/*
 * Maximum length for filename suffices, including the null terminator.
 * Unsigned integer. Filenames with longer suffices are rejected.
 */
#if !defined(MAX_SUFFIX_LEN)
#define MAX_SUFFIX_LEN 16U
#endif

#if MAX_SUFFIX_LEN < 8
#error MAX_SUFFIX_LEN is smaller than 8.
#endif

#if MAX_SUFFIX_LEN > MAX_STR_LEN
#error MAX_SUFFIX_LEN is greater than MAX_STR_LEN.
#endif

#if MAX_SUFFIX_LEN < 1
#error MAX_SUFFIX_LEN is non-positive.
#endif


/*
 * Maximum length for environment variables, including the null terminator.
 * Unsigned integer. Longer variables are ignored.
 */
#if !defined(MAX_VAR_LEN)
#define MAX_VAR_LEN MAX_FNAME_LEN
#endif

#if MAX_VAR_LEN < MAX_FNAME_LEN
#error MAX_VAR_LEN is smaller than MAX_FNAME_LEN.
#endif

#if MAX_VAR_LEN > MAX_STR_LEN
#error MAX_VAR_LEN is greater than MAX_STR_LEN.
#endif

#if MAX_VAR_LEN < 1
#error MAX_VAR_LEN is non-positive.
#endif


/*
 * Maximum length for environment variable names, including the null
 * terminator. Unsigned integer. Variables with longer names are ignored.
 */
#if !defined(MAX_VARNAME_LEN)
#define MAX_VARNAME_LEN 32U
#endif

#if MAX_VARNAME_LEN < 32
#error MAX_VARNAME_LEN is smaller than 32.
#endif

#if MAX_VARNAME_LEN > MAX_STR_LEN
#error MAX_VARNAME_LEN is greater than MAX_STR_LEN.
#endif


/*
 * Maximum number of groups a user may be a member of. Unsigned integer.
 * Users who are members of more groups are rejected.
 */
#if !defined(MAX_NGROUPS)
#define MAX_NGROUPS 256U
#endif

#if MAX_NGROUPS < 1
#error MAX_NGROUPS is non-positive.
#endif

#if MAX_NGROUPS > INT_MAX
#error MAX_NGROUPS is greater than INT_MAX.
#endif


/*
 * Maximum number of environment variables. Unsigned integer.
 * If the environment contains more variables, a run-time error is raised.
 */
#if !defined(MAX_NVARS)
#define MAX_NVARS 512U
#endif

#if MAX_NVARS < 1
#error MAX_NVARS is non-positive.
#endif

#if MAX_NVARS > SHRT_MAX
#error MAX_NVARS is greater than SHRT_MAX.
#endif


/*
 * System
 */

/* Name of the standard library. */
#if defined(__GLIBC__) || defined(__GNU_LIBRARY__)
#define LIBC "glibc"
#elif defined(__KLIBC__)
#define LIBC "klibc"
#elif defined(__UCLIBC__)
#define LIBC "uClibc"
#elif defined(__DragonFly__)
#define LIBC "DragonFly"
#elif defined(__FreeBSD__)
#define LIBC "FreeBSD"
#elif defined(__NetBSD__)
#define LIBC "NetBSD"
#elif defined(__OpenBSD__)
#define LIBC "OpenBSD"
#elif defined(__MACH__)
#define LIBC "Apple"
#endif

/* Name of configuration string for a conforming environment. */
#if defined(_CS_V7_ENV)
#define ENV_CONFSTR _CS_V7_ENV
#elif defined(_CS_V6_ENV)
#define ENV_CONFSTR _CS_V6_ENV
#endif


#endif /* !defined(PARAMS_H) */
