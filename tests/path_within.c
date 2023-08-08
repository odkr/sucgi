/*
 * Test path_chk_sub.
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

#define _BSD_SOURCE
#define _DARWIN_C_SOURCE
#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <err.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../macros.h"
#include "../params.h"
#include "../path.h"
#include "util/abort.h"
#include "util/types.h"


/*
 * Data types
 */

/* Mapping of constant arguments to a constant return value. */
typedef struct {
    const char *const basedir;
    const char *const fname;
    bool retval;
    int signal;
} PathWithinArgs;


/*
 * Main
 */

int
main(void)
{
    /* RATS: ignore; used safely. */
    char hugefname[MAX_FNAME_LEN + 1] = {0};

    /* RATS: ignore; used safely. */
    char longfname[MAX_FNAME_LEN] = {0};

/*
 * FIXME: Add assertions.
 */

    const PathWithinArgs cases[] = {
        /* Long filenames. */
        {"foo", longfname, false, 0},
        {longfname, "foo", false, 0},

        /* Absolute paths. */
        {"/", "/", false, 0},
        {"/", "/foo", true, 0},
        {"/foo", "/foo/bar", true, 0},
        {"/foo", "/bar", false, 0},
        {"/bar", "/foo", false, 0},
        {"/foo", "/foobar", false, 0},
        {"/", "foo", false, 0},
        {"/foo", "/", false, 0},
        {"/foo", "/foo", false, 0},
        {"/foo", "/bar/foo", false, 0},

        /* Relative paths. */
        {"foo", "foo/bar", true, 0},
        {".", "foo/bar", true, 0},
        {"foo", "foo", false, 0},
        {"bar", "foo", false, 0},
        {"foo", "bar/foo", false, 0},

        /* Leading dot. */
        {".", "./foo", true, 0},
        {"./foo", "./foo/bar", true, 0},
        {".", ".foo", true, 0},
        {"./bar", "./foo", false, 0},
        {"./foo", ".", false, 0},
        {"./foo", "./", false, 0},
        {"./foo", "./foo", false, 0},
        {".", ".", false, 0},
        {".f", ".foo", false, 0},
        {".foo", ".foo", false, 0},
        {"./foo", "./bar/foo", false, 0},

        /* Realistc tests. */
        {"/home/jdoe", "/home/jdoe/public_html", true, 0},
        {"/srv/www", "/srv/www/jdoe", true, 0},
        {"/home/jdoe", "/srv/www/jdoe", false, 0},
        {"/srv/www", "/home/jdoe/public_html", false, 0},

        /* UTF-8. */
        {"/", "/𝒇ȫǭ", true, 0},
        {"/𝒇ȫǭ", "/𝒇ȫǭ/𝕓ắ𝚛", true, 0},
        {"/𝒇ȫǭ", "/𝕓ắ𝚛", false, 0},
        {"/𝕓ắ𝚛", "/𝒇ȫǭ", false, 0},
        {"/𝒇ȫǭ", "/𝒇ȫǭ𝕓ắ𝚛", false, 0},
        {"/", "𝒇ȫǭ", false, 0},
        {"/𝒇ȫǭ", "/", false, 0},
        {"/𝒇ȫǭ", "/𝒇ȫǭ", false, 0},
        {"𝒇ȫǭ", "𝒇ȫǭ/𝕓ắ𝚛", true, 0},
        {".", "𝒇ȫǭ/𝕓ắ𝚛", true, 0},
        {"𝒇ȫǭ", "𝒇ȫǭ", false, 0},
        {"𝕓ắ𝚛", "𝒇ȫǭ", false, 0},
        {".", "./𝒇ȫǭ", true, 0},
        {"./𝒇ȫǭ", "./𝒇ȫǭ/𝕓ắ𝚛", true, 0},
        {".", ".𝒇ȫǭ", true, 0},
        {"./𝕓ắ𝚛", "./𝒇ȫǭ", false, 0},
        {"./𝒇ȫǭ", ".", false, 0},
        {"./𝒇ȫǭ", "./", false, 0},
        {"./𝒇ȫǭ", "./𝒇ȫǭ", false, 0},
        {".", ".", false, 0},
        {".f", ".𝒇ȫǭ", false, 0},
        {".𝒇ȫǭ", ".𝒇ȫǭ", false, 0},
        {"/home/⒥𝑑𝓸𝖊", "/home/⒥𝑑𝓸𝖊/public_html", true, 0},
        {"/srv/www", "/srv/www/⒥𝑑𝓸𝖊", true, 0},
        {"/home/⒥𝑑𝓸𝖊", "/srv/www/⒥𝑑𝓸𝖊", false, 0},
        {"/srv/www", "/home/⒥𝑑𝓸𝖊/public_html", false, 0}
    };

    volatile int result = PASS;

    (void) memset(hugefname, 'x', sizeof(hugefname) - 1U);
    (void) memset(longfname, 'x', sizeof(longfname) - 1U);

    for (volatile size_t i = 0; i < NELEMS(cases); ++i) {
        const PathWithinArgs args = cases[i];

        if (sigsetjmp(abort_env, 1) == 0) {
            size_t fnamelen;
            size_t basedirlen;
            bool retval;

            fnamelen = strnlen(args.fname, MAX_FNAME_LEN);
            basedirlen = strnlen(args.basedir, MAX_FNAME_LEN);

            if (args.signal != 0) {
                warnx("the next test should fail an assertion.");
            }

            (void) abort_catch(err);
            retval = path_within(fnamelen, args.fname,
                                 basedirlen, args.basedir);
            (void) abort_reset(err);


            if (retval != args.retval) {
                result = FAIL;
                warnx("%zu: checking (%s, %s) → %d [!]",
                      i, args.fname, args.basedir, retval);
            }
        }

        if (abort_signal != args.signal) {
            result = FAIL;
            warnx("%zu: checking (%s, %s) ↑ %s [!]",
                  i, args.fname, args.basedir, strsignal(abort_signal));
        }
    }

    return result;
}
