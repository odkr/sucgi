/*
 * Test path_check_in.
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
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../macros.h"
#include "../max.h"
#include "../path.h"
#include "result.h"


/*
 * Constants
 */

/* Maximum length of dynamically created filenames. */
#define FNAME_LEN 3U

/* Maximum length of dynamically created sub-directories. */
#define SUBDIR_LEN (FNAME_LEN * 2U + 1U)

/*
 * The number that tostr maps to a slash when given
 * ASCII shifted one byte to the left as digits.
 */
#define SLASH ('/' - 1U)

/*
 * The number that tostr maps to a dot when given
 * ASCII shifted one byte to the left as digits.
 */
#define DOT ('.' - 1U)


/*
 * Data types
 */

/* Mapping of constant arguments to a constant return value. */
typedef struct {
    const char *const basedir;
    const char *const fname;
    const Error ret;
} Args;


/*
 * Module variables
 */

/* A filename just within MAX_FNAME_LEN. */
static char longfname[MAX_FNAME_LEN] = {'\0'};

/* A filename that exceeds MAX_FNAME_LEN. */
static char hugefname[MAX_FNAME_LEN + 1U] = {'\0'};

/* Static test cases. */
static const Args cases[] = {
    /* Long filenames. */
    {"foo", longfname, ERR_BASEDIR},
    {longfname, "foo", ERR_BASEDIR},
    {"foo", hugefname, ERR_LEN},
    {hugefname, "foo", ERR_LEN},

    /* Absolute paths. */
    {"/", "/", ERR_BASEDIR},
    {"/", "/foo", OK},
    {"/foo", "/foo/bar", OK},
    {"/foo", "/bar", ERR_BASEDIR},
    {"/bar", "/foo", ERR_BASEDIR},
    {"/foo", "/foobar", ERR_BASEDIR},
    {"/", "foo", ERR_BASEDIR},
    {"/foo", "/", ERR_BASEDIR},
    {"/foo", "/foo", ERR_BASEDIR},

    /* Relative paths. */
    {"foo", "foo/bar", OK},
    {".", "foo/bar", OK},
    {"foo", "foo", ERR_BASEDIR},
    {"bar", "foo", ERR_BASEDIR},

    /* Leading dot. */
    {".", "./foo", OK},
    {"./foo", "./foo/bar", OK},
    {".", ".foo", OK},
    {"./bar", "./foo", ERR_BASEDIR},
    {"./foo", ".", ERR_BASEDIR},
    {"./foo", "./", ERR_BASEDIR},
    {"./foo", "./foo", ERR_BASEDIR},
    {".", ".", ERR_BASEDIR},
    {".f", ".foo", ERR_BASEDIR},
    {".foo", ".foo", ERR_BASEDIR},

    /* Realistc tests. */
    {"/home/jdoe", "/home/jdoe/public_html", OK},
    {"/srv/www", "/srv/www/jdoe", OK},
    {"/home/jdoe", "/srv/www/jdoe", ERR_BASEDIR},
    {"/srv/www", "/home/jdoe/public_html", ERR_BASEDIR},

    /* UTF-8. */
    {"/", "/𝒇ȫǭ", OK},
    {"/𝒇ȫǭ", "/𝒇ȫǭ/𝕓ắ𝚛", OK},
    {"/𝒇ȫǭ", "/𝕓ắ𝚛", ERR_BASEDIR},
    {"/𝕓ắ𝚛", "/𝒇ȫǭ", ERR_BASEDIR},
    {"/𝒇ȫǭ", "/𝒇ȫǭ𝕓ắ𝚛", ERR_BASEDIR},
    {"/", "𝒇ȫǭ", ERR_BASEDIR},
    {"/𝒇ȫǭ", "/", ERR_BASEDIR},
    {"/𝒇ȫǭ", "/𝒇ȫǭ", ERR_BASEDIR},
    {"𝒇ȫǭ", "𝒇ȫǭ/𝕓ắ𝚛", OK},
    {".", "𝒇ȫǭ/𝕓ắ𝚛", OK},
    {"𝒇ȫǭ", "𝒇ȫǭ", ERR_BASEDIR},
    {"𝕓ắ𝚛", "𝒇ȫǭ", ERR_BASEDIR},
    {".", "./𝒇ȫǭ", OK},
    {"./𝒇ȫǭ", "./𝒇ȫǭ/𝕓ắ𝚛", OK},
    {".", ".𝒇ȫǭ", OK},
    {"./𝕓ắ𝚛", "./𝒇ȫǭ", ERR_BASEDIR},
    {"./𝒇ȫǭ", ".", ERR_BASEDIR},
    {"./𝒇ȫǭ", "./", ERR_BASEDIR},
    {"./𝒇ȫǭ", "./𝒇ȫǭ", ERR_BASEDIR},
    {".", ".", ERR_BASEDIR},
    {".f", ".𝒇ȫǭ", ERR_BASEDIR},
    {".𝒇ȫǭ", ".𝒇ȫǭ", ERR_BASEDIR},
    {"/home/⒥𝑑𝓸𝖊", "/home/⒥𝑑𝓸𝖊/public_html", OK},
    {"/srv/www", "/srv/www/⒥𝑑𝓸𝖊", OK},
    {"/home/⒥𝑑𝓸𝖊", "/srv/www/⒥𝑑𝓸𝖊", ERR_BASEDIR},
    {"/srv/www", "/home/⒥𝑑𝓸𝖊/public_html", ERR_BASEDIR}
};


/*
 * Main
 */

int
main(void)
{
    int result = TEST_PASSED;

    (void) memset(longfname, 'x', sizeof(longfname));
    longfname[sizeof(longfname) - 1U] = '\0';

    (void) memset(hugefname, 'x', sizeof(hugefname));
    hugefname[sizeof(hugefname) - 1U] = '\0';

    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];
        Error ret;

        ret = path_check_in(args.basedir, args.fname);
        if (ret != args.ret) {
            warnx("checking (%s, %s) -> %u ... [!]",
                  args.basedir, args.fname, ret);
            result = TEST_FAILED;
        }
    }

    return result;
}
