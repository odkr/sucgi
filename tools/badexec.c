/*
 * Run a programme with a custom/without argv[0].
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
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int
main (int argc, char **argv)
{
    int ch;             /* Option character. */

    while ((ch = getopt(argc, argv, "in:h")) != -1) {
        switch (ch) {
        case 'h':
            (void) puts(
"badexec - run a programme with a custom/without argv[0]\n\n"
"Usage:    badexec COMM [ARG ...]\n"
"          badexec -h\n\n"
"Operands:\n"
"    COMM  A command to run.\n"
"    ARG   An argument to COMM, starting with the 0th argument\n\n"
"Options:\n"
"    -h    Print this help screen.\n\n"
"Copyright 2023 Odin Kroeger.\n"
"Released under the GNU General Public License.\n"
"This programme comes with ABSOLUTELY NO WARRANTY."
            );
            return EXIT_SUCCESS;
        default:
            return EXIT_FAILURE;
        }
    }

    argc -= optind;
    argv += optind;

    if (argc < 1) {
        (void) fputs("usage: badexec COMM [ARG ...]\n", stderr);
        return EXIT_FAILURE;
    }

    errno = 0;
    (void) execvp(argv[0], &argv[1]);
    err(EXIT_FAILURE, "exec %s", argv[0]);
}
