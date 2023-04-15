/*
 * Test userdirexp.
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
#include <limits.h>
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../macros.h"
#include "../max.h"
#include "../str.h"
#include "../userdir.h"
#include "check.h"


/*
 * Constants
 */

#if !defined(LOGIN_NAME_MAX) || LOGIN_NAME_MAX < _POSIX_LOGIN_NAME_MAX
#define MAX_LOGNAME_LEN 256
#else
#define MAX_LOGNAME_LEN LOGIN_NAME_MAX
#endif


/*
 * Data types
 */

/* A mapping of arguments to return values. */
typedef struct {
    const char *const str;
    const struct passwd *const user;
    const char *const dir;
    const Error retval;
    int signo;
} Args;


/*
 * Module variables
 */

#if !defined(NDEBUG)
/* A filename that that exceeds MAX_FNAME_LEN. */
static char xhugefname[MAX_FNAME_LEN + 1U] = {0};
#endif

/* A filename just within MAX_FNAME_LEN */
static char xlongfname[MAX_FNAME_LEN] = {0};

/* A long relative filename. */
char longrelfname[MAX_FNAME_LEN];

/* A long absolute filename. */
char longabsname[MAX_FNAME_LEN];

/* A long user directory pattern. */
char longpattern[MAX_FNAME_LEN];

/* A fake login name. */
char logname[MAX_LOGNAME_LEN] = "jdoe";

/* A fake home directory. */
char homedir[MAX_FNAME_LEN] = "/home/jdoe";

/* A fake user account. */
const struct passwd user = {
    .pw_name = logname,
    .pw_dir = homedir
};

/* Test cases. */
static const Args cases[] = {
    /* Illegal arguments. */
#if !defined(NDEBUG)
    {xhugefname, &user, NULL, ERR_LEN, SIGABRT},
#endif

    /* Long filename. */
    {xlongfname, &user, NULL, ERR_LEN, 0},

    /* Simple tests. */
    {"public_html", &user, "/home/jdoe/public_html", OK, 0},
    {"/srv/www", &user, "/srv/www/jdoe", OK, 0},
    {"/srv/www/%s/html", &user, "/srv/www/jdoe/html", OK, 0},

    /* Spaces. */
    {"Web docs", &user, "/home/jdoe/Web docs", OK, 0},
    {"/Server files/Web docs", &user, "/Server files/Web docs/jdoe", OK, 0},
    {"/User files/%s/Web docs", &user, "/User files/jdoe/Web docs", OK, 0},

    /* UTF-8. */
    {"ⓟů𝕓ḹḭⓒ﹏𝒽𝚝ṃḹ", &user, "/home/jdoe/ⓟů𝕓ḹḭⓒ﹏𝒽𝚝ṃḹ", OK, 0},
    {"/𝘴ȑṽ/𝙬𝙬𝙬", &user, "/𝘴ȑṽ/𝙬𝙬𝙬/jdoe", OK, 0},
    {"/𝘴ȑṽ/𝙬𝙬𝙬/%s/𝒽𝚝ṃḹ", &user, "/𝘴ȑṽ/𝙬𝙬𝙬/jdoe/𝒽𝚝ṃḹ", OK, 0},

    /* Long filenames. */
    {longrelfname, &user, NULL, ERR_LEN, 0},
    {longabsname, &user, NULL, ERR_LEN, 0},
    {longpattern, &user, NULL, ERR_LEN, 0},
};


/*
 * Main
 */

int
main(void)
{
    volatile int result = TEST_PASSED;

    checkinit();

#if !defined(NDEBUG)
    (void) memset(xhugefname, 'x', sizeof(xhugefname) - 1U);
#endif
    (void) memset(xlongfname, 'x', sizeof(xlongfname) - 1U);
    (void) memset(longrelfname, 'x', sizeof(longrelfname) - 1U);
    (void) memset(longabsname, 'x', sizeof(longabsname) - 1U);
    longabsname[0] = '/';

    (void) memset(longpattern, 'x', sizeof(longpattern) - 1U);
    (void) copystr(4, "/%s", &longpattern[sizeof(longpattern) - 4U]);
    longpattern[0] = '/';

    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];
        char dir[MAX_FNAME_LEN];
        int jumpval;
        Error retval;

        (void) memset(dir, '\0', MAX_FNAME_LEN);

        jumpval = sigsetjmp(checkenv, true);

        if (jumpval == 0) {
/* args.str is not a literal. */
#if defined(__GNUC__) && __GNUC__ >= 3
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif
            checking = 1;
            retval = userdirexp(args.str, args.user, dir);
            checking = 0;

#if defined(__GNUC__) && __GNUC__ >= 3
#pragma GCC diagnostic pop
#endif

            if (retval != args.retval) {
                warnx("(%s, %s, → %s) → %u [!]",
                      args.str, args.user->pw_name, dir, retval);
                result = TEST_FAILED;
            }

            if (retval == OK && strcmp(args.dir, dir) != 0) {
                warnx("(%s, %s, → %s [!]) → %u",
                      args.str, args.user->pw_name, dir, retval);
                result = TEST_FAILED;
            }
        }

        if (jumpval != args.signo) {
            warnx("(%s, %s, → %s) ↑ %s [!]",
                  args.str, args.user->pw_name, dir, strsignal(jumpval));
            result = TEST_FAILED;
        }
    }

    return result;
}
