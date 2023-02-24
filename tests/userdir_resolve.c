/*
 * Test userdir_resolve.
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

#include <err.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "../macros.h"
#include "../max.h"
#include "../str.h"
#include "../userdir.h"
#include "lib.h"


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
    const char *const s;
    const struct passwd *const user;
    const char *const dir;
    const Error ret;
} Args;


/*
 * Module variables
 */

/* A long relative filename. */
char long_rel_fname[MAX_FNAME_LEN];

/* A long absolute filename. */
char long_abs_fname[MAX_FNAME_LEN];

/* A long user directory pattern. */
char long_pattern[MAX_FNAME_LEN];

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
    /* Simple tests. */
    {"public_html", &user, "/home/jdoe/public_html", OK},
    {"/srv/www", &user, "/srv/www/jdoe", OK},
    {"/srv/www/%s/html", &user, "/srv/www/jdoe/html", OK},

    /* Spaces. */
    {"Web docs", &user, "/home/jdoe/Web docs", OK},
    {"/Server files/Web docs", &user, "/Server files/Web docs/jdoe", OK},
    {"/User files/%s/Web docs", &user, "/User files/jdoe/Web docs", OK},

    /* UTF-8. */
    {"ⓟů𝕓ḹḭⓒ﹏𝒽𝚝ṃḹ", &user, "/home/jdoe/ⓟů𝕓ḹḭⓒ﹏𝒽𝚝ṃḹ", OK},
    {"/𝘴ȑṽ/𝙬𝙬𝙬", &user, "/𝘴ȑṽ/𝙬𝙬𝙬/jdoe", OK},
    {"/𝘴ȑṽ/𝙬𝙬𝙬/%s/𝒽𝚝ṃḹ", &user, "/𝘴ȑṽ/𝙬𝙬𝙬/jdoe/𝒽𝚝ṃḹ", OK},

    /* Long filenames. */
    {long_rel_fname, &user, NULL, ERR_LEN},
    {long_abs_fname, &user, NULL, ERR_LEN},
    {long_pattern, &user, NULL, ERR_LEN},
};


/*
 * Main
 */

int
main(void)
{
    FILL_STR('x', long_rel_fname);

    FILL_STR('x', long_abs_fname);
    long_abs_fname[0] = '/';

    FILL_STR('x', long_pattern);
    (void) str_cp(4, "/%s", &long_pattern[sizeof(long_pattern) - 4]);
    long_pattern[0] = '/';

    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];
        char dir[MAX_FNAME_LEN];    /* RATS: ignore */
        Error ret;

        (void) memset(dir, '\0', MAX_FNAME_LEN);

        warnx("checking (%s, %s, -> %s) -> %u ...",
              args.s, args.user->pw_name, args.dir, args.ret);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
        ret = userdir_resolve(args.s, args.user, dir);
#pragma GCC diagnostic pop

        if (ret != args.ret) {
            errx(TEST_FAILED, "returned code %u", ret);
        }
        if (!(args.dir == NULL || strcmp(args.dir, dir) == 0)) {
            errx(TEST_FAILED, "returned directory %s", dir);
        }
    }

    warnx("all tests passed");
    return EXIT_SUCCESS;
}
