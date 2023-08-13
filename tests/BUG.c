/*
 * Test BUG.
 *
 * Copyright 2022 and 2023 Odin Kroeger.
 *
 * This file is part of suCGI.
 *
 * SuCGI is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * SuCGI is distributed in the hope that it will be useful, but WITHOUT
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

#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <syslog.h>

#include "../error.h"
#include "../macros.h"

#if !defined LOG_PERROR
#error LOG_PERROR is not defined.
#endif

int
main(int argc, char **argv)
{
    const char *arg;

    if (argc < 2) {
        (void) fputs("usage: BUG ARG\n", stderr);
        return EXIT_FAILURE;
    }

    arg = argv[1];

    errno = 0;
    if (atexit(closelog) != 0) {
        err(EXIT_FAILURE, "atexit");
    }

    openlog("error", LOG_NDELAY | LOG_PERROR, LOG_USER);

    (void) BUG("%s", arg);

    /* NOTREACHED */
    return EXIT_SUCCESS;
}
