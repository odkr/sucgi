/*
 * Test str_cp.
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
#include <stdlib.h>
#include <string.h>

#include "../macros.h"
#include "../max.h"
#include "../str.h"
#include "lib.h"


/*
 * Constants
 */

/* Maximum length of dynamically created strings. */
#define STR_LEN 4U


/*
 * Data types
 */

/* Mapping of arguments to return values. */
typedef struct {
    const size_t n;
    const char *const src;
    const char *const dest;
    const Error ret;
} Args;


/*
 * Module variables
 */

/* Tests. */
static const Args tests[] = {
    /* Simple test. */
    {MAX_STR_LEN - 1U, "foo", "foo", OK},

    /* Almost truncated. */
    {1, "x", "x", OK},

    /* Truncation. */
    {3, "abcd", "abc", ERR_LEN},

    /* Truncate to 0. */
    {0, "foo", "", ERR_LEN},

    /* Empty strings. */
    {MAX_STR_LEN - 1U, "", "", OK},
    {1, "", "", OK},
    {0, "", "", OK}
};


/*
 * Main
 */

int
main (void) {
    char ascii[127];

    for (unsigned int i = 0; i < sizeof(ascii); ++i) {
        ascii[i] = (char) (i + 1);
    }

    for (size_t i = 0; i < NELEMS(tests); ++i) {
        const Args t = tests[i];
        char dest[MAX_STR_LEN];        /* RATS: ignore */
        Error ret;

        (void) memset(dest, '\0', sizeof(dest));

        warnx("checking (%zu, %s, -> %s) -> %u ...",
              t.n, t.src, t.dest, t.ret);

        ret = str_cp(t.n, t.src, dest);

        if (ret != t.ret) {
            errx(TEST_FAILED, "returned %u", ret);
        }
        if (!(t.dest == dest || strncmp(t.dest, dest, MAX_STR_LEN) == 0)) {
            errx(TEST_FAILED, "got copy '%s'", dest);
        }
    }

    warnx("checking dynamically created strings ...");
    for (unsigned int i = 0; i < pow(sizeof(ascii), STR_LEN); ++i) {
/* rc is needed when debugging is enabled. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
        int rc;
#pragma GCC diagnostic pop
        char src[MAX_STR_LEN];
        char dest[MAX_STR_LEN];
        Error ret;

        (void) memset(src, '\0', sizeof(src));
        (void) memset(dest, '\0', sizeof(dest));

        rc = to_str(i, sizeof(ascii), ascii, sizeof(src), src);
        assert(rc == 0);

        ret = str_cp(sizeof(dest) - 1, src, dest);

        if (ret != OK) {
            errx(TEST_FAILED, "(%s, -> %s) -> %u!", src, dest, ret);
        }
        if (strncmp(src, dest, MAX_STR_LEN) != 0) {
            errx(TEST_FAILED, "(%s, -> %s!) -> %u", src, dest, ret);
        }
    }

    warnx("all tests passed");
    return EXIT_SUCCESS;
}
