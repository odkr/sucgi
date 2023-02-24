/*
 * Test handler_lookup.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../config.h"
#include "../handler.h"
#include "../macros.h"
#include "../max.h"
#include "lib.h"


/*
 * Data types
 */

/* Mapping of arguments to return values. */
typedef struct {
    const char *const script;
    const char *const hdl;
    const Error ret;
} Args;


/*
 * Module variables
 */

/* Test cases. */
static const Args cases[] = {
    /* Simple errors. */
    {"file", NULL, ERR_NO_SUFFIX},
    {".", NULL, ERR_NO_SUFFIX},
    {".sh", NULL, ERR_NO_SUFFIX},
    {".py", NULL, ERR_NO_SUFFIX},
    {"file.null", NULL, ERR_BAD},
    {"file.empty", NULL, ERR_BAD},
    {"file.py", NULL, ERR_NO_MATCH},
    {"file.post", NULL, ERR_NO_MATCH},
    {"long.suffix-0123456789abcdef", NULL, ERR_LEN},

    /* Empty string shenanigans. */
    {" ", NULL, ERR_NO_SUFFIX},
    {". ", NULL, ERR_NO_SUFFIX},
    {".sh ", NULL, ERR_NO_SUFFIX},
    {".py ", NULL, ERR_NO_SUFFIX},
    {" .null", NULL, ERR_BAD},
    {" .empty", NULL, ERR_BAD},
    {" .py", NULL, ERR_NO_MATCH},
    {" .post", NULL, ERR_NO_MATCH},
    {" . ", NULL, ERR_NO_MATCH},

    /* Unicode shenanigans. */
    {"𝕗ïḻę", NULL, ERR_NO_SUFFIX},
    {".", NULL, ERR_NO_SUFFIX},
    {".sh", NULL, ERR_NO_SUFFIX},
    {".py", NULL, ERR_NO_SUFFIX},
    {"𝕗ïḻę.null", NULL, ERR_BAD},
    {"𝕗ïḻę.empty", NULL, ERR_BAD},
    {"𝕗ïḻę.py", NULL, ERR_NO_MATCH},
    {"𝕗ïḻę.post", NULL, ERR_NO_MATCH},
    {"𝕗ïḻę.suffix-0123456789abcdef", NULL, ERR_LEN},

    /* Simple test. */
    {"file.sh", "sh", OK},
    {"file.", "dot", OK}
};

/* Filename suffix-handler database for testing. */
static const Pair db[] = {
    {"", "unreachable"},
    {".", "dot"},
    {".sh", "sh"},
    {".null", NULL},
    {".empty", ""},
    {".pre", "pre"}
};


/*
 * Main
 */

int
main(void)
{
    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];         /* Shorthand. */
        const char *hdl;                    /* Handler. */
        Error ret;                          /* Return value. */

        warnx("checking (db, %s, -> %s) -> %u ...",
              args.script, args.hdl, args.ret);

        ret = handler_lookup(NELEMS(db), db, args.script, &hdl);

        if (args.ret != ret)
            errx(TEST_FAILED, "returned code %u", ret);

        if (ret == OK && strncmp(hdl, args.hdl, MAX_STR_LEN) != 0) {
            errx(TEST_FAILED, "returned script %s", args.hdl);
        }
    }

    warnx("all tests passed");
    return EXIT_SUCCESS;
}
