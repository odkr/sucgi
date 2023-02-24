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

#include <assert.h>
#include <err.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../macros.h"
#include "../max.h"
#include "../path.h"
#include "lib.h"


/*
 * Constants
 */

/* Maximum length of dynamically created filenames. */
#define FNAME_LEN 3U

/* Maximum length of dynamically created sub-directories. */
#define SUBDIR_LEN (FNAME_LEN * 2U + 1U)

/*
 * The number that to_str maps to a slash when given
 * ASCII shifted one byte to the left as digits.
 */
#define SLASH ('/' - 1U)

/*
 * The number that to_str maps to a dot when given
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
 * Prototypes
 */

/* Test path_check_in. */
static void test(const char *basedir, const char *fname, Error ret);


/*
 * Module variables
 */

/* Static test cases. */
static const Args cases[] = {
    /* Absolute paths. */
    {"/", "/", ERR_NO_MATCH},
    {"/", "/foo", OK},
    {"/foo", "/foo/bar", OK},
    {"/foo", "/bar", ERR_NO_MATCH},
    {"/bar", "/foo", ERR_NO_MATCH},
    {"/foo", "/foobar", ERR_NO_MATCH},
    {"/", "foo", ERR_NO_MATCH},
    {"/foo", "/", ERR_NO_MATCH},
    {"/foo", "/foo", ERR_NO_MATCH},

    /* Relative paths. */
    {"foo", "foo/bar", OK},
    {".", "foo/bar", OK},
    {"foo", "foo", ERR_NO_MATCH},
    {"bar", "foo", ERR_NO_MATCH},

    /* Leading dot. */
    {".", "./foo", OK},
    {"./foo", "./foo/bar", OK},
    {".", ".foo", OK},
    {"./bar", "./foo", ERR_NO_MATCH},
    {"./foo", ".", ERR_NO_MATCH},
    {"./foo", "./", ERR_NO_MATCH},
    {"./foo", "./foo", ERR_NO_MATCH},
    {".", ".", ERR_NO_MATCH},
    {".f", ".foo", ERR_NO_MATCH},
    {".foo", ".foo", ERR_NO_MATCH},

    /* Realistc tests. */
    {"/home/jdoe", "/home/jdoe/public_html", OK},
    {"/srv/www", "/srv/www/jdoe", OK},
    {"/home/jdoe", "/srv/www/jdoe", ERR_NO_MATCH},
    {"/srv/www", "/home/jdoe/public_html", ERR_NO_MATCH},

    /* UTF-8. */
    {"/", "/𝒇ȫǭ", OK},
    {"/𝒇ȫǭ", "/𝒇ȫǭ/𝕓ắ𝚛", OK},
    {"/𝒇ȫǭ", "/𝕓ắ𝚛", ERR_NO_MATCH},
    {"/𝕓ắ𝚛", "/𝒇ȫǭ", ERR_NO_MATCH},
    {"/𝒇ȫǭ", "/𝒇ȫǭ𝕓ắ𝚛", ERR_NO_MATCH},
    {"/", "𝒇ȫǭ", ERR_NO_MATCH},
    {"/𝒇ȫǭ", "/", ERR_NO_MATCH},
    {"/𝒇ȫǭ", "/𝒇ȫǭ", ERR_NO_MATCH},
    {"𝒇ȫǭ", "𝒇ȫǭ/𝕓ắ𝚛", OK},
    {".", "𝒇ȫǭ/𝕓ắ𝚛", OK},
    {"𝒇ȫǭ", "𝒇ȫǭ", ERR_NO_MATCH},
    {"𝕓ắ𝚛", "𝒇ȫǭ", ERR_NO_MATCH},
    {".", "./𝒇ȫǭ", OK},
    {"./𝒇ȫǭ", "./𝒇ȫǭ/𝕓ắ𝚛", OK},
    {".", ".𝒇ȫǭ", OK},
    {"./𝕓ắ𝚛", "./𝒇ȫǭ", ERR_NO_MATCH},
    {"./𝒇ȫǭ", ".", ERR_NO_MATCH},
    {"./𝒇ȫǭ", "./", ERR_NO_MATCH},
    {"./𝒇ȫǭ", "./𝒇ȫǭ", ERR_NO_MATCH},
    {".", ".", ERR_NO_MATCH},
    {".f", ".𝒇ȫǭ", ERR_NO_MATCH},
    {".𝒇ȫǭ", ".𝒇ȫǭ", ERR_NO_MATCH},
    {"/home/⒥𝑑𝓸𝖊", "/home/⒥𝑑𝓸𝖊/public_html", OK},
    {"/srv/www", "/srv/www/⒥𝑑𝓸𝖊", OK},
    {"/home/⒥𝑑𝓸𝖊", "/srv/www/⒥𝑑𝓸𝖊", ERR_NO_MATCH},
    {"/srv/www", "/home/⒥𝑑𝓸𝖊/public_html", ERR_NO_MATCH}
};


/*
 * Functions
 */

static void
test(const char *const basedir, const char *const fname, const Error ret)
{
    Error retret;

    warnx("checking (%s, %s) -> %u ...", basedir, fname, ret);

    retret = path_check_in(basedir, fname);

    if (retret != ret) {
        errx(TEST_FAILED, "returned %u", retret);
    }
}


/*
 * Main
 */

int
main(void)
{
    char maxlen[MAX_FNAME_LEN];
    char errlen[MAX_FNAME_LEN + 1U];
    char ascii[127];
    char **fnames;
    char **plain;
    unsigned int nfnames;
    unsigned int nplain;

    FILL_STR('x', maxlen);
    FILL_STR('x', errlen);

    nfnames = (unsigned int) pow(sizeof(ascii), FNAME_LEN);
    nplain = 0;

    for (unsigned int i = 0; i < sizeof(ascii); ++i) {
        ascii[i] = (char) (i + 1);
    }

    fnames = calloc(nfnames, sizeof(*fnames));
    if(fnames == NULL) {
        err(TEST_ERROR, "calloc");
    }

    plain = calloc(nfnames, sizeof(*plain));
    if(plain == NULL) {
        err(TEST_ERROR, "calloc");
    }

    /* Basedir and filename just within bounds. */
    test(maxlen, maxlen, ERR_NO_MATCH);

    /* Basedir too long. */
    test(errlen, maxlen, ERR_LEN);

    /* Filename too long. */
    test(maxlen, errlen, ERR_LEN);

    /* Both too long. */
    test(errlen, errlen, ERR_LEN);

    /* Run static tests. */
    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];
        test(args.basedir, args.fname, args.ret);
    }

    /* Run dynamic tests. */
    warnx("generating %u filenames ...", nfnames);
    for (unsigned int i = 0; i < nfnames; ++i) {
        char *fname = calloc(FNAME_LEN + 1, sizeof(*fname));
        if(fname == NULL) {
            err(TEST_ERROR, "calloc");
        }

        assert(to_str(i, sizeof(ascii), ascii, FNAME_LEN + 1U, fname) == 0);

        fnames[i] = fname;
        if (i != DOT && strchr(fname, '/') == NULL) {
            plain[nplain++] = fname;
        }
    }

    /* Run dynamic tests. */
    warnx("checking dynamically created filenames ...");
    for (unsigned int i = 0; i < nfnames; ++i) {
        char *fname = fnames[i];
        bool isabs;
        bool isrel;
        Error ret;

        isabs = false;
        isrel = false;

        if (*fname == '/') {
            isabs = strncmp(fname, "/", 3) != 0;
        } else {
            isrel = strncmp(fname, ".", 3) != 0;
        }

        ret = path_check_in(fname, ".");
        if (ret != ERR_NO_MATCH) {
            errx(TEST_FAILED, "(%s, .) -> %u", fname, ret);
        }

        ret = path_check_in(fname, "/");
        if (ret != ERR_NO_MATCH) {
            errx(TEST_FAILED, "(%s, /) -> %u", fname, ret);
        }

        ret = path_check_in(".", fname);
        if (ret != ((isrel) ? OK : ERR_NO_MATCH)) {
            errx(TEST_FAILED, "(., %s) -> %u", fname, ret);
        }

        ret = path_check_in("/", fname);
        if (ret != ((isabs) ? OK : ERR_NO_MATCH)) {
            errx(TEST_FAILED, "(/, %s) -> %u", fname, ret);
        }
    }

    /* Testing for more than the first 512 paths takes too long. */
    for (unsigned int i = 0; i < 512; ++i) {
        if (i != SLASH) {
            for (unsigned int j = 0; j < nplain; ++j) {
                char *basedir = fnames[i];
                char *fname = plain[j];
                char subdir[SUBDIR_LEN + 1U];
                int n;
                Error ret;

                /* Sub-directories are always within their base directories. */
                n = snprintf(subdir, sizeof(subdir), "%s/%s", basedir, fname);
                if (n < 0) {
                    err(TEST_ERROR, "snprintf");
                }
                assert((size_t) n <= SUBDIR_LEN);

                ret = path_check_in(basedir, subdir);
                if (ret != OK) {
                    errx(TEST_FAILED, "(%s, %s) -> %u!", basedir, subdir, ret);
                }

                /*
                 * fname cannot be in basedir, because basedir
                 * cannot be "." and fname cannot contain '/'s.
                 */
                if (i != DOT) {
                    ret = path_check_in(basedir, fname);
                    if (ret != ERR_NO_MATCH) {
                        errx(TEST_FAILED, "(%s, %s) -> %u!",
                             basedir, fname, ret);
                    }
                }
            }
        }
    }

    warnx("all tests passed");
    return EXIT_SUCCESS;
}
