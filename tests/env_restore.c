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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../env.h"
#include "../config.h"
#include "../macros.h"
#include "../str.h"
#include "result.h"



/*
 * Constants
 */

/* Maximum number of environment variables for testing. */
#define MAX_TEST_NVARS (MAX_NVARS + 1U)

/* Maximum length of strings for testing. */
#define MAX_TEST_STR_LEN (MAX_STR_LEN + 1U)


/*
 * Macros
 */

/*
 * Join the given STRS using SEP and store the result in ARR, which must be
 * an array. There must be at most NSTRS strings. See joinstrs for details.
 */
#define JOINSTRS(nstrs, strs, sep, arr) \
    do { joinstrs(nstrs, strs, sep, sizeof(arr), arr); } while (0);


/*
 * Data types
 */

/* Mapping of constant inputs to constant outputs. */
typedef struct {
    char **vars;                    /* Variables to set. */
    size_t npatterns;               /* Number of patterns. */
    char **patterns;                /* Patterns. */
    char *env[MAX_TEST_NVARS];		/* Resulting environmnet. */
    Error ret;                      /* Return value. */
} Args;

/* Types of test errors. */
typedef enum {
	WRONG_RETVAL,
	WRONG_ENVIRON
} ErrType;


/*
 * Module variables
 */

/* See config.h. */
const char *const envpatterns[] = ENV_PATTERNS;

/* Too many variables. */
static char *toomanyvars[MAX_NVARS + 1U] = {NULL};

/* A variable that is as long as permitted. */
static char longvar[MAX_VAR_LEN] = {'\0'};

/* A variable that is too long. */
static char hugevar[MAX_VAR_LEN + 1U] = {'\0'};

/* A variable with a name that is as long as is permitted. */
static char longname[MAX_VAR_LEN] = {'\0'};

/* A variable with a name that is too long. */
static char hugename[MAX_VAR_LEN] = {'\0'};

/*
 * I am mixing string literals, arrays, and pointers for convenience;
 * or else I would have to define a lot of struct types.
 */
#if defined(__GNUC__)
#pragma GCC diagnostic push
#if __GNUC__ > 5 || (__GNUC__ == 5 && __GNUC_MINOR__ >= 1)
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#endif
#if __GNUC__ > 6 || (__GNUC__ == 6 && __GNUC_MINOR__ >= 1)
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
#endif
#endif /* defined(__GNUC__) */
#if defined(__clang__)
#pragma clang diagnostic push
#if __clang_major__ >= 4
#pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
#endif
#endif /* defined(__clang__) */
#if defined(__INTEL_COMPILER)
#pragma warning(disable: 2330 2331 3179)
#endif /* defined(__INTEL_COMPILER) */

/* Test cases. */
static const Args cases[] = {
    /* Null tests. */
    {(char *[]) {NULL}, 0, (char *[]) {NULL}, {NULL}, OK},
    {(char *[]) {NULL}, 1, (char *[]) {"."}, {NULL}, OK},

	/* Too many variables. */
	{toomanyvars, 1, (char *[]) {"."}, {NULL}, ERR_LEN},

	/* Overly long variables should be ignored. */
    {(char *[]) {longvar, NULL}, 1, (char *[]) {"."}, {longvar, NULL}, OK},
    {(char *[]) {hugevar, NULL}, 1, (char *[]) {"."}, {NULL}, OK},

	/* Variables with overly long should be ignored, too. */
    {(char *[]) {longname, NULL}, 1, (char *[]) {"."}, {longname, NULL}, OK},
	{(char *[]) {hugename, NULL}, 1, (char *[]) {"."}, {NULL}, OK},

    /* Simple tests. */
    {
        (char *[]) {"foo=foo", NULL},
        1, (char *[]) {"."},
        {"foo=foo", NULL}, OK
    },
    {
        (char *[]) {"foo=foo", "bar=bar", NULL},
        1, (char *[]) {"^foo$"},
        {"foo=foo", NULL}, OK
    },
    {
        (char *[]) {"foo=foo", "bar=bar", "baz=baz", "foobar=foobar", NULL},
        1, (char *[]) {"^foo"},
        {"foo=foo", "foobar=foobar", NULL}, OK
    },
    {
        (char *[]) {"foo=foo", "bar=bar", "baz=baz", "foobar=foobar", NULL},
        1, (char *[]) {"foo"},
        {"foo=foo", "foobar=foobar", NULL}, OK
    },
    {
        (char *[]) {"foo=foo", "bar=bar", "baz=baz", "foobar=foobar", NULL},
        1, (char *[]) {"bar$"},
        {"bar=bar", "foobar=foobar", NULL}, OK
    },
    {
        (char *[]) {"foo=foo", "bar=bar", "baz=baz", "foobar=foobar", NULL},
        1, (char *[]) {"bar"},
        {"bar=bar", "foobar=foobar", NULL}, OK
    },
    {
       (char *[])  {"foo=foo", "bar=bar", "baz=baz", NULL},
        0, (char *[]) {NULL},
        {NULL}, OK
    },
    {
        (char *[]) {"foo=foo", "bar=bar", "baz=baz", NULL},
        1, (char *[]) {"^$"},
        {NULL}, OK
    },

    /* Syntax errors. */
    {(char *[]) {"", NULL}, 1, (char *[]) {"."}, {NULL}, OK},
    {(char *[]) {"foo", NULL}, 1, (char *[]) {"."}, {NULL}, OK},
    {(char *[]) {"=foo", NULL}, 1, (char *[]) {"."}, {NULL}, OK},

    /* Illegal names. */
    {(char *[]) {" foo=foo", NULL}, 1, (char *[]) {"."}, {NULL}, OK},
    {(char *[]) {"0foo=foo", NULL}, 1, (char *[]) {"."}, {NULL}, OK},
    {(char *[]) {"*=foo", NULL}, 1, (char *[]) {"."}, {NULL}, OK},
    {(char *[]) {"foo =foo", NULL}, 1, (char *[]) {"."}, {NULL}, OK},
    {(char *[]) {"$(foo)=foo", NULL}, 1, (char *[]) {"."}, {NULL}, OK},
    {(char *[]) {"`foo`=foo", NULL}, 1, (char *[]) {"."}, {NULL}, OK},

    /* More realistic tests. */
    {
        (char *[]) {"bar=bar", "foo_0=foo", NULL},
        1, (char *[]) {"^foo_[0-9]+$"},
        {"foo_0=foo", NULL}, OK
    },
    {
        (char *[]) {"bar=bar", "foo_0=foo", NULL},
        1, (char *[]) {"^foo_(0|[1-9][0-9]*)$"},
        {"foo_0=foo", NULL}, OK
    },
    {
        (char *[]) {"foo_0=foo", "foo_01=foo", NULL},
        1, (char *[]) {"^foo_(0|[1-9][0-9]*)$"},
        {"foo_0=foo", NULL}, OK
    },

    /* Simple tests with real patterns. */
    {
        (char *[]) {"foo=foo", NULL},
        NELEMS(envpatterns), envpatterns,
        {NULL}, OK
    },
    {
        (char *[]) {"PATH_TRANSLATED=foo", NULL},
        NELEMS(envpatterns), envpatterns,
        {"PATH_TRANSLATED=foo", NULL}, OK
    },
    {
        (char *[]) {"PATH_TRANSLATED=foo", "foo=foo", NULL},
        NELEMS(envpatterns), envpatterns,
        {"PATH_TRANSLATED=foo", NULL}, OK
    },

    /* Illegal names. */
    {
    	(char *[]) {" IPV6=foo", NULL},
    	NELEMS(envpatterns), envpatterns,
    	{NULL}, OK
    },
    {
    	(char *[]) {"0IPV6=foo", NULL},
    	NELEMS(envpatterns), envpatterns,
    	{NULL}, OK
    },
    {
    	(char *[]) {"*=foo", NULL},
    	NELEMS(envpatterns), envpatterns,
    	{NULL}, OK
    },
    {
    	(char *[]) {"IPV6 =foo", NULL},
    	NELEMS(envpatterns), envpatterns,
    	{NULL}, OK
    },
    {
    	(char *[]) {"$(IPV6)=foo", NULL},
    	NELEMS(envpatterns), envpatterns,
    	{NULL}, OK
    },
    {
    	(char *[]) {"`IPV6`=foo", NULL},
    	NELEMS(envpatterns), envpatterns,
    	{NULL}, OK
    },

    /* Odd but legal values. */
    {
    	(char *[]) {
        	"SSL_CLIENT_S_DN_C_0=", "SSL_CLIENT_S_DN_C_1==",
            "SSL_CLIENT_S_DN_C_2= ", "SSL_CLIENT_S_DN_C_3=\t",
            "SSL_CLIENT_S_DN_C_4=\n", NULL
        },
        NELEMS(envpatterns), envpatterns,
        {
        	"SSL_CLIENT_S_DN_C_0=", "SSL_CLIENT_S_DN_C_1==",
            "SSL_CLIENT_S_DN_C_2= ", "SSL_CLIENT_S_DN_C_3=\t",
            "SSL_CLIENT_S_DN_C_4=\n", NULL
        }, OK
    },

    /* Real-world tests. */
    {
        (char *[]) {
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
        {NULL}, OK
    },
    {
        (char *[]) {
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
        {
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
        }, OK
	}
};

#if defined(__clang__)
#pragma clang diagnostic pop
#endif
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif


/*
 * Prototypes
 */

/*
 * Report an error of ERRTYPE for ARGS with return value RET.
 */
__attribute__((nonnull(1)))
static void report(const Args *args, Error ret, ErrType errtype);

/*
 * Check whether the environment equals VARS.
 */
__attribute__((nonnull(1), warn_unused_result))
static bool cmpenv(char *const vars[MAX_TEST_NVARS]);

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
static void joinstrs(size_t nstrs, char **strs, char *sep,
                     size_t size, char *dest);


/*
 * Functions
 */

static bool
cmpenv(char *const vars[MAX_TEST_NVARS])
{
    for (size_t i = 0; i < MAX_TEST_NVARS; ++i) {
        if (environ[i] == NULL && vars[i] == NULL) {
            return true;
        }

        if (environ[i] == NULL || vars[i] == NULL) {
            return false;
        }

        if (strncmp(environ[i], vars[i], MAX_TEST_STR_LEN) != 0) {
            return false;
        }
    }

    return false;
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
joinstrs(const size_t nstrs, char **const strs, char *const sep,
         const size_t size, char *const dest)
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

static void
report(const Args *const args, const Error ret, const ErrType errtype)
{
	char varstr[MAX_TEST_STR_LEN];
	char patstr[MAX_TEST_STR_LEN];
	char envstr[MAX_TEST_STR_LEN];
	char environstr[MAX_TEST_STR_LEN];

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
#if defined(__clang__) && __clang__
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
#endif
	JOINSTRS(MAX_TEST_NVARS, args->vars, ", ", varstr);
	JOINSTRS(args->npatterns, args->patterns, ", ", patstr);
	JOINSTRS(MAX_TEST_NVARS, args->env, " ", envstr);
	JOINSTRS(MAX_TEST_NVARS, environ, " ", environstr);
#pragma GCC diagnostic pop

	switch (errtype) {
	case WRONG_RETVAL:
		warnx("({%s}, %zu, {%s}) -> %u [!] => %s",
			  varstr, args->npatterns, patstr, ret, envstr);
		break;
	case WRONG_ENVIRON:
		warnx("({%s}, %zu, {%s}) -> %u => %s [!]",
			  varstr, args->npatterns, patstr, ret, environstr);
		break;
	default:
		/* Unreachable. */
		abort();
	}
}


/*
 * Main
 */

int
main (void) {
    char *null = NULL;
    int result = TEST_PASSED;

	for (size_t i = 0; i < NELEMS(toomanyvars); ++i) {
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

	(void) memset(longvar, 'x', sizeof(longvar) - 1U);
	(void) str_cp(4, "var=", longvar);

	(void) memset(hugevar, 'x', sizeof(hugevar) - 1U);
	(void) str_cp(4, "var=", longvar);

	(void) memset(longname, 'x', sizeof(longname) - 1U);
	longname[MAX_VARNAME_LEN - 1U] = '=';

	(void) memset(hugename, 'x', sizeof(hugename) - 1U);
	hugename[MAX_VARNAME_LEN] = '=';

    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];
        regex_t pregs[MAX_TEST_NVARS];
        Error ret;

        environ = &null;

        for (size_t j = 0; j < args.npatterns; ++j) {
            int err;

            err = regcomp(&pregs[j], args.patterns[j],
                          REG_EXTENDED | REG_NOSUB);
            if (err != 0) {
                char errmsg[MAX_ERRMSG_LEN];
                size_t errmsglen;

                errmsglen = regerror(err, &pregs[j], errmsg, sizeof(errmsg));
                if (errmsglen > sizeof(errmsg)) {
                    warnx("regular expression error truncated.");
                }

                errx(EXIT_FAILURE, "regcomp: %s", errmsg);
            }
        }

        ret = env_restore(args.vars, args.npatterns, pregs);

        if (ret != args.ret) {
        	report(&args, ret, WRONG_RETVAL);
            result = TEST_FAILED;
        }

        if (ret == OK && !cmpenv(args.env)) {
        	report(&args, ret, WRONG_ENVIRON);
            result = TEST_FAILED;
        }
    }

    return result;
}

