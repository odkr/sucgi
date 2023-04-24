/*
 * Build configuration for testing.
 *
 * Copyright 2022 and 2023 Odin Kroeger
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

#if !defined(TESTING_H)
#define TESTING_H

#if defined(CHECK) && CHECK

#if !defined(PARAMS_H)
#error params.h must be included before testing.h.
#endif

#undef USER_DIR
#define USER_DIR "/tmp/sucgi-check/%s"

#undef ENV_PATTERNS
#define ENV_PATTERNS {                              \
    "^GCOV_PREFIX$",                                \
    "^GCOV_PREFIX_STRIP$",                          \
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

#undef HANDLERS
#define HANDLERS {{".sh", "sh"}, {".empty", ""}}

#undef LOGGING_MASK
#define LOGGING_MASK LOG_UPTO(LOG_DEBUG)

#undef LOGGING_OPTIONS
#ifdef LOG_PERROR
#define LOGGING_OPTIONS (LOG_CONS | LOG_PERROR)
#else
#define LOGGING_OPTIONS LOG_CONS
#endif

#endif /* defined(CHECK) && CHECK */
#endif /* !defined(TESTING_H) */
