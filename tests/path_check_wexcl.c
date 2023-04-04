/*
 * Test path_check_wexcl.
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
#include <pwd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../path.h"


int
main (int argc, char **argv)
{
    struct passwd *user;
    char *basedir;
    char *fname;
    char *logname;
    Error ret;

    if (argc != 4) {
        fputs("usage: path_check_wexcl LOGNAME BASEDIR FNAME\n", stderr);
        return EXIT_FAILURE;
    }

    logname = argv[1];
    basedir = argv[2];
    fname = argv[3];

    errno = 0;
    user = getpwnam(logname);
    if (!user) {
        if (errno == 0) {
            errx(EXIT_FAILURE, "no such user");
        } else {
            err(EXIT_FAILURE, "getpwnam");
	    }
    }

    ret = path_check_wexcl(user->pw_uid, basedir, fname);
    switch (ret) {
    case OK:
        break;
    case ERR_BASEDIR:
        errx(EXIT_FAILURE, "file %s: not within %s", fname, basedir);
    case ERR_WEXCL:
        errx(EXIT_FAILURE, "file %s: writable by non-owner", fname);
    case ERR_SYS:
        err(EXIT_FAILURE, "stat %s", fname);
    default:
        errx(EXIT_FAILURE, "returned %u", ret);
    }

    return EXIT_SUCCESS;
}
