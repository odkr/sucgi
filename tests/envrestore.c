/*
 * Test envrestore.
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
#include "../config.h"
#include "../macros.h"
#include "../str.h"
#include "lib/check.h"



/*
 * Constants
 */

/* Maximum number of environment variables for testing. */
#define MAX_TEST_NVARS (MAX_NVARS + 1U)

/* Maximum length of strings for testing. */
#define MAX_TEST_STR_LEN (MAX_STR_LEN + 1U)


/*
 * Data types
 */

/* Mapping of constant inputs to constant outputs. */
typedef struct {
    const char *const *vars;        /* Variables to set. */
    size_t npatterns;               /* Number of patterns. */
    const char *const *patterns;    /* Patterns. */
    const char *const *environ;     /* Resulting environmnet. */
    Error retval;                   /* Return value. */
    int signo;                      /* Signal caught, if any. */
} Args;


/*
 * Module variables
 */

/* See config.h. */
const char *const envpatterns[] = ENV_PATTERNS;

/* Too many variables. */
static const char *toomanyvars[MAX_NVARS + 1U] = {NULL};

/* A variable that is as long as permitted. */
static char longvar[MAX_VAR_LEN] = {'\0'};

/* A variable that is too long. */
static char hugevar[MAX_VAR_LEN + 1U] = {'\0'};

/* A variable with a name that is as long as is permitted. */
static char longname[MAX_VAR_LEN] = {'\0'};

/* A variable with a name that is too long. */
static char hugename[MAX_VAR_LEN] = {'\0'};

/* Simple test cases. */
static const Args cases[] = {
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
        toomanyvars,
        1, (const char *const []) {"."},
        (const char *const []) {NULL}, ERR_LEN, 0
    },

    /* Overly long variables should be ignored. */
    {
        (const char *const []) {longvar, NULL},
        1, (const char *const []) {"."},
        (const char *const []) {longvar, NULL}, OK, 0
    },
    {
        (const char *const []) {hugevar, NULL},
        1, (const char *const []) {"."},
        (const char *const []) {NULL}, OK, 0
    },

    /* Variables with overly long should be ignored, too. */
    {
        (const char *const []) {longname, NULL},
        1, (const char *const []) {"."},
        (const char *const []) {longname, NULL}, OK, 0
    },
    {
        (const char *const []) {hugename, NULL},
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
        NELEMS(envpatterns), envpatterns,
        (const char *const []) {NULL}, OK, 0
    },

    {
        (const char *const []) {"PATH_TRANSLATED=foo", NULL},
        NELEMS(envpatterns), envpatterns,
        (const char *const []) {"PATH_TRANSLATED=foo", NULL}, OK, 0
    },
    {
        (const char *const []) {"PATH_TRANSLATED=foo", "foo=foo", NULL},
        NELEMS(envpatterns), envpatterns,
        (const char *const []) {"PATH_TRANSLATED=foo", NULL}, OK, 0
    },

    /* Illegal names. */
    {
        (const char *const []) {" IPV6=foo", NULL},
        NELEMS(envpatterns), envpatterns,
        (const char *const []) {NULL}, OK, 0
    },
    {
        (const char *const []) {"0IPV6=foo", NULL},
        NELEMS(envpatterns), envpatterns,
        (const char *const []) {NULL}, OK, 0
    },
    {
        (const char *const []) {"*=foo", NULL},
        NELEMS(envpatterns), envpatterns,
        (const char *const []) {NULL}, OK, 0
    },
    {
        (const char *const []) {"IPV6 =foo", NULL},
        NELEMS(envpatterns), envpatterns,
        (const char *const []) {NULL}, OK, 0
    },
    {
        (const char *const []) {"$(IPV6)=foo", NULL},
        NELEMS(envpatterns), envpatterns,
        (const char *const []) {NULL}, OK, 0
    },
    {
        (const char *const []) {"`IPV6`=foo", NULL},
        NELEMS(envpatterns), envpatterns,
        (const char *const []) {NULL}, OK, 0
    },

    /* Odd but legal values. */
    {
        (const char *const []) {
            "SSL_CLIENT_S_DN_C_0=", "SSL_CLIENT_S_DN_C_1==",
            "SSL_CLIENT_S_DN_C_2= ", "SSL_CLIENT_S_DN_C_3=\t",
            "SSL_CLIENT_S_DN_C_4=\n", NULL
        },
        NELEMS(envpatterns), envpatterns,
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
        NELEMS(envpatterns), envpatterns,
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
            "REQUEST_METHOD=GET"
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
        NELEMS(envpatterns), envpatterns,
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
            "REQUEST_METHOD=GET"
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


/*
 * Prototypes
 */

/*
 * Replace the format specifiers in MESSAGE with a string describing
 * ARGS->vars, ARGS->npatterns, a string describing ARGS->patterns,
 * and a string describing the environment, and the name of the
 * signal number SIGNO, in that order, and print the resulting
 * string and a linefeed to stderr.
 */
__attribute__((nonnull(1, 2)))
static void report(const char *message, const Args *args,
                   Error retval, int signo);


/*
 * Check whether the environment equals VARS.
 */
__attribute__((nonnull(1), warn_unused_result))
static bool cmpenv(const char *const vars[MAX_TEST_NVARS]);

/*
 * Concatenate the strings pointed to by DEST and SRC, append a NUL, and
 * return a pointer to the terminating NUL in END, but do not write past
 * LIM, which should be DEST + sizeof(DEST).
 *
 * If NDEBUG is defined, no error checking takes place.
 * Otherwise, aborts if appending SRC to DEST would require to write past LIM.
 */
__attribute__((nonnull(1, 2, 3, 4)))
static void catstrs(char *dest, const char *const src,
                    const char *const lim, char **end);

/*
 * Join the first N strings in STRS using the separator SEP and store the
 * result in DEST, which must be large enough to hold SIZE bytes. If STRS
 * contains a NULL pointer, processing stops at that pointer. STRS must
 * either be NULL-terminated or have at least N elements.
 *
 * If NDEBUG is defined, no error checking takes place.
 * Aborts if SIZE is too small to hold the result otherwise.
 */
__attribute__((nonnull(2, 3, 5)))
static void joinstrs(size_t nstrs, const char *const *strs,
                     const char *sep, size_t size, char *dest);


/*
 * Return the index of KEY within ARR or -1 if KEY cannot be found.
 * ARR must be NULL-terminated.
 */
__attribute__((nonnull(1, 2), warn_unused_result))
static int findstrel(const char *key, char const *const *arr);


/*
 * Functions
 */

static void
report(const char *const message, const Args *const args,
       const Error retval, const int signo)
{
    char varstr[MAX_TEST_STR_LEN];
    char patstr[MAX_TEST_STR_LEN];
    char envstr[MAX_TEST_STR_LEN];

    (void) joinstrs(MAX_TEST_NVARS, args->vars, ", ",
                    sizeof(varstr), varstr);
    (void) joinstrs(args->npatterns, args->patterns, ", ",
                    sizeof(patstr), patstr);
    (void) joinstrs(MAX_TEST_NVARS, (const char *const *) environ, " ",
                    sizeof(envstr), envstr);

/* message is not a literal. */
#if defined(__GNUC__) && __GNUC__ >= 3
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif

    warnx(message, varstr, args->npatterns, patstr,
          retval, envstr, strsignal(signo));

#if defined(__GNUC__) && __GNUC__ >= 3
#pragma GCC diagnostic pop
#endif
}

static bool
cmpenv(const char *const vars[MAX_TEST_NVARS])
{
    for (size_t i = 0; vars[i]; ++i) {
        if (findstrel((const char *) vars[i],
                      (const char *const *) environ) < 0)
        {
            return false;
        }
    }

    for (size_t i = 0; environ[i]; ++i) {
        if (findstrel((const char *) environ[i],
                      (const char *const *) vars) < 0)
        {
            return false;
        }
    }

    return true;
}

static void
catstrs(char *const dest, const char *const src,
        const char *const lim, char **const end)
{
    assert(dest != NULL);
    assert(src != NULL);
    assert(lim != NULL);
    assert(lim > dest);
    assert(end != NULL);

    *end = stpncpy(dest, src, (size_t) (lim - dest));
    assert(src[*end - dest] == '\0');
}

static void
joinstrs(const size_t nstrs, const char *const *const strs,
         const char *const sep, const size_t size, char *const dest)
{
    char *ptr;
    char *lim;

    assert(strs != NULL);
    assert(sep != NULL);
    assert(dest != NULL);

    ptr = dest;
    lim = dest + size - 1U;

    (void) memset(dest, '\0', size);
    for (size_t i = 0U; i < nstrs && strs[i]; ++i) {
        if (i > 0U) {
            catstrs(ptr, sep, lim, &ptr);
        }

        catstrs(ptr, strs[i], lim, &ptr);
    }
}

static int
findstrel(const char *const key, const char *const *const arr)
{
    size_t len;
    size_t size;

    len = strnlen(key, MAX_TEST_STR_LEN);
    assert(len < MAX_TEST_STR_LEN);

    size = len + 1U;
    for (int i = 0; arr[i]; ++i) {
        if (strncmp(key, arr[i], size) == 0) {
            return i;
        }
    }

    return -1;
}


/*
 * Main
 */

int
main (void) {
    volatile int result = TEST_PASSED;
    char *null = NULL;
    size_t nvars = NELEMS(toomanyvars) - 1U;

    checkinit();

    /* Initialise dynamic test cases. */
    for (size_t i = 0; i < nvars; ++i) {
        char *var;
        int nbytes;

        errno = 0;
        var = malloc(MAX_VAR_LEN);
        if (var == NULL) {
            err(EXIT_FAILURE, "malloc");
        }

        errno = 0;
        nbytes = snprintf(var, MAX_VAR_LEN, "var%zu=", i);
        if (nbytes < 1) {
            err(EXIT_FAILURE, "snprintf");
        }

        toomanyvars[i] = var;
    }
    toomanyvars[nvars] = NULL;

    (void) memset(longvar, 'x', sizeof(longvar) - 1U);
    (void) copystr(4, "var=", longvar);

    (void) memset(hugevar, 'x', sizeof(hugevar) - 1U);
    (void) copystr(4, "var=", longvar);

    (void) memset(longname, 'x', sizeof(longname) - 1U);
    longname[MAX_VARNAME_LEN - 1U] = '=';

    (void) memset(hugename, 'x', sizeof(hugename) - 1U);
    hugename[MAX_VARNAME_LEN] = '=';

    /* Run tests. */
    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];
        regex_t pregs[MAX_TEST_NVARS];
        int jumpval;
        volatile Error retval;

        environ = &null;

        for (size_t j = 0; j < args.npatterns; ++j) {
            int err;

            err = regcomp(&pregs[j], args.patterns[j],
                          REG_EXTENDED | REG_NOSUB);

            if (err != 0) {
                char errmsg[MAX_ERRMSG_LEN];

                (void) regerror(err, &pregs[j], errmsg, sizeof(errmsg));
                errx(EXIT_FAILURE, "regcomp: %s", errmsg);
            }
        }

        jumpval = sigsetjmp(checkenv, true);

        if (jumpval == 0) {
            checking = 1;
            retval = envrestore(args.vars, args.npatterns, pregs);
            checking = 0;

            if (retval != args.retval) {
                report("({%s}, %zu, {%s}) → %u [!] → %s ↑ %s", &args,
                       retval, jumpval);
                result = TEST_FAILED;
            }

            if (retval == OK && !cmpenv(args.environ)) {
                report("({%s}, %zu, {%s}) → %u ─→ %s [!] ↑ %s",
                       &args, retval, jumpval);
                result = TEST_FAILED;
            }
        }

        if (jumpval != args.signo) {
            report("({%s}, %zu, {%s}) → %u ─→ %s ↑ %s [!]",
                   &args, retval, jumpval);
            result = TEST_FAILED;
        }
    }

    return result;
}

