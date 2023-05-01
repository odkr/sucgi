/*
 * Signal handling for tests.
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
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "../../macros.h"
#include "trap.h"


/*
 * Globals
 */

/* Last signal caught. */
int trapped = 0;


/*
 * Functions
 */

static void
catch(const int signo) {
    trapped = signo;
}

int
trapsigs(const size_t nsignals, const int *const signos)
{
    const struct sigaction action = {
        .sa_handler = catch,
        .sa_flags = SA_RESTART
    };

    for (size_t i = 0; i < nsignals; ++i) {
        errno = 0;
        if (sigaction(signos[i], &action, NULL) != 0) {
            return -1;
        }
    }
    return 0;
}

void
traphandle(void (*cleanup)(void))
{
    if (trapped > 0) {
        /* Default signal handler. */
        const struct sigaction defhdl = {
            .sa_handler = SIG_DFL
        };

        warnx("%s", strsignal((int) trapped));

        if (cleanup != NULL) {
            cleanup();
        }

        (void) sigaction((int) trapped, &defhdl, NULL);
        (void) raise((int) trapped);

        /* Should be unreachable. */
        abort();
    }
}
