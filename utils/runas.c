/*
 * Run a command as a given user.
 *
 * Copyright 2022 and 2023 Odin Kroeger.
 *
 * This file is part of suCGI.
 *
 * suCGI is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * suCGI is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General
 * Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with suCGI. If not, see <https://www.gnu.org/licenses/>.
 */

#define _BSD_SOURCE
#define _DARWIN_C_SOURCE
#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <sys/types.h>
#include <err.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


int
main (int argc, char **argv)
{
    struct passwd *pwd;
    char **comm;
    int opt;

    while ((opt = getopt(argc, argv, "hr")) != -1) {
        switch (opt) {
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
"    -h       Print this help screen.\n\n"
"Copyright 2022 and 2023 Odin Kroeger.\n"
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

    if (setgroups(1, (gid_t[1]) {(gid_t) pwd->pw_gid}) != 0) {
        err(EXIT_FAILURE, "setgroups");
    }

    if (setgid(pwd->pw_gid) != 0) {
        err(EXIT_FAILURE, "setgid");
    }

    if (setuid(pwd->pw_uid) != 0) {
        err(EXIT_FAILURE, "setuid");
    }

    comm = &argv[1];
    (void) execvp(*comm, comm);

    /* NOTREACHED */
    err(EXIT_FAILURE, "exec %s", *comm);
}
