/*
 * Test env_is_name.
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

#include <assert.h>
#include <err.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../env.h"
#include "../macros.h"
#include "lib.h"


/*
 * Constants
 */

/* Characters valid in variable names. */
#define CHARS "0123456789_ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
                         "abcdefghijklmnopqrstuvwxyz"

/* Number of characters valid in a variable name. */
#define NCHARS (sizeof(CHARS) - 1U)


/*
 * Data types
 */

/* Mapping of a string to a return value. */
typedef struct {
    const char *const s;
    const bool ret;
} Args;


/*
 * Module variables
 */

/* Static test cases. */
static const Args cases[] = {
    /* Invalid names. */
    {"", false},
    {" foo", false},
    {"1foo", false},
    {"=foo", false},
    {"*", false},
    {"FOO ", false},
    {"$(foo)", false},
    {"`foo`", false},

    /* Valid names. */
    {"_", true},
    {"_f", true},
    {"_F", true},
    {"f", true},
    {"F", true},
    {"F_", true},
    {"f0", true},
    {"F0", true},

    /* Unicode shenanigans. */
    {"𝘧", false},
    {"𝘧oo", false},
    {"fȭo", false},
    {"foộ", false},
    {"𝘧ȭộ", false},

    /* Permitted variables. */
    {"AUTH_TYPE", true},
    {"CONTENT_LENGTH", true},
    {"CONTENT_TYPE", true},
    {"CONTEXT_DOCUMENT_ROOT", true},
    {"CONTEXT_PREFIX", true},
    {"DATE_GMT", true},
    {"DATE_LOCAL", true},
    {"DOCUMENT_NAME", true},
    {"DOCUMENT_PATH_INFO", true},
    {"DOCUMENT_ROOT", true},
    {"DOCUMENT_URI", true},
    {"GATEWAY_INTERFACE", true},
    {"HANDLER", true},
    {"HTTP_ACCEPT", true},
    {"HTTP_COOKIE", true},
    {"HTTP_FORWARDED", true},
    {"HTTP_HOST", true},
    {"HTTP_PROXY_CONNECTION", true},
    {"HTTP_REFERER", true},
    {"HTTP_USER_AGENT", true},
    {"HTTP2", true},
    {"HTTPS", true},
    {"IS_SUBREQ", true},
    {"IPV6", true},
    {"LAST_MODIFIED", true},
    {"PATH_INFO", true},
    {"PATH_TRANSLATED", true},
    {"POSIXLY_CORRECT", true},
    {"QUERY_STRING", true},
    {"QUERY_STRING_UNESCAPED", true},
    {"REMOTE_ADDR", true},
    {"REMOTE_HOST", true},
    {"REMOTE_IDENT", true},
    {"REMOTE_PORT", true},
    {"REMOTE_USER", true},
    {"REDIRECT_ERROR_NOTES", true},
    {"REDIRECT_HANDLER", true},
    {"REDIRECT_QUERY_STRING", true},
    {"REDIRECT_REMOTE_USER", true},
    {"REDIRECT_SCRIPT_FILENAME", true},
    {"REDIRECT_STATUS", true},
    {"REDIRECT_URL", true},
    {"REQUEST_LOG_ID", true},
    {"REQUEST_METHOD", true},
    {"REQUEST_SCHEME", true},
    {"REQUEST_STATUS", true},
    {"REQUEST_URI", true},
    {"SCRIPT_FILENAME", true},
    {"SCRIPT_NAME", true},
    {"SCRIPT_URI", true},
    {"SCRIPT_URL", true},
    {"SERVER_ADMIN", true},
    {"SERVER_NAME", true},
    {"SERVER_ADDR", true},
    {"SERVER_PORT", true},
    {"SERVER_PROTOCOL", true},
    {"SERVER_SIGNATURE", true},
    {"SERVER_SOFTWARE", true},
    {"SSL_CIPHER", true},
    {"SSL_CIPHER_EXPORT", true},
    {"SSL_CIPHER_USEKEYSIZE", true},
    {"SSL_CIPHER_ALGKEYSIZE", true},
    {"SSL_CLIENT_M_VERSION", true},
    {"SSL_CLIENT_M_SERIAL", true},
    {"SSL_CLIENT_S_DN", true},
    {"SSL_CLIENT_S_DN_CN", true},
    {"SSL_CLIENT_SAN_Email_0", true},
    {"SSL_CLIENT_SAN_DNS_0", true},
    {"SSL_CLIENT_SAN_OTHER_msUPN_0", true},
    {"SSL_CLIENT_I_DN", true},
    {"SSL_CLIENT_I_DN_CN", true},
    {"SSL_CLIENT_V_START", true},
    {"SSL_CLIENT_V_END", true},
    {"SSL_CLIENT_V_REMAIN", true},
    {"SSL_CLIENT_A_SIG", true},
    {"SSL_CLIENT_A_KEY", true},
    {"SSL_CLIENT_CERT", true},
    {"SSL_CLIENT_CERT_CHAIN_0", true},
    {"SSL_CLIENT_CERT_RFC4523_CEA", true},
    {"SSL_CLIENT_VERIFY", true},
    {"SSL_COMPRESS_METHOD", true},
    {"SSL_PROTOCOL", true},
    {"SSL_SECURE_RENEG", true},
    {"SSL_SERVER_M_VERSION", true},
    {"SSL_SERVER_M_SERIAL", true},
    {"SSL_SERVER_S_DN", true},
    {"SSL_SERVER_SAN_Email_0", true},
    {"SSL_SERVER_SAN_DNS_0", true},
    {"SSL_SERVER_SAN_OTHER_dnsSRV_0", true},
    {"SSL_SERVER_S_DN_CN", true},
    {"SSL_SERVER_I_DN", true},
    {"SSL_SERVER_I_DN_CN", true},
    {"SSL_SERVER_V_START", true},
    {"SSL_SERVER_V_END", true},
    {"SSL_SERVER_A_SIG", true},
    {"SSL_SERVER_A_KEY", true},
    {"SSL_SERVER_CERT", true},
    {"SSL_SESSION_ID", true},
    {"SSL_SESSION_RESUMED", true},
    {"SSL_SRP_USER", true},
    {"SSL_SRP_USERINFO", true},
    {"SSL_TLS_SNI", true},
    {"SSL_VERSION_INTERFACE", true},
    {"SSL_VERSION_LIBRARY", true},
    {"UNIQUE_ID", true},
    {"USER_NAME", true},
    {"THE_REQUEST", true},
    {"TIME_YEAR", true},
    {"TIME_MON", true},
    {"TIME_DAY", true},
    {"TIME_HOUR", true},
    {"TIME_MIN", true},
    {"TIME_SEC", true},
    {"TIME_WDAY", true},
    {"TIME", true},
    {"TMPDIR", true},
    {"TZ", true}
};


/*
 * Main
 */

int
main (void)
{
    char varname[6];

    (void) memset(varname, 0, sizeof(varname));

    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];
        bool ret;

        warnx("checking (%s) -> %d ...", args.s, args.ret);

        ret = env_is_name(args.s);
        if (ret != args.ret) {
            errx(TEST_FAILED, "returned %d", ret);
        }
    }

    warnx("checking dynamically created names ...");
    for (unsigned int i = 0; i < pow(NCHARS, sizeof(varname) - 1); ++i) {
/* tostr_ret is needed when debugging is enabled. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
        int tostr_ret;
#pragma GCC diagnostic pop

        tostr_ret = tostr(i, NCHARS, CHARS, sizeof(varname), varname);
        assert(tostr_ret == 0);

        if ('0' <= *varname && *varname <= '9') {
            if (env_is_name(varname)) {
                 errx(TEST_FAILED, "(%s) -> 1!", varname);
            }
        } else {
            if (!env_is_name(varname)) {
                 errx(TEST_FAILED, "(%s) -> 0!", varname);
            }
        }
    }

    warnx("all tests passed");
    return EXIT_SUCCESS;
}

