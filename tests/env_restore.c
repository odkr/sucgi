/*
 * Test env_restore.
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

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <regex.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../env.h"
#include "../macros.h"
#include "../params.h"
#include "../str.h"
#include "util/abort.h"
#include "util/array.h"
#include "util/str.h"
#include "util/types.h"


/*
 * Constants
 */

/* Maximum number of environment variables for testing. */
#define MAX_TEST_NVARS (MAX_NVARS + 1U)

/* Maximum string length for testing. */
#define MAX_TEST_STR_LEN (MAX_STR_LEN + 1U)


/*
 * Data types
 */

/* Mapping of constant inputs to constant outputs. */
typedef struct {
    const char *const *vars;        /* Variables to set. */
    size_t npatterns;               /* Number of patterns. */
    const char *const *patterns;    /* Patterns. */
    const char *const *env;         /* Resulting environmnet. */
    Error retval;                   /* Return value. */
    int signal;                     /* Signal caught, if any. */
} EnvRestoreArgs;


/*
 * Main
 */

int
main(void)
{

    /* RATS: ignore; used safely. */
    const char *const var_exprs[] = /* cppcheck-suppress misra-c2012-9.2 */
        SAFE_ENV_VARS;

    /* RATS: ignore; used safely. */
    const char *too_many_vars[MAX_NVARS + 1U] = {0};

    /* RATS: ignore; used safely. */
    char long_var[MAX_VAR_LEN] = {0};

    /* RATS: ignore; used safely. */
    char huge_var[MAX_VAR_LEN + 1] = {0};

    /* RATS: ignore; used safely. */
    char long_name[MAX_VAR_LEN] = {0};

    /* RATS: ignore; used safely. */
    char huge_name[MAX_VAR_LEN] = {0};

    const EnvRestoreArgs cases[] = {
        /* Null tests. */
        {
            (const char *const []) {NULL},
            0, (const char *const []) {NULL},
            (const char *const []) {NULL}, OK, 0
        },
        {
            (const char *const []) {NULL},
            1, (const char *const []) {"."},
            (const char *const []) {NULL}, OK, 0
        },

        /* Too many variables. */
        {
            too_many_vars,
            1, (const char *const []) {"."},
            (const char *const []) {NULL}, ERR_LEN, 0
        },

        /* Overly long variables should be ignored. */
        {
            (const char *const []) {long_var, NULL},
            1, (const char *const []) {"."},
            (const char *const []) {long_var, NULL}, OK, 0
        },
        {
            (const char *const []) {huge_var, NULL},
            1, (const char *const []) {"."},
            (const char *const []) {NULL}, OK, 0
        },

        /* Variables with overly long should be ignored, too. */
        {
            (const char *const []) {long_name, NULL},
            1, (const char *const []) {"."},
            (const char *const []) {long_name, NULL}, OK, 0
        },
        {
            (const char *const []) {huge_name, NULL},
            1, (const char *const []) {"."},
            (const char *const []) {NULL}, OK, 0
        },

        /* Simple tests. */
        {
            (const char *const []) {"foo=foo", NULL},
            1, (const char *const []) {"."},
            (const char *const []) {"foo=foo", NULL}, OK, 0
        },
        {
            (const char *const []) {"foo=foo", "bar=bar", NULL},
            1, (const char *const []) {"^foo$"},
            (const char *const []) {"foo=foo", NULL}, OK, 0
        },
        {
            (const char *const []) {"foo=foo", "bar=bar", "foobar=foobar", NULL},
            1, (const char *const []) {"^foo"},
            (const char *const []) {"foo=foo", "foobar=foobar", NULL}, OK, 0
        },
        {
            (const char *const []) {"foo=foo", "bar=bar", "foobar=foobar", NULL},
            1, (const char *const []) {"foo"},
            (const char *const []) {"foo=foo", "foobar=foobar", NULL}, OK, 0
        },
        {
            (const char *const []) {"foo=foo", "bar=bar", "foobar=foobar", NULL},
            1, (const char *const []) {"bar$"},
            (const char *const []) {"bar=bar", "foobar=foobar", NULL}, OK, 0
        },
        {
            (const char *const []) {"foo=foo", "bar=bar", "foobar=foobar", NULL},
            1, (const char *const []) {"bar"},
            (const char *const []) {"bar=bar", "foobar=foobar", NULL}, OK, 0
        },
        {
            (const char *const []) {"foo=foo", "bar=bar", "baz=baz", NULL},
            0, (const char *const []) {NULL},
            (const char *const []) {NULL}, OK, 0
        },
        {
            (const char *const []) {"foo=foo", "bar=bar", "baz=baz", NULL},
            1, (const char *const []) {"^$"},
            (const char *const []) {NULL}, OK, 0
        },

        /* Syntax errors. */
        {
            (const char *const []) {"", NULL},
            1, (const char *const []) {"."},
            (const char *const []) {NULL}, OK, 0
        },
        {
            (const char *const []) {"foo", NULL},
            1, (const char *const []) {"."},
            (const char *const []) {NULL}, OK, 0
        },
        {
            (const char *const []) {"=foo", NULL},
            1, (const char *const []) {"."},
            (const char *const []) {NULL}, OK, 0
        },

        /* Illegal names. */
        {
            (const char *const []) {" foo=foo", NULL},
            1, (const char *const []) {"."},
            (const char *const []) {NULL}, OK, 0
        },
        {
            (const char *const []) {"0foo=foo", NULL},
            1, (const char *const []) {"."},
            (const char *const []) {NULL}, OK, 0
        },
        {
            (const char *const []) {"*=foo", NULL},
            1, (const char *const []) {"."},
            (const char *const []) {NULL}, OK, 0
        },
        {
            (const char *const []) {"foo =foo", NULL},
            1, (const char *const []) {"."},
            (const char *const []) {NULL}, OK, 0
        },
        {
            (const char *const []) {"$(foo)=foo", NULL},
            1, (const char *const []) {"."},
            (const char *const []) {NULL}, OK, 0
        },
        {
            (const char *const []) {"`foo`=foo", NULL},
            1, (const char *const []) {"."},
            (const char *const []) {NULL}, OK, 0
        },

        /* More realistic tests. */
        {
            (const char *const []) {"bar=bar", "foo_0=foo", NULL},
            1, (const char *const []) {"^foo_[0-9]+$"},
            (const char *const []) {"foo_0=foo", NULL}, OK, 0
        },
        {
            (const char *const []) {"bar=bar", "foo_0=foo", NULL},
            1, (const char *const []) {"^foo_(0|[1-9][0-9]*)$"},
            (const char *const []) {"foo_0=foo", NULL}, OK, 0
        },
        {
            (const char *const []) {"foo_0=foo", "foo_01=foo", NULL},
            1, (const char *const []) {"^foo_(0|[1-9][0-9]*)$"},
            (const char *const []) {"foo_0=foo", NULL}, OK, 0
        },


        /* Simple tests with real patterns. */
        {
            (const char *const []) {"foo=foo", NULL},
            NELEMS(var_exprs), var_exprs,
            (const char *const []) {NULL}, OK, 0
        },

        {
            (const char *const []) {"PATH_TRANSLATED=foo", NULL},
            NELEMS(var_exprs), var_exprs,
            (const char *const []) {"PATH_TRANSLATED=foo", NULL}, OK, 0
        },
        {
            (const char *const []) {"PATH_TRANSLATED=foo", "foo=foo", NULL},
            NELEMS(var_exprs), var_exprs,
            (const char *const []) {"PATH_TRANSLATED=foo", NULL}, OK, 0
        },

        /* Illegal names. */
        {
            (const char *const []) {" IPV6=foo", NULL},
            NELEMS(var_exprs), var_exprs,
            (const char *const []) {NULL}, OK, 0
        },
        {
            (const char *const []) {"0IPV6=foo", NULL},
            NELEMS(var_exprs), var_exprs,
            (const char *const []) {NULL}, OK, 0
        },
        {
            (const char *const []) {"*=foo", NULL},
            NELEMS(var_exprs), var_exprs,
            (const char *const []) {NULL}, OK, 0
        },
        {
            (const char *const []) {"IPV6 =foo", NULL},
            NELEMS(var_exprs), var_exprs,
            (const char *const []) {NULL}, OK, 0
        },
        {
            (const char *const []) {"$(IPV6)=foo", NULL},
            NELEMS(var_exprs), var_exprs,
            (const char *const []) {NULL}, OK, 0
        },
        {
            (const char *const []) {"`IPV6`=foo", NULL},
            NELEMS(var_exprs), var_exprs,
            (const char *const []) {NULL}, OK, 0
        },

        /* Odd but legal values. */
        {
            (const char *const []) {
                "SSL_CLIENT_S_DN_C_0=", "SSL_CLIENT_S_DN_C_1==",
                "SSL_CLIENT_S_DN_C_2= ", "SSL_CLIENT_S_DN_C_3=\t",
                "SSL_CLIENT_S_DN_C_4=\n", NULL
            },
            NELEMS(var_exprs), var_exprs,
            (const char *const []) {
                "SSL_CLIENT_S_DN_C_0=", "SSL_CLIENT_S_DN_C_1==",
                "SSL_CLIENT_S_DN_C_2= ", "SSL_CLIENT_S_DN_C_3=\t",
                "SSL_CLIENT_S_DN_C_4=\n", NULL
            }, OK, 0
        },

        /* Real-world tests. */
        {
            (const char *const []) {
                "CLICOLOR=x",
                "EDITOR=vim",
                "HOME=/home/jdoe",
                "IFS=",
                "LANG=en_GB.UTF-8",
                "LOGNAME=jdoe",
                "OLDPWD=/home",
                "PAGER=less",
                "PATH=/bin:/usr/bin",
                "PWD=/home/jdoe",
                "SHELL=/bin/zsh",
                "USER=jdoe",
                "VISUAL=vim",
                NULL
            },
            NELEMS(var_exprs), var_exprs,
            (const char *const []) {NULL}, OK, 0
        },
        {
            (const char *const []) {
                "CLICOLOR=x",
                "DOCUMENT_ROOT=/home/jdoe/public_html",
                "EDITOR=vim",
                "HOME=/home/jdoe",
                "HTTP_HOST=www.foo.example",
                "HTTP_REFERER=https://www.bar.example",
                "HTTP_USER_AGENT=FakeZilla/1",
                "HTTPS=on",
                "IFS=",
                "LANG=en_GB.UTF-8",
                "LOGNAME=jdoe",
                "OLDPWD=/home",
                "PAGER=less",
                "PATH_TRANSLATED=/home/jdoe/public_html/index.php",
                "PATH=/bin:/usr/bin",
                "PWD=/home/jdoe",
                "QUERY_STRING=foo=bar&bar=baz",
                "REMOTE_ADDR=100::1:2:3",
                "REMOTE_HOST=100::1:2:3",
                "REMOTE_PORT=50000",
                "REQUEST_METHOD=GET",
                "REQUEST_URI=/index.php",
                "SCRIPT_NAME=index.php",
                "SERVER_ADMIN=admin@foo.example",
                "SERVER_NAME=www.foo.example",
                "SERVER_PORT=443",
                "SERVER_SOFTWARE=Apache v2.4",
                "SHELL=/bin/zsh",
                "TMPDIR=/tmp/user/1000",
                "USER=jdoe",
                "VISUAL=vim",
                NULL
            },
            NELEMS(var_exprs), var_exprs,
            (const char *const []) {
                "HTTP_HOST=www.foo.example",
                "HTTP_REFERER=https://www.bar.example",
                "HTTP_USER_AGENT=FakeZilla/1",
                "HTTPS=on",
                "PATH_TRANSLATED=/home/jdoe/public_html/index.php",
                "QUERY_STRING=foo=bar&bar=baz",
                "REMOTE_ADDR=100::1:2:3",
                "REMOTE_HOST=100::1:2:3",
                "REMOTE_PORT=50000",
                "REQUEST_METHOD=GET",
                "REQUEST_URI=/index.php",
                "SCRIPT_NAME=index.php",
                "SERVER_ADMIN=admin@foo.example",
                "SERVER_NAME=www.foo.example",
                "SERVER_PORT=443",
                "SERVER_SOFTWARE=Apache v2.4",
                NULL
            }, OK, 0
        }
    };


    volatile int result = PASS;
    char *null = NULL;
    size_t nvars = NELEMS(too_many_vars) - 1U;

    /* Initialise dynamic test cases. */
    for (size_t i = 0; i < nvars; ++i) {
        char *var;
        int nbytes;

        errno = 0;
        /* cppcheck-suppress [misra-c2012-11.5]; bad advice for malloc. */
        var = malloc(MAX_VAR_LEN);
        if (var == NULL) {
            err(EXIT_FAILURE, "malloc");
        }

        errno = 0;
        /* RATS: ignore; format is a literal and expansion is bounded. */
        nbytes = snprintf(var, MAX_VAR_LEN, "var%zu=", i);
        if (nbytes < 1) {
            err(EXIT_FAILURE, "snprintf");
        }

        too_many_vars[i] = var;
    }
    too_many_vars[nvars] = NULL;

    (void) memset(long_var, 'x', sizeof(long_var) - 1U);
    (void) memset(huge_var, 'x', sizeof(huge_var) - 1U);

/* I want to truncate those strings. */
#if defined(__GNUC__) && __GNUC__ >= 8
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif

    /* RATS: ignore; the buffer is large enough. */
    (void) strncpy(long_var, "var=", 4);
    /* RATS: ignore; the buffer is large enough. */
    (void) strncpy(huge_var, "var=", 4);

#if defined(__GNUC__) && __GNUC__ >= 8
#pragma GCC diagnostic pop
#endif

    (void) memset(long_name, 'x', sizeof(long_name) - 1U);
    long_name[MAX_VARNAME_LEN - 1U] = '=';

    (void) memset(huge_name, 'x', sizeof(huge_name) - 1U);
    huge_name[MAX_VARNAME_LEN] = '=';

    /* Run tests. */
    for (volatile size_t i = 0; i < NELEMS(cases); ++i) {
        const EnvRestoreArgs args = cases[i];

        if (sigsetjmp(abort_env, 1) == 0) {
            regex_t pregs[MAX_TEST_NVARS];
            Error retval;

            environ = &null;

            for (size_t j = 0; j < args.npatterns; ++j) {
                int err;

                err = regcomp(&pregs[j], args.patterns[j],
                              REG_EXTENDED | REG_NOSUB);

                if (err != 0) {
                    /* RATS: ignore; used safely. */
                    char errmsg[MAX_ERRMSG_LEN];

                    (void) regerror(err, &pregs[j], errmsg, sizeof(errmsg));
                    errx(EXIT_FAILURE, "%s:%d: regcomp: %s",
                         __FILE__, __LINE__, errmsg);
                }
            }

            (void) abort_catch(err);
            retval = env_restore(args.vars, args.npatterns, pregs);
            (void) abort_reset(err);

            if (retval != args.retval) {
                result = FAIL;
                warnx("%zu: (<vars>, %zu, <pregs>) → %u [!]",
                      i, args.npatterns, retval);
            }

            if (retval == OK) {
                volatile size_t nexpected = 0;
                volatile size_t nactual = 0;

                while (args.env[nexpected] != NULL) {
                    assert(nexpected < SIZE_MAX);
                    ++nexpected;
                }

                while (environ[nactual] != NULL) {
                    assert(nexpected < SIZE_MAX);
                    ++nactual;
                }

                if (
                    !array_eq(
                        args.env, nexpected, sizeof(*args.env),
                        environ, nactual, sizeof(*environ),
                        (CompFn) str_cmp_ptrs
                    )
                ) {
                    result = FAIL;
                    warnx("%zu: (<vars>, %zu, <pregs>) ─→ <environ> [!]",
                          i, args.npatterns);
                }
            }
        }

        if (abort_signal != args.signal) {
            result = FAIL;
            warnx("%zu: (<vars>, %zu, <pregs>) ↑ %s [!]",
                  i, args.npatterns, strerror(abort_signal));
        }
    }

    return result;
}
