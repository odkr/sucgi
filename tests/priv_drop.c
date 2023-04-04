/*
 * Test priv_drop.
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

#include <sys/types.h>
#include <err.h>
#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../priv.h"
#include "../types.h"


int
main (int argc, char **argv)
{
    struct passwd *pwd;
    Error ret;

    if (argc != 2) {
        fputs("usage: priv_drop LOGNAME\n", stderr);
        return EXIT_FAILURE;
    }

    errno = 0;
    pwd = getpwnam(argv[1]);
    if (!pwd) {
        if (errno == 0) {
            errx(EXIT_FAILURE, "no such user");
        } else {
            err(EXIT_FAILURE, "getpwnam");
        }
    }

    ret = priv_drop(pwd->pw_uid, pwd->pw_gid, 1, (gid_t [1]) {pwd->pw_gid});
    switch (ret) {
    case OK:
        break;
    case ERR_SYS:
        err(EXIT_FAILURE, "privilege drop");
    case ERR_PRIV:
        errx(EXIT_FAILURE, "could resume superuser privileges.");
    default:
        errx(EXIT_FAILURE, "returned %u.", ret);
    }

    printf("euid=%llu egid=%llu ruid=%llu rgid=%llu\n",
           (unsigned long long) geteuid(),
           (unsigned long long) getegid(),
           (unsigned long long) getuid(),
           (unsigned long long) getgid());

    return EXIT_SUCCESS;
}
