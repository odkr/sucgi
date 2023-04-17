/*
 * Test pathreal.
 *
 * Copyright 2023 Odin Kroeger.
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
#include <stdlib.h>
#include <stdio.h>

#include "../path.h"


int
main (int argc, char **argv)
{
    const char *fname;
    const char *real;
    Error ret;

    if (argc != 2) {
        fputs("usage: pathreal FNAME\n", stderr);
        return EXIT_FAILURE;
    }

    fname = argv[1];

    ret = pathreal(fname, &real);
    switch (ret) {
    case OK:
        break;
    case ERR_LEN:
        errx(EXIT_FAILURE, "filename too long");
    case ERR_SYS:
        err(EXIT_FAILURE, "realpath %s", fname);
    default:
        errx(EXIT_FAILURE, "returned %u", ret);
    }

    (void) printf("%s\n", real);

    return EXIT_SUCCESS;
}
