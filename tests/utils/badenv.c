/*
 * Run a programme with a given environment, any environment.
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

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>


/*
 * Global variables
 */

/* The environment. */
extern char **environ;


/*
 * Main
 */

int
main (int argc, char **argv)
{
    long nenv = 0;
    long nvars = 0;
    bool inherit = true;

    int opt;
    while ((opt = getopt(argc, argv, "in:h")) != -1) {
        switch (opt) {
        case 'h':
            (void) puts(
"badenv - run a programme with a given environment, any environment\n\n"
"Usage:    badenv [-i] [-nN] [VAR ...] [CMD [ARG ...]]\n"
"          badenv -h\n\n"
"Operands:\n"
"    VAR   An environment variable. Use -n if VAR contains no '='.\n"
"    CMD   A command to run. Not searched for in $PATH.\n"
"    ARG   An argument to CMD.\n\n"
"Options:\n"
"    -i    Do not inherit the current environment.\n"
"    -n N  Treat the first N arguments as environment variables.\n"
"    -h    Print this help screen.\n\n"
"Copyright 2022 and 2023 Odin Kroeger.\n"
"Released under the GNU General Public License.\n"
"This programme comes with ABSOLUTELY NO WARRANTY."
            );
            return EXIT_SUCCESS;
        case 'i':
            inherit = false;
            break;
        case 'n':
            errno = 0;
            nvars = strtol(optarg, NULL, 10);
            if (nvars == 0 && errno != 0) {
                err(EXIT_FAILURE, "-n");
            }
            if (nvars < 0) {
                errx(EXIT_FAILURE, "-n: %ld is negative", nvars);
            }
            if (nvars > SHRT_MAX) {
                errx(EXIT_FAILURE, "-n: %ld is too large", nvars);
            }
            break;
        default:
            return EXIT_FAILURE;
        }
    }

    argc -= optind;
    argv += optind;

    if (nvars > (long) argc) {
        errx(EXIT_FAILURE, "-n: only %d arguments given", argc);
    }

    if (inherit) {
        while (environ[nenv] != NULL) {
            ++nenv;
        }
    }

    if (nenv > SHRT_MAX) {
        errx(EXIT_FAILURE, "environment contains too many variables");
    }

    if (nvars == 0) {
        while (nvars < argc
               && argv[nvars] != NULL
               && strchr(argv[nvars], '='))
        {
            ++nvars;
        }
    }

    errno = 0;
    char **vars = calloc((size_t) (nenv + nvars + 1), sizeof(*vars));
    if (vars == NULL) {
        err(EXIT_FAILURE, "calloc");
    }

    /*
     * New variables are simply inserted before the current ones,
     * so there may be duplicates; this is BADenv, after all.
     */

    if (nvars > 0) {
        if ((size_t) nvars > SIZE_MAX / sizeof(*argv)) {
            /* NOTREACHED */
            errx(EXIT_FAILURE, "too many variables given");
        }

        (void) memcpy(vars, argv, (size_t) nvars * sizeof(*argv));
    }

    if (nenv > 0) {
        if ((size_t) nenv > SIZE_MAX / sizeof(*environ)) {
            /* NOTREACHED */
            errx(EXIT_FAILURE, "environment contains too many variables");
        }

        (void) memcpy(&vars[nvars], environ, (size_t) nenv * sizeof(*environ));
    }

    char **cmd = &argv[nvars];
    if (*cmd != NULL) {
        errno = 0;
        (void) execve(*cmd, cmd, vars);
        err(EXIT_FAILURE, "exec %s", *cmd);
    }

    for (; *vars; vars++) {
        (void) puts(*vars);
    }

    return EXIT_SUCCESS;
}