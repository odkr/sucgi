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

#if !defined(PARAMS_H)
#define PARAMS_H

#include <sys/stat.h>
#include <limits.h>
#include <syslog.h>

#include "compat.h"
#include "config.h"
#include "testing.h"
#include "pregs.h"


/* User directory. String. */
#if !defined(USER_DIR)
#define USER_DIR "public_html"
#endif

/* Lowest user ID that may be assigned to a regular user. */
#if !defined(MIN_UID)
#if defined(__illumos__) || defined(__sun) || defined(__minix)
#define MIN_UID 100
#elif defined(__MACH__)
#define MIN_UID 500
#else
#define MIN_UID 1000
#endif
#endif

#if MIN_UID < 1
#error MIN_UID must be greater than 0.
#endif

#if defined(MINUID) && MIN_UID < MINUID
#error MIN_UID must be greater than MINUID.
#endif


/* Highest user ID that may be assigned to a regular user. */
#if !defined(MAX_UID)
#if defined(__OpenBSD__)
#define MAX_UID 30000
#else
#define MAX_UID 60000
#endif
#endif

#if MAX_UID < MIN_UID
#error MAX_UID is smaller than MIN_UID.
#endif

#if MAX_UID > (INT_MAX - 1)
#error MAX_UID is greater than (INT_MAX - 1).
#endif

#if defined(MAXUID) && MAX_UID > MAXUID
#error MAX_UID is greater than MAXUID.
#endif


/* ID of the group with the lowest ID a regular user may be a member of. */
#if !defined(MIN_GID)
#if defined(__MACH__) || defined(__illumos__) || defined(__sun)
#define MIN_GID 1
#elif defined(__NetBSD__) || defined(__minix)
#define MIN_GID 100
#else
#define MIN_GID MIN_UID
#endif
#endif

#if MIN_GID < 1
#error MIN_GID must be greater than 0.
#endif

#if defined(MINGID) && MIN_GID <= MINGID
#error MIN_GID must be greater than MINGID.
#endif


/* ID of the group with the highest ID a regular user may be a member of. */
#if !defined(MAX_GID)
#define MAX_GID MAX_UID
#endif

#if MAX_GID < MIN_GID
#error MAX_GID is smaller than MIN_GID.
#endif

#if MAX_GID > (INT_MAX - 1)
#error MAX_GID is greater than (INT_MAX - 1).
#endif

#if defined(NOGROUP) && MAX_GID >= NOGROUP
#error MAX_GID is greater than or equal to NOGROUP.
#endif

#if defined(MAXGID) && MAX_GID > MAX_GID
#error MAX_GID is greater than MAXGID.
#endif


/* Environment variables to keep. */
#if !defined(ENV_PATTERNS)
#define ENV_PATTERNS {                              \
    "^AUTH_TYPE$",                                  \
    "^CONTENT_LENGTH$",                             \
    "^CONTENT_TYPE$",                               \
    "^CONTEXT_DOCUMENT_ROOT$",                      \
    "^CONTEXT_PREFIX$",                             \
    "^DATE_GMT$",                                   \
    "^DATE_LOCAL$",                                 \
    "^DOCUMENT_NAME$",                              \
    "^DOCUMENT_PATH_INFO$",                         \
    "^DOCUMENT_URI$",                               \
    "^GATEWAY_INTERFACE$",                          \
    "^HANDLER$",                                    \
    "^HTTP_ACCEPT$",                                \
    "^HTTP_COOKIE$",                                \
    "^HTTP_FORWARDED$",                             \
    "^HTTP_HOST$",                                  \
    "^HTTP_PROXY_CONNECTION$",                      \
    "^HTTP_REFERER$",                               \
    "^HTTP_USER_AGENT$",                            \
    "^HTTP2$",                                      \
    "^HTTPS$",                                      \
    "^IS_SUBREQ$",                                  \
    "^IPV6$",                                       \
    "^LAST_MODIFIED$",                              \
    "^PATH_INFO$",                                  \
    "^PATH_TRANSLATED$",                            \
    "^QUERY_STRING$",                               \
    "^QUERY_STRING_UNESCAPED$",                     \
    "^REMOTE_ADDR$",                                \
    "^REMOTE_HOST$",                                \
    "^REMOTE_IDENT$",                               \
    "^REMOTE_PORT$",                                \
    "^REMOTE_USER$",                                \
    "^REDIRECT_ERROR_NOTES$",                       \
    "^REDIRECT_HANDLER$",                           \
    "^REDIRECT_QUERY_STRING$",                      \
    "^REDIRECT_REMOTE_USER$",                       \
    "^REDIRECT_SCRIPT_FILENAME$",                   \
    "^REDIRECT_STATUS$",                            \
    "^REDIRECT_URL$",                               \
    "^REQUEST_LOG_ID$",                             \
    "^REQUEST_METHOD$",                             \
    "^REQUEST_SCHEME$",                             \
    "^REQUEST_STATUS$",                             \
    "^REQUEST_URI$",                                \
    "^SCRIPT_FILENAME$",                            \
    "^SCRIPT_NAME$",                                \
    "^SCRIPT_URI$",                                 \
    "^SCRIPT_URL$",                                 \
    "^SERVER_ADMIN$",                               \
    "^SERVER_NAME$",                                \
    "^SERVER_ADDR$",                                \
    "^SERVER_PORT$",                                \
    "^SERVER_PROTOCOL$",                            \
    "^SERVER_SIGNATURE$",                           \
    "^SERVER_SOFTWARE$",                            \
    "^SSL_CIPHER$",                                 \
    "^SSL_CIPHER_EXPORT$",                          \
    "^SSL_CIPHER_USEKEYSIZE$",                      \
    "^SSL_CIPHER_ALGKEYSIZE$",                      \
    "^SSL_CLIENT_M_VERSION$",                       \
    "^SSL_CLIENT_M_SERIAL$",                        \
    "^SSL_CLIENT_S_DN$",                            \
    "^SSL_CLIENT_S_DN_" PREG_X509 "$",              \
    "^SSL_CLIENT_S_DN_" PREG_X509 "_" PREG_N "$",   \
    "^SSL_CLIENT_SAN_Email_" PREG_N "$",            \
    "^SSL_CLIENT_SAN_DNS_" PREG_N "$",              \
    "^SSL_CLIENT_SAN_OTHER_msUPN_" PREG_N "$",      \
    "^SSL_CLIENT_I_DN$",                            \
    "^SSL_CLIENT_I_DN_" PREG_X509 "$",              \
    "^SSL_CLIENT_I_DN_" PREG_X509 "_" PREG_N "$",   \
    "^SSL_CLIENT_V_START$",                         \
    "^SSL_CLIENT_V_END$",                           \
    "^SSL_CLIENT_V_REMAIN$",                        \
    "^SSL_CLIENT_A_SIG$",                           \
    "^SSL_CLIENT_A_KEY$",                           \
    "^SSL_CLIENT_CERT$",                            \
    "^SSL_CLIENT_CERT_CHAIN_" PREG_N "$",           \
    "^SSL_CLIENT_CERT_RFC4523_CEA$",                \
    "^SSL_CLIENT_VERIFY$",                          \
    "^SSL_COMPRESS_METHOD$",                        \
    "^SSL_PROTOCOL$",                               \
    "^SSL_SECURE_RENEG$",                           \
    "^SSL_SERVER_M_VERSION$",                       \
    "^SSL_SERVER_M_SERIAL$",                        \
    "^SSL_SERVER_S_DN_" PREG_X509 "$",              \
    "^SSL_SERVER_S_DN_" PREG_X509 "_" PREG_N "$",   \
    "^SSL_SERVER_SAN_Email_" PREG_N "$",            \
    "^SSL_SERVER_SAN_DNS_" PREG_N "$",              \
    "^SSL_SERVER_SAN_OTHER_dnsSRV_" PREG_N "$",     \
    "^SSL_SERVER_I_DN_" PREG_X509 "$",              \
    "^SSL_SERVER_I_DN_" PREG_X509 "_" PREG_N "$",   \
    "^SSL_SERVER_V_START$",                         \
    "^SSL_SERVER_V_END$",                           \
    "^SSL_SERVER_A_SIG$",                           \
    "^SSL_SERVER_A_KEY$",                           \
    "^SSL_SERVER_CERT$",                            \
    "^SSL_SESSION_ID$",                             \
    "^SSL_SESSION_RESUMED$",                        \
    "^SSL_SRP_USER$",                               \
    "^SSL_SRP_USERINFO$",                           \
    "^SSL_TLS_SNI$",                                \
    "^SSL_VERSION_INTERFACE$",                      \
    "^SSL_VERSION_LIBRARY$",                        \
    "^UNIQUE_ID$",                                  \
    "^USER_NAME$",                                  \
    "^THE_REQUEST$",                                \
    "^TIME_YEAR$",                                  \
    "^TIME_MON$",                                   \
    "^TIME_DAY$",                                   \
    "^TIME_HOUR$",                                  \
    "^TIME_MIN$",                                   \
    "^TIME_SEC$",                                   \
    "^TIME_WDAY$",                                  \
    "^TIME$",                                       \
    "^TZ$"                                          \
}
#endif


/* Handlers to run CGI scripts with. */
#if !defined(HANDLERS)
#define HANDLERS {                                  \
    {".php", "php"},                                \
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
#if !defined(LOGGING_FACILITY)
#define LOGGING_FACILITY LOG_AUTH
#endif


/* Priorities to log. */
#if !defined(LOGGING_MASK)
#define LOGGING_MASK LOG_UPTO(LOG_ERR)
#endif

/* Syslog options. */
#if !defined(LOGGING_OPTIONS)
#if defined(LOG_PERROR)
#define LOGGING_OPTIONS (LOG_CONS | LOG_PERROR)
#else
#define LOGGING_OPTIONS LOG_CONS
#endif
#endif /* !defined(LOGGING_OPTIONS) */


#endif /* !defined(PARAMS_H) */
