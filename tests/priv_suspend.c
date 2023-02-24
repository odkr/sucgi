/*
 * Test priv_suspend.
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
main (void)
{
    Error ret;

    ret = priv_suspend();
    switch (ret) {
    case OK:
        break;
    case ERR_SYS_SETGROUPS:
        err(EXIT_FAILURE, "setgroups");
    case ERR_SYS_SETGID:
        err(EXIT_FAILURE, "setgid");
    case ERR_SYS_SETUID:
        err(EXIT_FAILURE, "setuid");
    case ERR_PRIV_RESUME:
        errx(EXIT_FAILURE, "could resume superuser privileges.");
    default:
        errx(EXIT_FAILURE, "returned %u.", ret);
    }

    printf("euid=%llu egid=%llu ruid=%llu rgid=%llu\n",
           (unsigned long long) geteuid(),
           (unsigned long long) getegid(),
           (unsigned long long) getuid(),
           (unsigned long long) getgid());

    errno = 0;
    if (setuid(0) != 0) {
        err(EXIT_FAILURE, "seteuid");
    }

    return EXIT_SUCCESS;
}
