/*
 * Test pathchkloc.
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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../macros.h"
#include "../params.h"
#include "../path.h"
#include "check.h"


/*
 * Data types
 */

/* Mapping of constant arguments to a constant return value. */
typedef struct {
    const char *const basedir;
    const char *const fname;
    const Error retval;
    int signo;
} Args;


/*
 * Module variables
 */

/* A filename that exceeds MAX_FNAME_LEN. */
static char hugefname[MAX_FNAME_LEN + 1U] = {'\0'};

/* A filename just within MAX_FNAME_LEN. */
static char longfname[MAX_FNAME_LEN] = {'\0'};

/* Static test cases. */
static const Args cases[] = {
    /* Long filenames. */
    {"foo", longfname, ERR_BASEDIR, 0},
    {longfname, "foo", ERR_BASEDIR, 0},
    {"foo", hugefname, ERR_LEN, 0},
    {hugefname, "foo", ERR_LEN, 0},

    /* Absolute paths. */
    {"/", "/", ERR_BASEDIR, 0},
    {"/", "/foo", OK, 0},
    {"/foo", "/foo/bar", OK, 0},
    {"/foo", "/bar", ERR_BASEDIR, 0},
    {"/bar", "/foo", ERR_BASEDIR, 0},
    {"/foo", "/foobar", ERR_BASEDIR, 0},
    {"/", "foo", ERR_BASEDIR, 0},
    {"/foo", "/", ERR_BASEDIR, 0},
    {"/foo", "/foo", ERR_BASEDIR, 0},
    {"/foo", "/bar/foo", ERR_BASEDIR, 0},

    /* Relative paths. */
    {"foo", "foo/bar", OK, 0},
    {".", "foo/bar", OK, 0},
    {"foo", "foo", ERR_BASEDIR, 0},
    {"bar", "foo", ERR_BASEDIR, 0},
    {"foo", "bar/foo", ERR_BASEDIR, 0},

    /* Leading dot. */
    {".", "./foo", OK, 0},
    {"./foo", "./foo/bar", OK, 0},
    {".", ".foo", OK, 0},
    {"./bar", "./foo", ERR_BASEDIR, 0},
    {"./foo", ".", ERR_BASEDIR, 0},
    {"./foo", "./", ERR_BASEDIR, 0},
    {"./foo", "./foo", ERR_BASEDIR, 0},
    {".", ".", ERR_BASEDIR, 0},
    {".f", ".foo", ERR_BASEDIR, 0},
    {".foo", ".foo", ERR_BASEDIR, 0},
    {"./foo", "./bar/foo", ERR_BASEDIR, 0},

    /* Realistc tests. */
    {"/home/jdoe", "/home/jdoe/public_html", OK, 0},
    {"/srv/www", "/srv/www/jdoe", OK, 0},
    {"/home/jdoe", "/srv/www/jdoe", ERR_BASEDIR, 0},
    {"/srv/www", "/home/jdoe/public_html", ERR_BASEDIR, 0},

    /* UTF-8. */
    {"/", "/𝒇ȫǭ", OK, 0},
    {"/𝒇ȫǭ", "/𝒇ȫǭ/𝕓ắ𝚛", OK, 0},
    {"/𝒇ȫǭ", "/𝕓ắ𝚛", ERR_BASEDIR, 0},
    {"/𝕓ắ𝚛", "/𝒇ȫǭ", ERR_BASEDIR, 0},
    {"/𝒇ȫǭ", "/𝒇ȫǭ𝕓ắ𝚛", ERR_BASEDIR, 0},
    {"/", "𝒇ȫǭ", ERR_BASEDIR, 0},
    {"/𝒇ȫǭ", "/", ERR_BASEDIR, 0},
    {"/𝒇ȫǭ", "/𝒇ȫǭ", ERR_BASEDIR, 0},
    {"𝒇ȫǭ", "𝒇ȫǭ/𝕓ắ𝚛", OK, 0},
    {".", "𝒇ȫǭ/𝕓ắ𝚛", OK, 0},
    {"𝒇ȫǭ", "𝒇ȫǭ", ERR_BASEDIR, 0},
    {"𝕓ắ𝚛", "𝒇ȫǭ", ERR_BASEDIR, 0},
    {".", "./𝒇ȫǭ", OK, 0},
    {"./𝒇ȫǭ", "./𝒇ȫǭ/𝕓ắ𝚛", OK, 0},
    {".", ".𝒇ȫǭ", OK, 0},
    {"./𝕓ắ𝚛", "./𝒇ȫǭ", ERR_BASEDIR, 0},
    {"./𝒇ȫǭ", ".", ERR_BASEDIR, 0},
    {"./𝒇ȫǭ", "./", ERR_BASEDIR, 0},
    {"./𝒇ȫǭ", "./𝒇ȫǭ", ERR_BASEDIR, 0},
    {".", ".", ERR_BASEDIR, 0},
    {".f", ".𝒇ȫǭ", ERR_BASEDIR, 0},
    {".𝒇ȫǭ", ".𝒇ȫǭ", ERR_BASEDIR, 0},
    {"/home/⒥𝑑𝓸𝖊", "/home/⒥𝑑𝓸𝖊/public_html", OK, 0},
    {"/srv/www", "/srv/www/⒥𝑑𝓸𝖊", OK, 0},
    {"/home/⒥𝑑𝓸𝖊", "/srv/www/⒥𝑑𝓸𝖊", ERR_BASEDIR, 0},
    {"/srv/www", "/home/⒥𝑑𝓸𝖊/public_html", ERR_BASEDIR, 0}
};


/*
 * Main
 */

int
main(void)
{
    volatile int result = TEST_PASSED;

    checkinit();

    (void) memset(hugefname, 'x', sizeof(hugefname) - 1U);
    (void) memset(longfname, 'x', sizeof(longfname) - 1U);

    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];
        int jumpval;
        Error retval;

        jumpval = sigsetjmp(checkenv, true);

        if (jumpval == 0) {
            checking = 1;
            retval = pathchkloc(args.basedir, args.fname);
            checking = 0;

            if (retval != args.retval) {
                result = TEST_FAILED;
                warnx("checking (%s, %s) → %u [!]",
                      args.basedir, args.fname, retval);
            }
        }

        if (jumpval != args.signo) {
            result = TEST_FAILED;
            warnx("checking (%s, %s) ↑ %s [!]",
                  args.basedir, args.fname, strsignal(jumpval));
        }
    }

    return result;
}
