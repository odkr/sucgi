/*
 * Test envisname.
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

#define _BSD_SOURCE
#define _DARWIN_C_SOURCE
#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <err.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../env.h"
#include "../macros.h"
#include "util/check.h"


/*
 * Data types
 */

/* Mapping of a string to a return value. */
typedef struct {
    const char *const str;
    const bool retval;
    int signo;
} Args;


/*
 * Module variables
 */

#if !defined(NDEBUG)
/* A variable name that exceeds MAX_VARNAME_LEN. */
static char hugename[MAX_VARNAME_LEN + 1U] = {'\0'};
#endif

/* A variable name just within MAX_VARNAME_LEN. */
static char longname[MAX_VARNAME_LEN] = {'\0'};

/* Static test cases. */
static const Args cases[] = {
    /* Invalid arguments. */
#if !defined(NDEBUG)
    {hugename, true, SIGABRT},
#endif

    /* Long, but okay. */
    {longname, true, 0},

    /* Invalid names. */
    {"", false, 0},
    {" foo", false, 0},
    {"1foo", false, 0},
    {"=foo", false, 0},
    {"*", false, 0},
    {"FOO ", false, 0},
    {"$(foo)", false, 0},
    {"`foo`", false, 0},

    /* Valid names. */
    {"_", true, 0},
    {"_f", true, 0},
    {"_F", true, 0},
    {"f", true, 0},
    {"F", true, 0},
    {"F_", true, 0},
    {"f0", true, 0},
    {"F0", true, 0},

    /* Unicode shenanigans. */
    {"𝘧", false, 0},
    {"𝘧oo", false, 0},
    {"fȭo", false, 0},
    {"foộ", false, 0},
    {"𝘧ȭộ", false, 0},

    /* Permitted variables. */
    {"AUTH_TYPE", true, 0},
    {"CONTENT_LENGTH", true, 0},
    {"CONTENT_TYPE", true, 0},
    {"CONTEXT_DOCUMENT_ROOT", true, 0},
    {"CONTEXT_PREFIX", true, 0},
    {"DATE_GMT", true, 0},
    {"DATE_LOCAL", true, 0},
    {"DOCUMENT_NAME", true, 0},
    {"DOCUMENT_PATH_INFO", true, 0},
    {"DOCUMENT_ROOT", true, 0},
    {"DOCUMENT_URI", true, 0},
    {"GATEWAY_INTERFACE", true, 0},
    {"HANDLER", true, 0},
    {"HTTP_ACCEPT", true, 0},
    {"HTTP_COOKIE", true, 0},
    {"HTTP_FORWARDED", true, 0},
    {"HTTP_HOST", true, 0},
    {"HTTP_PROXY_CONNECTION", true, 0},
    {"HTTP_REFERER", true, 0},
    {"HTTP_USER_AGENT", true, 0},
    {"HTTP2", true, 0},
    {"HTTPS", true, 0},
    {"IS_SUBREQ", true, 0},
    {"IPV6", true, 0},
    {"LAST_MODIFIED", true, 0},
    {"PATH_INFO", true, 0},
    {"PATH_TRANSLATED", true, 0},
    {"POSIXLY_CORRECT", true, 0},
    {"QUERY_STRING", true, 0},
    {"QUERY_STRING_UNESCAPED", true, 0},
    {"REMOTE_ADDR", true, 0},
    {"REMOTE_HOST", true, 0},
    {"REMOTE_IDENT", true, 0},
    {"REMOTE_PORT", true, 0},
    {"REMOTE_USER", true, 0},
    {"REDIRECT_ERROR_NOTES", true, 0},
    {"REDIRECT_HANDLER", true, 0},
    {"REDIRECT_QUERY_STRING", true, 0},
    {"REDIRECT_REMOTE_USER", true, 0},
    {"REDIRECT_SCRIPT_FILENAME", true, 0},
    {"REDIRECT_STATUS", true, 0},
    {"REDIRECT_URL", true, 0},
    {"REQUEST_LOG_ID", true, 0},
    {"REQUEST_METHOD", true, 0},
    {"REQUEST_SCHEME", true, 0},
    {"REQUEST_STATUS", true, 0},
    {"REQUEST_URI", true, 0},
    {"SCRIPT_FILENAME", true, 0},
    {"SCRIPT_NAME", true, 0},
    {"SCRIPT_URI", true, 0},
    {"SCRIPT_URL", true, 0},
    {"SERVER_ADMIN", true, 0},
    {"SERVER_NAME", true, 0},
    {"SERVER_ADDR", true, 0},
    {"SERVER_PORT", true, 0},
    {"SERVER_PROTOCOL", true, 0},
    {"SERVER_SIGNATURE", true, 0},
    {"SERVER_SOFTWARE", true, 0},
    {"SSL_CIPHER", true, 0},
    {"SSL_CIPHER_EXPORT", true, 0},
    {"SSL_CIPHER_USEKEYSIZE", true, 0},
    {"SSL_CIPHER_ALGKEYSIZE", true, 0},
    {"SSL_CLIENT_M_VERSION", true, 0},
    {"SSL_CLIENT_M_SERIAL", true, 0},
    {"SSL_CLIENT_S_DN", true, 0},
    {"SSL_CLIENT_S_DN_CN", true, 0},
    {"SSL_CLIENT_SAN_Email_0", true, 0},
    {"SSL_CLIENT_SAN_DNS_0", true, 0},
    {"SSL_CLIENT_SAN_OTHER_msUPN_0", true, 0},
    {"SSL_CLIENT_I_DN", true, 0},
    {"SSL_CLIENT_I_DN_CN", true, 0},
    {"SSL_CLIENT_V_START", true, 0},
    {"SSL_CLIENT_V_END", true, 0},
    {"SSL_CLIENT_V_REMAIN", true, 0},
    {"SSL_CLIENT_A_SIG", true, 0},
    {"SSL_CLIENT_A_KEY", true, 0},
    {"SSL_CLIENT_CERT", true, 0},
    {"SSL_CLIENT_CERT_CHAIN_0", true, 0},
    {"SSL_CLIENT_CERT_RFC4523_CEA", true, 0},
    {"SSL_CLIENT_VERIFY", true, 0},
    {"SSL_COMPRESS_METHOD", true, 0},
    {"SSL_PROTOCOL", true, 0},
    {"SSL_SECURE_RENEG", true, 0},
    {"SSL_SERVER_M_VERSION", true, 0},
    {"SSL_SERVER_M_SERIAL", true, 0},
    {"SSL_SERVER_S_DN", true, 0},
    {"SSL_SERVER_SAN_Email_0", true, 0},
    {"SSL_SERVER_SAN_DNS_0", true, 0},
    {"SSL_SERVER_SAN_OTHER_dnsSRV_0", true, 0},
    {"SSL_SERVER_S_DN_CN", true, 0},
    {"SSL_SERVER_I_DN", true, 0},
    {"SSL_SERVER_I_DN_CN", true, 0},
    {"SSL_SERVER_V_START", true, 0},
    {"SSL_SERVER_V_END", true, 0},
    {"SSL_SERVER_A_SIG", true, 0},
    {"SSL_SERVER_A_KEY", true, 0},
    {"SSL_SERVER_CERT", true, 0},
    {"SSL_SESSION_ID", true, 0},
    {"SSL_SESSION_RESUMED", true, 0},
    {"SSL_SRP_USER", true, 0},
    {"SSL_SRP_USERINFO", true, 0},
    {"SSL_TLS_SNI", true, 0},
    {"SSL_VERSION_INTERFACE", true, 0},
    {"SSL_VERSION_LIBRARY", true, 0},
    {"UNIQUE_ID", true, 0},
    {"USER_NAME", true, 0},
    {"THE_REQUEST", true, 0},
    {"TIME_YEAR", true, 0},
    {"TIME_MON", true, 0},
    {"TIME_DAY", true, 0},
    {"TIME_HOUR", true, 0},
    {"TIME_MIN", true, 0},
    {"TIME_SEC", true, 0},
    {"TIME_WDAY", true, 0},
    {"TIME", true, 0},
    {"TMPDIR", true, 0},
    {"TZ", true, 0}
};


/*
 * Main
 */

int
main (void)
{
    volatile int result = TEST_PASSED;

    if (checkinit() != 0) {
        err(TEST_ERROR, "sigaction");
    }

#if !defined(NDEBUG)
    (void) memset(hugename, 'x', sizeof(hugename) - 1U);
#endif
    (void) memset(longname, 'x', sizeof(longname) - 1U);

    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];
        volatile bool retval;
        int jumpval;

        jumpval = sigsetjmp(checkenv, true);

        if (jumpval == 0) {
            checking = 1;
            retval = envisname(args.str);
            checking = 0;

            if (retval != args.retval) {
                result = TEST_FAILED;
                warnx("(%s) → %d [!]", args.str, retval);
            }
        }

        if (jumpval != args.signo) {
            result = TEST_FAILED;
            warnx("(%s) ↑ %s [!]", args.str, strsignal(jumpval));
        }
    }

    return result;
}

