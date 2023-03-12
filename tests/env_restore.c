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
#include <math.h>
#include <regex.h>
#include <search.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "../config.h"
#include "../env.h"
#include "../macros.h"
#include "../max.h"
#include "../str.h"
#include "lib.h"


/*
 * Constants
 */

/* Maximum number of environment variables for testing. */
#define MAX_TEST_NVARS (MAX_NVARS + 1U)

/* Maximum length of strings for testing. */
#define MAX_TEST_STR_LEN (MAX_STR_LEN * 3U)


/*
 * Data types
 */

/* Mapping of constant inputs to constant outputs. */
typedef struct {
    const char *env[MAX_TEST_NVARS];        /* Initial Environment. */
    size_t n;                               /* Number of patterns. */
    const char *patterns[MAX_TEST_NVARS];   /* Patterns. */
    const char *rst[MAX_TEST_NVARS];        /* Restored environmnet. */
    Error ret;                              /* Return value. */
} ConstArgs;

/* Mapping of run-time generated inputs to run-time generated outputs. */
typedef struct {
    char *env[MAX_TEST_NVARS];              /* Initial Environment. */
    size_t n;                               /* Number of patterns. */
    const char *patterns[MAX_TEST_NVARS];   /* Patterns. */
    char *rst[MAX_TEST_NVARS];              /* Restored environmnet. */
    Error ret;                              /* Return value. */
} DynArgs;

/* Mapping of inputs to outputs with realistc patterns. */
typedef struct {
    const char *env[MAX_TEST_NVARS];        /* Initial Environment. */
    size_t n;                               /* Number of patterns. */
    char *patterns[MAX_TEST_NVARS];         /* Patterns. */
    const char *rst[MAX_TEST_NVARS];        /* Restored environmnet. */
    Error ret;                              /* Return value. */
} RealArgs;


/*
 * Prototypes
 */

/*
 * Create a new environment and populate it with VARS.
 *
 * Return value:
 *     0  Success.
 *     1  calloc failed. errno should be set.
 */
__attribute__((nonnull(1), warn_unused_result))
static int init_env(const char *const vars[MAX_TEST_NVARS]);

/*
 * Free the memory used by the array of strings ARR, unless ARR is NULL.
 * ARR must have at most N elements and may be NULL-terminated. Elements
 * after the first NULL element are not freed, but ARR is freed regardless.
 */
static void free_arr(size_t n, char ***arr);

/*
 * Check whether the environment equals VARS.
 */
__attribute__((nonnull(1), warn_unused_result))
static bool cmp_env(const char *const vars[MAX_TEST_NVARS]);

/*
 * Create a variable the name of which is NAMELEN bytes long and that is
 * VARLEN bytes long overall, allocate memory to store that variable, and
 * return a pointer to that memory in the memory region pointed to by VARS.
 * Memory is to be freed by the caller.
 *
 * Return value:
 *     0  Success.
 *     1  calloc failed. errno should be set.
 */
__attribute__((nonnull(3), warn_unused_result))
static int mkvar(size_t namelen, size_t varlen, char **var);

/*
 * Populate the array of strings VARS with N - 1 variables and
 * terminate it with a NULL pointer.
 *
 * Return value:
 *     0  Success.
 *     1  calloc failed. errno should be set.
 *     2  snprintf failed.
 */
__attribute__((nonnull(2), warn_unused_result))
static int mkvars(size_t n, char **vars);

/* Test env_restore. */
__attribute__((nonnull(1)))
static void test(const ConstArgs *args);


/*
 * Module variables
 */

/* Simple test cases. */
static const ConstArgs cases[] = {
    /* Null tests. */
    {{NULL}, 0, {NULL}, {NULL}, OK},
    {{NULL}, 1, {"."}, {NULL}, OK},

    /* Simple tests. */
    {{"foo=foo", "bar=bar", "baz=baz", NULL}, 1, {"."},
     {"foo=foo", "bar=bar", "baz=baz", NULL}, OK},
    {{"foo=foo", "bar=bar", "baz=baz", NULL}, 1, {".*"},
     {"foo=foo", "bar=bar", "baz=baz", NULL}, OK},
    {{"foo=foo", "bar=bar", "baz=baz", NULL}, 1, {"^.*$"},
     {"foo=foo", "bar=bar", "baz=baz", NULL}, OK},
    {{"foo=foo", "bar=bar", "baz=baz", NULL}, 1, {"^foo$"},
     {"foo=foo", NULL}, OK},
    {{"foo=foo", "bar=bar", "baz=baz", "foobar=foobar", NULL}, 1, {"^foo"},
     {"foo=foo", "foobar=foobar", NULL}, OK},
    {{"foo=foo", "bar=bar", "baz=baz", "foobar=foobar", NULL}, 1, {"foo"},
     {"foo=foo", "foobar=foobar", NULL}, OK},
    {{"foo=foo", "bar=bar", "baz=baz", "foobar=foobar", NULL}, 1, {"bar$"},
     {"bar=bar", "foobar=foobar", NULL}, OK},
    {{"foo=foo", "bar=bar", "baz=baz", "foobar=foobar", NULL}, 1, {"bar"},
     {"bar=bar", "foobar=foobar", NULL}, OK},
    {{"foo=foo", "bar=bar", "baz=baz", NULL}, 0, {NULL}, {NULL}, OK},
    {{"foo=foo", "bar=bar", "baz=baz", NULL}, 1, {"^$"}, {NULL}, OK},

    /* Syntax errors. */
    {{"", "foo=", NULL}, 1, {"."}, {"foo=", NULL}, OK},
    {{"foo", "bar=", NULL}, 1, {"."}, {"bar=", NULL}, OK},
    {{"=foo", "bar=", NULL}, 1, {"."}, {"bar=", NULL}, OK},

    /* Illegal names. */
    {{" foo=foo", NULL}, 1, {"."}, {NULL}, OK},
    {{"0foo=foo", NULL}, 1, {"."}, {NULL}, OK},
    {{"*=foo", NULL}, 1, {"."}, {NULL}, OK},
    {{"foo =foo", NULL}, 1, {"."}, {NULL}, OK},
    {{"$(foo)=foo", NULL}, 1, {"."}, {NULL}, OK},
    {{"`foo`=foo", NULL}, 1, {"."}, {NULL}, OK},

    /* More realistic tests. */
    {{"bar=bar", "foo_0=foo", NULL}, 1, {"^foo_[0-9]+$"},
     {"foo_0=foo", NULL}, OK},
    {{"bar=bar", "foo_0=foo", NULL}, 1, {"^foo_(0|[1-9][0-9]*)$"},
     {"foo_0=foo", NULL}, OK},
    {{"foo_0=foo", "foo_01=foo", NULL}, 1, {"^foo_(0|[1-9][0-9]*)$"},
     {"foo_0=foo", NULL}, OK},
};

/* More realistic test cases. */
static const RealArgs realistic[] = {
    /* Null test. */
    {{NULL}, 0, {NULL}, {NULL}, OK},

    /* Simple tests. */
    {{"foo=foo", NULL}, 0, {NULL}, {NULL}, OK},
    {{"PATH_TRANSLATED=foo", NULL}, 0, {NULL},
     {"PATH_TRANSLATED=foo", NULL}, OK},
    {{"PATH_TRANSLATED=foo", "foo=foo", NULL}, 0, {NULL},
     {"PATH_TRANSLATED=foo", NULL}, OK},

    /* Illegal names. */
    {{" IPV6=foo", NULL}, 0, {NULL}, {NULL}, OK},
    {{"0IPV6=foo", NULL}, 0, {NULL}, {NULL}, OK},
    {{"*=foo", NULL}, 0, {NULL}, {NULL}, OK},
    {{"IPV6 =foo", NULL}, 0, {NULL}, {NULL}, OK},
    {{"$(IPV6)=foo", NULL}, 0, {NULL}, {NULL}, OK},
    {{"`IPV6`=foo", NULL}, 0, {NULL}, {NULL}, OK},

    /* Odd but legal values. */
    {{"SSL_CLIENT_S_DN_C_0=", "SSL_CLIENT_S_DN_C_1==",
      "SSL_CLIENT_S_DN_C_2= ", "SSL_CLIENT_S_DN_C_3=\t",
      "SSL_CLIENT_S_DN_C_4=\n", NULL}, 0, {NULL},
     {"SSL_CLIENT_S_DN_C_0=", "SSL_CLIENT_S_DN_C_1==",
      "SSL_CLIENT_S_DN_C_2= ", "SSL_CLIENT_S_DN_C_3=\t",
      "SSL_CLIENT_S_DN_C_4=\n", NULL}, OK},

    /* Real-world tests. */
    {{"PWD=/home/jdoe", "SHELL=/bin/zsh", "PATH=/bin:/usr/bin",
      "HOME=/home/jdoe", "USER=jdoe", "LOGNAME=jdoe", "OLDPWD=/home",
      "LANG=en_GB.UTF-8", "EDITOR=vim", "VISUAL=vim", "PAGER=less",
      "CLICOLOR=x", "IFS=", NULL},
     0, {NULL}, {NULL}, OK},
    {{"PWD=/home/jdoe", "SHELL=/bin/zsh", "PATH=/bin:/usr/bin",
      "HOME=/home/jdoe", "TMPDIR=/tmp/user/1000", "USER=jdoe",
      "LOGNAME=jdoe", "OLDPWD=/home", "LANG=en_GB.UTF-8", "EDITOR=vim",
      "VISUAL=vim", "PAGER=less", "CLICOLOR=x", "IFS=",
      "DOCUMENT_ROOT=/home/jdoe/public_html",
      "HTTP_HOST=www.foo.example", "HTTP_REFERER=https://www.bar.example",
      "HTTP_USER_AGENT=FakeZilla/1", "HTTPS=on",
      "QUERY_STRING=foo=bar&bar=baz", "REMOTE_ADDR=100::1:2:3",
      "REMOTE_HOST=100::1:2:3", "REMOTE_PORT=50000", "REQUEST_METHOD=GET"
      "REQUEST_URI=/index.php",
      "PATH_TRANSLATED=/home/jdoe/public_html/index.php",
      "SCRIPT_NAME=index.php", "SERVER_ADMIN=admin@foo.example",
      "SERVER_NAME=www.foo.example", "SERVER_PORT=443",
      "SERVER_SOFTWARE=Apache v2.4", NULL}, 0, {NULL},
     {"HTTP_HOST=www.foo.example", "HTTP_REFERER=https://www.bar.example",
      "HTTP_USER_AGENT=FakeZilla/1", "HTTPS=on",
      "QUERY_STRING=foo=bar&bar=baz", "REMOTE_ADDR=100::1:2:3",
      "REMOTE_HOST=100::1:2:3", "REMOTE_PORT=50000", "REQUEST_METHOD=GET"
      "REQUEST_URI=/index.php",
      "PATH_TRANSLATED=/home/jdoe/public_html/index.php",
      "SCRIPT_NAME=index.php", "SERVER_ADMIN=admin@foo.example",
      "SERVER_NAME=www.foo.example", "SERVER_PORT=443",
      "SERVER_SOFTWARE=Apache v2.4", NULL}, OK}
};


/*
 * Functions
 */

static int
init_env(const char *const vars[MAX_TEST_NVARS])
{
    size_t n;   /* Number of variables given. */

    n = 0;
    while (n < MAX_TEST_NVARS && vars[n] != NULL) {
        ++n;
    }

    environ = calloc(n + 1U, sizeof(*environ));
    if (environ == NULL) {
       return 1;
    }

    for (size_t i = 0; i < n; ++i) {
        size_t len;     /* Length of variable. */

        len = strnlen(vars[i], MAX_TEST_STR_LEN);
        assert(len < MAX_TEST_STR_LEN);

        environ[i] = calloc(len + 1U, sizeof(*environ[i]));
        if (environ[i] == NULL) {

            free_arr(MAX_TEST_NVARS, &environ);
            return 1;
        }

        (void) stpncpy(environ[i], vars[i], len + 1U);
    }

    return 0;
}

static void
free_arr(size_t n, char ***arr)
{
    if (*arr != NULL) {
        for (size_t i = 0; i < n && *arr[i]; ++i) {
            free(*arr[i]);
            *arr[i] = NULL;
        }

        free(*arr);
        *arr = NULL;
    }
}

static bool
cmp_env(const char *const vars[MAX_TEST_NVARS])
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

static int mkvar(size_t namelen, size_t varlen, char **var) {
    assert(namelen < varlen);

    *var = calloc((size_t) varlen, sizeof(**var));
    if (*var == NULL) {
        return 1;
    }

    fillstr('x', varlen, *var);
    (*var)[namelen] = '=';

    return 0;
}

static int
mkvars(size_t n, char **vars)
{
    static const char fmt[] = "v%zu=";
    size_t len;
    size_t lst;

    assert(n < SHRT_MAX);
    lst = n - 1U;
    len = (size_t) log10((double) n) + sizeof(fmt) + 1U;

    for (size_t i = 0; i < lst; ++i) {
        int m;

        vars[i] = calloc(len, sizeof(*vars[i]));
        if(vars[i] == NULL) {
            free_arr(n, &vars);
            return 1;
        }

        m = snprintf(vars[i], len, fmt, i);
        if (m < 0 || (size_t) m > len) {
            return 2;
        }
    }

    vars[lst] = NULL;

    return 0;
}

static void
test(const ConstArgs *args)
{
    static char *null = NULL;
    char patbuf[MAX_TEST_STR_LEN];
    char envbuf[MAX_TEST_STR_LEN];
    char rstbuf[MAX_TEST_STR_LEN];
    regex_t pregs[MAX_TEST_STR_LEN];
    char **vars;
    int rc;
    Error ret;

    assert(args != NULL);

    rc = joinstrs(args->n, args->patterns, ", ", sizeof(patbuf), patbuf);
    assert(rc == 0);
    rc = joinstrs(NELEMS(args->env), args->env, " ", sizeof(envbuf), envbuf);
    assert(rc == 0);
    rc = joinstrs(NELEMS(args->rst), args->rst, " ", sizeof(rstbuf), rstbuf);
    assert(rc == 0);

    warnx("checking %s => (%zu, {%s}) -> %u => %s ...",
          envbuf, args->n, patbuf, args->ret, rstbuf);

    if (init_env(args->env) != 0) {
        err(TEST_ERROR, "calloc");
    }

    vars = environ;
    environ = &null;

    for (size_t j = 0; j < args->n && j < MAX_TEST_NVARS; ++j) {
        rc = regcomp(&pregs[j], args->patterns[j], REG_EXTENDED | REG_NOSUB);
        if (rc != 0) {
            char message[MAX_ERRMSG_LEN];
            size_t len;

            len = regerror(rc, &pregs[j], message, sizeof(message));
            if (len > sizeof(message)) {
                warnx("regular expression error has been truncated.");
            }

            errx(TEST_ERROR, "regcomp: %s", message);
        }
    }

    ret = env_restore(vars, args->n, pregs);
    if (ret != args->ret) {
        errx(TEST_FAILED, "returned error %u", args->ret);
    }

    for (size_t i = 0; vars[i]; ++i) {
        free(vars[i]);
        vars[i] = NULL;
    }
    free(vars);
    vars = NULL;

    if (ret == OK && !cmp_env(args->rst)) {
        char buf[MAX_TEST_STR_LEN];

        rc = joinstrs(MAX_TEST_NVARS, (const char *const *) environ, " ",
                       sizeof(buf), buf);
        assert(rc == 0);
        errx(TEST_FAILED, "restored environment: %s", buf);
    }
}


/*
 * Main
 */

int
main (void) {
    const char *const env_patterns[] = ENV_PATTERNS;
    DynArgs max_nvars = {{NULL}, 1, {"."}, {NULL}, OK};
    DynArgs err_nvars = {{NULL}, 1, {"."}, {NULL}, ERR_LEN};
    DynArgs max_name = {{NULL}, 1, {"."}, {NULL}, OK};
    DynArgs err_name = {{NULL}, 1, {"."}, {NULL}, OK};
    DynArgs max_var = {{NULL}, 1, {"."}, {NULL}, OK};
    DynArgs err_var = {{NULL}, 1, {"."}, {NULL}, OK};
    int rc;

    openlog("env_restore", LOGGING_OPTIONS, LOG_AUTH);

    /* Maximum variables. */
    assert(MAX_NVARS <= sizeof(max_nvars.env));
    rc = mkvars(MAX_NVARS, max_nvars.env);
    if (rc != 0) {
        errx(TEST_ERROR, "mkvars returned %d", rc);
    }

    assert(MAX_NVARS <= sizeof(max_nvars.rst));
    rc = mkvars(MAX_NVARS, max_nvars.rst);
    if (rc != 0) {
        errx(TEST_ERROR, "mkvars returned %d", rc);
    }

    test((const ConstArgs *) &max_nvars);

    /* Too many variables. */
    assert(MAX_NVARS + 1U <= sizeof(err_nvars.env));
    rc = mkvars(MAX_NVARS + 1U, err_nvars.env);
    if (rc != 0) {
        errx(TEST_ERROR, "mkvars returned %d", rc);
    }

    test((const ConstArgs *) &err_nvars);

    /* Long name. */
    rc = mkvar(MAX_VARNAME_LEN - 1U, MAX_VARNAME_LEN + 1U, &max_name.env[0]);
    if (rc != 0) {
        err(TEST_ERROR, "calloc");
    }

    max_name.rst[0] = strndup(max_name.env[0], MAX_VARNAME_LEN + 1U);
    if (max_name.rst[0] == NULL) {
        err(TEST_ERROR, "strndup");
    }

    test((const ConstArgs *) &max_name);

    /* Too long name. */
    rc = mkvar(MAX_VARNAME_LEN, MAX_VARNAME_LEN + 2U, &err_name.env[0]);
    if (rc != 0) {
        err(TEST_ERROR, "calloc");
    }

    test((const ConstArgs *) &err_name);

    /* Long variable. */
    rc = mkvar(MAX_VARNAME_LEN - 1U, MAX_VAR_LEN, &max_var.env[0]);
    if (rc != 0) {
        err(TEST_ERROR, "calloc");
    }

    max_var.rst[0] = strndup(max_var.env[0], MAX_VAR_LEN);
    if (max_var.rst[0] == NULL) {
        err(TEST_ERROR, "strndup");
    }

    test((const ConstArgs *) &max_var);

    /* Too long variable. */
    rc = mkvar(MAX_VARNAME_LEN - 1U, MAX_VAR_LEN + 1U, &err_var.env[0]);
    if (rc != 0) {
        err(TEST_ERROR, "calloc");
    }

    test((const ConstArgs *) &err_var);

    /* Static test cases. */
    for (size_t i = 0; i < NELEMS(cases); ++i) {
        test(&cases[i]);
    }

    /* Realistic test cases. */
    for (size_t i = 0; i < NELEMS(realistic); ++i) {
        RealArgs arg;

        (void) memcpy(&arg, &realistic[i], sizeof(arg));
        (void) memcpy(&arg.patterns, &env_patterns,
                      NELEMS(env_patterns) * sizeof(*env_patterns));
        arg.n = NELEMS(env_patterns);

        test((const ConstArgs *) &arg);
    }

    warnx("all tests passed");
    return EXIT_SUCCESS;
}

