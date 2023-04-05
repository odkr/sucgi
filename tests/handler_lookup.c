/*
 * Test handlerfind.
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
#include "result.h"


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
    {"file", "<n/a>", ERR_SUFFIX},
    {".", "<n/a>", ERR_SUFFIX},
    {".sh", "<n/a>", ERR_SUFFIX},
    {".py", "<n/a>", ERR_SUFFIX},
    {"file.null", "<n/a>", ERR_BAD},
    {"file.empty", "<n/a>", ERR_BAD},
    {"file.py", "<n/a>", ERR_SEARCH},
    {"file.post", "<n/a>", ERR_SEARCH},
    {"long.suffix-0123456789abcdef", "<n/a>", ERR_LEN},

    /* Empty string shenanigans. */
    {" ", "<n/a>", ERR_SUFFIX},
    {". ", "<n/a>", ERR_SUFFIX},
    {".sh ", "<n/a>", ERR_SUFFIX},
    {".py ", "<n/a>", ERR_SUFFIX},
    {" .null", "<n/a>", ERR_BAD},
    {" .empty", "<n/a>", ERR_BAD},
    {" .py", "<n/a>", ERR_SEARCH},
    {" .post", "<n/a>", ERR_SEARCH},
    {" . ", "<n/a>", ERR_SEARCH},

    /* Unicode shenanigans. */
    {"𝕗ïḻę", "<n/a>", ERR_SUFFIX},
    {".", "<n/a>", ERR_SUFFIX},
    {".sh", "<n/a>", ERR_SUFFIX},
    {".py", "<n/a>", ERR_SUFFIX},
    {"𝕗ïḻę.null", "<n/a>", ERR_BAD},
    {"𝕗ïḻę.empty", "<n/a>", ERR_BAD},
    {"𝕗ïḻę.py", "<n/a>", ERR_SEARCH},
    {"𝕗ïḻę.post", "<n/a>", ERR_SEARCH},
    {"𝕗ïḻę.suffix-0123456789abcdef", "<n/a>", ERR_LEN},

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
    {".pre", "pre"},
    {".suffix-0123456789abcdef", "<unreachable>"}
};


/*
 * Main
 */

int
main(void)
{
    int result = TEST_PASSED;

    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];
        const char *hdl;
        Error ret;

        ret = handlerfind(NELEMS(db), db, args.script, &hdl);

        if (args.ret != ret) {
            warnx("(db, %s, -> %s) -> %u [!]",
                  args.script, args.hdl, ret);
            result = TEST_FAILED;
        }

        if (ret == OK && strncmp(hdl, args.hdl, MAX_STR_LEN) != 0) {
            warnx("(db, %s, -> %s [!]) -> %u",
                  args.script, hdl, args.ret);
            result = TEST_FAILED;
        }
    }

    return result;
}
