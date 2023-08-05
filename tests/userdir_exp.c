/*
 * Test userdir_exp.
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
#include <limits.h>
#include <setjmp.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "../macros.h"
#include "../params.h"
#include "../str.h"
#include "../userdir.h"
#include "util/abort.h"
#include "util/types.h"


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
    int signal;
} UserDirExpArgs;


/*
 * Main
 */

int
main(void)
{
#if !defined(NDEBUG)

    /* RATS: ignore; used safely. */
    char hugefname[MAX_FNAME_LEN + 1U] = {0};
#endif

    /* RATS: ignore; used safely. */
    char longrelfname[MAX_FNAME_LEN] = {0};

    /* RATS: ignore; used safely. */
    char longabsname[MAX_FNAME_LEN] = {0};

    /* RATS: ignore; used safely. */
    char longpattern[MAX_FNAME_LEN] = {0};

    /* RATS: ignore; used safely. */
    char logname[MAX_LOGNAME_LEN] = "jdoe";

    /* RATS: ignore; used safely. */
    char homedir[MAX_FNAME_LEN] = "/home/jdoe";

    const struct passwd user = {
        .pw_name = logname,
        .pw_dir = homedir
    };

    const UserDirExpArgs cases[] = {
        /* Illegal arguments. */
#if !defined(NDEBUG)
        {hugefname, &user, NULL, ERR_LEN, SIGABRT},
#endif

        /* Illegal formats. */
        {"/%/truncated", &user, NULL, ERR_BAD, 0},
        {"/%d/notastring", &user, NULL, ERR_BAD, 0},
        {"/%ö/notaflag", &user, NULL, ERR_BAD, 0},
        {"/%04s/legal, but forbidden", &user, NULL, ERR_BAD, 0},
        {"/%1$s/legal, but forbidden", &user, NULL, ERR_BAD, 0},
        {"/too/%s/many/%s/specs", &user, NULL, ERR_BAD, 0},

        /* Simple tests. */
        {"public_html", &user, "/home/jdoe/public_html", OK, 0},
        {"public_%s", &user, "/home/jdoe/public_%s", OK, 0},
        {"/srv/www", &user, "/srv/www/jdoe", OK, 0},
        {"/srv/www/%s/html", &user, "/srv/www/jdoe/html", OK, 0},
        {"/srv/www/%%s/%s/html", &user, "/srv/www/%s/jdoe/html", OK, 0},

        /* Spaces. */
        {"Web docs", &user, "/home/jdoe/Web docs", OK, 0},
        {"/Server files/Web docs", &user, "/Server files/Web docs/jdoe", OK, 0},
        {"/User files/%s/Web docs", &user, "/User files/jdoe/Web docs", OK, 0},
        {"/User files/%s/%%s/Web docs", &user, "/User files/jdoe/%s/Web docs", OK, 0},

        /* UTF-8. */
        {"ⓟů𝕓ḹḭⓒ﹏𝒽𝚝ṃḹ", &user, "/home/jdoe/ⓟů𝕓ḹḭⓒ﹏𝒽𝚝ṃḹ", OK, 0},
        {"ⓟů𝕓ḹḭⓒ﹏%s", &user, "/home/jdoe/ⓟů𝕓ḹḭⓒ﹏%s", OK, 0},
        {"/𝘴ȑṽ/𝙬𝙬𝙬", &user, "/𝘴ȑṽ/𝙬𝙬𝙬/jdoe", OK, 0},
        {"/𝘴ȑṽ/𝙬𝙬𝙬/%s/𝒽𝚝ṃḹ", &user, "/𝘴ȑṽ/𝙬𝙬𝙬/jdoe/𝒽𝚝ṃḹ", OK, 0},
        {"/𝘴ȑṽ/𝙬𝙬𝙬/%%s/%s/𝒽𝚝ṃḹ", &user, "/𝘴ȑṽ/𝙬𝙬𝙬/%s/jdoe/𝒽𝚝ṃḹ", OK, 0},
        {"/%/𝓉𝐫ṷ𝖓𝓬⒜𝓉ḕⅆ", &user, NULL, ERR_BAD, 0},
        {"/%d/𝖓º𝓉⒜𝖘𝓉𝐫ĩ𝖓𝘨", &user, NULL, ERR_BAD, 0},
        {"/%ö/𝖓º𝓉⒜ⓕℓ⒜𝘨", &user, NULL, ERR_BAD, 0},
        {"/%04s/ℓḕ𝘨⒜ℓ, ḇṷ𝓉 ⓕº𝐫ḇĩⓓⓓḕ𝖓", &user, NULL, ERR_BAD, 0},
        {"/%1$s/ℓḕ𝘨⒜ℓ, ḇṷ𝓉 ⓕº𝐫ḇĩⓓⓓḕ𝖓", &user, NULL, ERR_BAD, 0},
        {"/𝓉ºº/%s/m⒜𝖓⒴/%s/spḕ𝓬𝖘", &user, NULL, ERR_BAD, 0},

        /* Long filenames. */
        {longrelfname, &user, NULL, ERR_LEN, 0},
        {longabsname, &user, NULL, ERR_LEN, 0},
        {longpattern, &user, NULL, ERR_LEN, 0}
    };

    /* Test result. */
    volatile int result = PASS;

#if !defined(NDEBUG)
    (void) memset(hugefname, 'x', sizeof(hugefname) - 1U);
#endif
    (void) memset(longrelfname, 'x', sizeof(longrelfname) - 1U);
    (void) memset(longabsname, 'x', sizeof(longabsname) - 1U);
    longabsname[0] = '/';

    (void) memset(longpattern, 'x', sizeof(longpattern) - 1U);
    /* RATS: ignore; the buffer is large enough. */
    (void) strncpy(&longpattern[sizeof(longpattern) - 4U], "/%s", 4);
    longpattern[0] = '/';

    for (volatile size_t i = 0; i < NELEMS(cases); ++i) {
        const UserDirExpArgs args = cases[i];

        if (sigsetjmp(abort_env, 1) == 0) {
            /* RATS: ignore; used safely. */
            char dir[MAX_FNAME_LEN];
            size_t dirlen;
            Error retval;

            (void) memset(dir, '\0', MAX_FNAME_LEN);

            if (args.signal != 0) {
                warnx("the next test should fail an assertion.");
            }

/* args.str is not a literal, but that's okay. */
#if defined(__GNUC__) && __GNUC__ >= 3
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif

            (void) abort_catch(err);
            retval = userdir_exp(args.str, args.user, sizeof(dir),
                                 &dirlen, dir);
            (void) abort_reset(err);

#if defined(__GNUC__) && __GNUC__ >= 3
#pragma GCC diagnostic pop
#endif

            if (retval != args.retval) {
                result = FAIL;
                warnx("%zu: (%s, %s, %zu, → %zu, → %s) → %u [!]",
                      i, args.str, args.user->pw_name, sizeof(dir),
                      dirlen, dir, retval);
            }

            if (retval == OK) {
                assert(args.dir != NULL);

                if (strnlen(dir, MAX_FNAME_LEN) != dirlen ||
                    dirlen >= (size_t) MAX_FNAME_LEN)
                {
                    result = FAIL;
                    warnx("%zu: (%s, %s, %zu, → %zu [!], → %s) → %u",
                          i, args.str, args.user->pw_name, sizeof(dir),
                          dirlen, dir, retval);
                }


                if (strcmp(args.dir, dir) != 0) {
                    result = FAIL;
                    warnx("%zu: (%s, %s, %zu, → %zu, → %s [!]) → %u",
                          i, args.str, args.user->pw_name, sizeof(dir),
                          dirlen, dir, retval);
                }
            }

        }

        if (abort_signal != args.signal) {
            result = FAIL;
            warnx("%zu: (%s, %s, → <dir>) ↑ %s [!]",
                  i, args.str, args.user->pw_name, strsignal(abort_signal));
        }
    }

    return result;
}
