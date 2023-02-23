/*
 * Run a command as a given user.
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
#define _GNU_SOURCE

#include <err.h>
#include <errno.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


int
main (int argc, char **argv)
{
    struct passwd *pwd;     /* The user. */
    char **comm;            /* The command. */
    bool real;              /* Set only the real user and group IDs? */
    int ch;                 /* An option character. */

    real = false;

    while ((ch = getopt(argc, argv, "hr")) != -1) {
        switch (ch) {
        case 'h':
            (void) puts(
"runas - run a command as a given user\n\n"
"Usage:       runas [-r] LOGNAME COMM [ARG ...]\n"
"             runas -h\n\n"
"Operands:\n"
"    LOGNAME  Login name.\n"
"    COMM     Command to run.\n"
"    ARG      Argument to COMM.\n\n"
"Options:\n"
"    -r       Only set the real user and group IDs.\n"
"    -h       Print this help screen.\n\n"
"Copyright 2022 and 2023 Odin Kroeger.\n"
"Released under the GNU General Public License.\n"
"This programme comes with ABSOLUTELY NO WARRANTY."
            );
            return EXIT_SUCCESS;
        case 'r':
            real = true;
            break;
        default:
            return EXIT_FAILURE;
        }
    }

    argc -= optind;
    argv += optind;

    if (argc < 2) {
        (void) fputs("usage: runas LOGNAME CMD [ARG ...]\n", stderr);
        return EXIT_FAILURE;
    }

    errno = 0;
    pwd = getpwnam(argv[0]);
    if (!pwd) {
        if (errno == 0) {
            errx(EXIT_FAILURE, "no such user");
        } else {
            err(EXIT_FAILURE, "getpwnam");
        }
    }

    if (real) {
        if (setreuid(pwd->pw_uid, geteuid()) != 0) {
            err(EXIT_FAILURE, "setreuid");
        }
        if (setregid(pwd->pw_gid, getegid()) != 0) {
            err(EXIT_FAILURE, "setregid");
        }
    } else {
        if (setgroups(1, (gid_t[1]) {(gid_t) pwd->pw_gid}) != 0) {
            err(EXIT_FAILURE, "setgroups");
        }
        if (setgid(pwd->pw_gid) != 0) {
            err(EXIT_FAILURE, "setgid");
        }
        if (setuid(pwd->pw_uid) != 0) {
            err(EXIT_FAILURE, "setuid");
        }
    }

    comm = &argv[1];
    (void) execvp(*comm, comm);

    /* This point should not be reached. */
    err(EXIT_FAILURE, "exec %s", *comm);
}
