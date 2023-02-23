dnl Defaults for constants.
dnl
dnl See config.h for details.
dnl
dnl Copyright 2022 and 2023 Odin Kroeger
dnl
dnl This file is part of suCGI.
dnl
dnl suCGI is free software: you can redistribute it and/or modify it under
dnl the terms of the GNU General Public License as published by the Free
dnl Software Foundation, either version 3 of the License, or (at your option)
dnl any later version.
dnl
dnl suCGI is distributed in the hope that it will be useful, but WITHOUT
dnl ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
dnl FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
dnl for more details.
dnl
dnl You should have received a copy of the GNU General Public License along
dnl with suCGI. If not, see <https://www.gnu.org/licenses>.
dnl
define(`__DEFINE_USER_DIR__', `dnl
#define USER_DIR "public_html"')dnl
dnl
define(`__DEFINE_MIN_UID__', `dnl
#define MIN_UID 1000')dnl
dnl
define(`__DEFINE_MAX_UID__', `dnl
#define MAX_UID 30000')dnl
dnl
define(`__DEFINE_MIN_GID__', `dnl
#define MIN_GID 1000')dnl
dnl
define(`__DEFINE_MAX_GID__', `dnl
#define MAX_GID 30000')dnl
dnl
define(`__DEFINE_ALLOW_GROUP__', `dnl
#define ALLOW_GROUP ""')dnl
dnl
define(`__DEFINE_DENY_GROUPS__', `dnl
#define DENY_GROUPS {                               \
    "root",                                         \
    "*admin",                                       \
    "*adm",                                         \
    "daemon",                                       \
    "nobody",                                       \
    "nogroup",                                      \
    "wheel",                                        \
    "_*"                                            \
}')dnl
dnl
define(`__DEFINE_ENV_PATTERNS__', `dnl
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
    "^POSIXLY_CORRECT$",                            \
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
    "^SSL_CLIENT_SAN_Email_(0|[1-9][0-9]*)$",       \
    "^SSL_CLIENT_SAN_DNS_(0|[1-9][0-9]*)$",         \
    "^SSL_CLIENT_SAN_OTHER_msUPN_(0|[1-9][0-9]*)$", \
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
    "^SSL_SERVER_SAN_Email_(0|[1-9][0-9]*)$",       \
    "^SSL_SERVER_SAN_DNS_(0|[1-9][0-9]*)$",         \
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
    "^TMPDIR$",                                     \
    "^TZ$"                                          \
}')dnl
dnl
define(`__DEFINE_HANDLERS__', `dnl
#define HANDLERS {                                  \
    {".php", "php"},                                \
}')dnl
dnl
define(`__DEFINE_PATH__', `dnl
#define PATH "/usr/bin:/bin"')dnl
dnl
define(`__DEFINE_UMASK__', `dnl
#define UMASK 07077u')dnl
dnl
define(`__DEFINE_LOGGING_FACILITY__', `dnl
#define LOGGING_FACILITY LOG_AUTH')dnl
dnl
define(`__DEFINE_LOGGING_LEVEL__', `dnl
#define LOGGING_LEVEL (                             \
    LOG_MASK(LOG_EMERG)   | LOG_MASK(LOG_ALERT) |   \
    LOG_MASK(LOG_CRIT)    | LOG_MASK(LOG_ERR)   |   \
    LOG_MASK(LOG_WARNING) | LOG_MASK(LOG_NOTICE)    \
)')dnl
dnl
define(`__DEFINE_LOGGING_OPTIONS__', `dnl
#if defined(LOG_PERROR)
#define LOGGING_OPTIONS ( LOG_CONS | LOG_PERROR )
#else /* !defined(LOG_PERROR) */
#define LOGGING_OPTIONS ( LOG_CONS )
#endif /* defined(LOG_PERROR) */')dnl
dnl
define(`__DEFINE_MAX_STR_LEN__', `dnl
#define MAX_STR_LEN 8192U')dnl
dnl
define(`__DEFINE_MAX_ERRMSG_LEN__', `dnl
#define MAX_ERRMSG_LEN 128U')dnl
dnl
define(`__DEFINE_MAX_FNAME_LEN__', `dnl
#if defined(PATH_MAX) && PATH_MAX > -1
#if PATH_MAX < MAX_STR_LEN
#define MAX_FNAME_LEN PATH_MAX
#else /* PATH_MAX >= MAX_STR_LEN */
#define MAX_FNAME_LEN MAX_STR_LEN
#endif /* PATH_MAX < MAX_STR_LEN */
#else /* !define(PATH_MAX) || PATH_MAX < 0 */
#define MAX_FNAME_LEN 1024
#endif /* defined(PATH_MAX) && PATH_MAX > -1 */')dnl
dnl
define(`__DEFINE_MAX_GRPNAME_LEN__', `dnl
#if defined(LOGIN_NAME_MAX) && LOGIN_NAME_MAX > -1
#if LOGIN_NAME_MAX < MAX_STR_LEN
#define MAX_GRPNAME_LEN LOGIN_NAME_MAX
#else /* LOGIN_NAME_MAX >= MAX_STR_LEN */
#define MAX_GRPNAME_LEN MAX_STR_LEN
#endif /* LOGIN_NAME_MAX < MAX_STR_LEN */
#else /* !define(LOGIN_NAME_MAX) || LOGIN_NAME_MAX < 0 */
#define MAX_GRPNAME_LEN 48U
#endif /* defined(LOGIN_NAME_MAX) && LOGIN_NAME_MAX > -1 */')dnl
dnl
define(`__DEFINE_MAX_SUFFIX_LEN__', `dnl
#define MAX_SUFFIX_LEN 8U')dnl
dnl
define(`__DEFINE_MAX_VAR_LEN__', `dnl
#define MAX_VAR_LEN MAX_FNAME_LEN')dnl
dnl
define(`__DEFINE_MAX_VARNAME_LEN__', `dnl
#define MAX_VARNAME_LEN 32U')dnl
dnl
define(`__DEFINE_MAX_NGROUPS__', `dnl
#define MAX_NGROUPS 128U')dnl
dnl
define(`__DEFINE_MAX_NVARS__', `dnl
#define MAX_NVARS 512U')dnl
