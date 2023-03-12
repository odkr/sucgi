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
static const Args cases[] = {
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
    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];
        char dest[MAX_STR_LEN];
        Error ret;

        (void) memset(dest, '\0', sizeof(dest));

        warnx("checking (%zu, %s, -> %s) -> %u ...",
              args.n, args.src, args.dest, args.ret);

        ret = str_cp(args.n, args.src, dest);

        if (ret != args.ret) {
            errx(TEST_FAILED, "returned %u", ret);
        }

        if (ret == OK && strncmp(args.dest, dest, MAX_STR_LEN) != 0) {
            errx(TEST_FAILED, "copied '%s'", dest);
        }
    }

    warnx("all tests passed");
    return EXIT_SUCCESS;
}
