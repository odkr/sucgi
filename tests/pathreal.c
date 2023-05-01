/*
 * Test pathreal.
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

#include <sys/stat.h>
#include <assert.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <libgen.h>
#include <limits.h>
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../macros.h"
#include "../params.h"
#include "../path.h"
#include "../str.h"
#include "../types.h"
#include "util/check.h"
#include "util/ftree.h"
#include "util/errlst.h"
#include "util/longp.h"
#include "util/str.h"
#include "util/tmp.h"
#include "util/trap.h"


/*
 * Data types
 */

/* Mapping of filenames to return values. */
typedef struct {
    const bool create;
    const int type;
    const char *const fname;
    const char *const real;
    const ErrList retvals;
    int signo;
} Args;

/* Handle for parsing paths. */
typedef struct {
    const char *fname;
    const char *pos;
} PathHandle;


/*
 * Module variables
 */

/* A filename that that exceeds MAX_FNAME_LEN. */
static char hugefname[MAX_FNAME_LEN + 1U] = {0};

/* A filename just within MAX_FNAME_LEN */
static char longfname[MAX_FNAME_LEN] = {0};

/* Test cases. */
static const Args cases[] = {
#if !defined(NDEBUG)
    /* Illegal arguments. */
    {false, FTW_F, "", "<n/a>", {1, (const Error []) {OK}}, SIGABRT},
#endif

    /* Filename is too long before resolution. */
    {false, FTW_F, hugefname, "<n/a>", {1, (const Error []) {ERR_LEN}}, 0},
    {true, FTW_F, hugefname, "<n/a>", {1, (const Error []) {ERR_LEN}}, 0},

    /* Long filename. */
    {false, FTW_F, longfname, "<n/a>", {1, (const Error []) {ERR_SYS}}, 0},
    {true, FTW_F, longfname, longfname, {1, (const Error []) {OK}}, 0},

    /* Simple tests. */
    {
        true, FTW_F, "file", "file",
        {1, (const Error []) {OK}}, 0
    },
    {
        false, FTW_F, "<nosuchfile>", "<n/a>",
        {1, (const Error []) {ERR_SYS}}, 0
    },
    {
        true, FTW_D, "dir", "dir",
        {1, (const Error []) {OK}}, 0
    },
    {
        false, FTW_D, "<nosuchdir>", "<n/a>",
        {1, (const Error []) {ERR_SYS}}, 0
    },
    {
        true, FTW_SL, "symlink", "linked",
        {1, (const Error []) {OK}}, 0
    },
    {
        false, FTW_SL, "<nosuchlink>", "<n/a>",
        {1, (const Error []) {ERR_SYS}}, 0
    },
    {
        true, FTW_SLN, "linktonowhere", "<nosuchfile>",
        {1, (const Error []) {ERR_SYS}}, 0
    },
    {
        false, FTW_SLN, "<nosuchlink>", "<nosuchfile>",
        {1, (const Error []) {ERR_SYS}}, 0
    },
    {
        true, FTW_F, "dir/file", "dir/file",
        {1, (const Error []) {OK}}, 0
    },
    {
        false, FTW_F, "dir/<nosuchfile>", "<n/a>",
        {1, (const Error []) {ERR_SYS}}, 0
    },
    {
        true, FTW_D, "dir/subdir", "dir/subdir",
        {1, (const Error []) {OK}}, 0
    },
    {
        false, FTW_D, "dir/<nosuchdir>", "<n/a>",
        {1, (const Error []) {ERR_SYS}}, 0
    },
    {
        true, FTW_SL, "dir/symlink", "dir/linked",
        {1, (const Error []) {OK}}, 0
    },
    {
        false, FTW_SL, "dir/<nosuchlink>", "<n/a>",
        {1, (const Error []) {ERR_SYS}}, 0
    },
    {
        true, FTW_SLN, "linktonowhere", "<nosuchfile>",
        {1, (const Error []) {ERR_SYS}}, 0
    },
    {
        false, FTW_SLN, "<nosuchlink>", "<nosuchfile>",
        {1, (const Error []) {ERR_SYS}}, 0
    },

    /* Links to filenames that are too long. */
    {   false, FTW_SL, "hugelink", hugefname,
        {1, (const Error []) {ERR_SYS}}, 0
    },
    {
        true, FTW_SL, "hugelink", hugefname,
        {2, (const Error []) {ERR_LEN, ERR_SYS}}, 0
    }
};

/* Canonical path of temporary directory. */
char *tmpdir = NULL;


/*
 * Prototypes
 */

/* FIXME */
static void cleanup(void);

/* FIXME */
__attribute__((nonnull(2)))
static void genpath(size_t len, char *path);

/* FIXME */
__attribute__((nonnull(1, 3), warn_unused_result))
static int topseg(const char *fname, size_t size, char *dir);

/* FIXME */
__attribute__((nonnull(1), warn_unused_result))
static int mkdir_lp(const char *fname, mode_t mode);

/* FIXME */
__attribute__((nonnull(1), warn_unused_result))
static int touch_lp(const char *const fname, mode_t mode);

/* FIXME */
__attribute__((nonnull(1, 2), warn_unused_result))
static int symlink_lp(const char *real, const char *link);

/* FIXME */
__attribute__((nonnull(2), warn_unused_result))
static int mkdir_w(LongPRole, const char *fname, va_list argp);

/* FIXME */
__attribute__((nonnull(2), warn_unused_result))
static int symlink_w(LongPRole role, const char *fname, va_list argp);

/* FIXME */
__attribute__((nonnull(3), warn_unused_result))
static int touch_w(LongPRole role, const char *fname, va_list argp);


/*
 * Functions
 */

static void
cleanup(void)
{
    errno = 0;
    if (chdir("/") != 0) {
        warn("chdir /");
    }

    errno = 0;
    if (tmpdirremove() != 0) {
        warn("remove %s", tmpdir);
    }
}

static void
genpath(const size_t len, char *const path) {
    assert(path != NULL);

    (void) memset(path, 'x', len);

    for (size_t i = 13U; i < len - 1U; i += 14U) {
        path[i] = '/';
    }

    path[len] = '\0';
}

static int
topseg(const char *const fname, const size_t size, char *const dir)
{
    size_t pos;

    assert(fname != NULL);
    assert(*fname != '\0');
    assert(dir != NULL);

    pos = strcspn(fname, "/");
    if (pos >= size) {
        errno = ENAMETOOLONG;
        return -1;
    }

    (void) memset(dir, '\0', size);
    (void) strncpy(dir, fname, pos);

    return 0;
}

static int
mkdir_w(LongPRole role __attribute__((unused)),
        const char *const fname, va_list argp)
{
    int retval;

    assert(fname != NULL);
    assert(*fname != '\0');

    retval = mkdir(fname, (mode_t) va_arg(argp, int));
    if (retval != 0 && errno != EEXIST) {
        return retval;
    }

    return 0;
}

static int
symlink_w(const LongPRole role, const char *const fname, va_list argp)
{
    assert(fname != NULL);
    assert(*fname != '\0');

    if (role == LONGP_FILE) {
        return symlink(fname, va_arg(argp, const char *));
    }

    return 0;
}

static int
touch_w(const LongPRole role, const char *const fname, va_list argp)
{
    assert(fname != NULL);
    assert(*fname != '\0');

    if (role == LONGP_FILE) {
        int fd, retval;

        do {
            fd = open(fname, O_RDONLY | O_CREAT | O_EXCL | O_CLOEXEC);
        } while (fd < 0 && errno == EINTR);

        if (fd < 0) {
            return fd;
        }

        do {
            retval = close(fd);
        } while (retval != 0 && errno == EINTR);
    } else if (role == LONGP_DIR) {
        int retval;

        retval = mkdir(fname, (mode_t) va_arg(argp, int));
        if (retval != 0 && errno != EEXIST) {
            return retval;
        }
    }

    return 0;
}

static int
mkdir_lp(const char *const fname, const mode_t mode) {
    assert(fname != NULL);
    assert(*fname != '\0');

    return longpdo(mkdir_w, fname, mode);
}

static int
symlink_lp(const char *const real, const char *const link) {
    assert(real != NULL);
    assert(*real != '\0');
    assert(link != NULL);
    assert(*link != '\0');

    return longpdo(symlink_w, real, link);
}

static int
touch_lp(const char *const fname, const mode_t mode) {
    assert(fname != NULL);
    assert(*fname != '\0');

    return longpdo(touch_w, fname, mode);
}


/*
 * Main
 */

int
main (void)
{
    const int signals[] = {SIGHUP, SIGINT, SIGTERM};
    volatile int result = TEST_PASSED;
    size_t tmpdirlen;

    (void) umask(022);

    errno = 0;
    if (atexit(cleanup) != 0) {
        err(TEST_ERROR, "atexit");
    }

    if (trapsigs(NELEMS(signals), signals) != 0) {
        err(TEST_ERROR, "sigaction");
    }

    tmpdir = tmpdirmake();
    if (tmpdir == NULL) {
        err(TEST_ERROR, "tmpdirmake");
    }

    if (strnlen(tmpdir, MAX_FNAME_LEN) >= MAX_FNAME_LEN) {
        errx(TEST_ERROR, "temporary directory filename too long");
    }

    if (chdir(tmpdir) != 0) {
        err(TEST_ERROR, "chdir %s", tmpdir);
    }

    if (checkinit() != 0) {
        err(TEST_ERROR, "sigaction");
    }

    tmpdirlen = strnlen(tmpdir, MAX_FNAME_LEN);

#if !defined(NDEBUG)
    genpath(sizeof(hugefname) - 1U, hugefname);
#endif
    /* 2U because of the '/' realpath will insert and the terminating '\0'. */
    genpath(sizeof(longfname) - tmpdirlen - 2U, longfname);

    for (size_t i = 0; i < NELEMS(cases) && trapped == 0; ++i) {
        const Args args = cases[i];
        char absreal[MAX_STR_LEN];
        char absfname[MAX_STR_LEN];
        char *real;
        int jumpval, nchars;
        volatile Error retval;

        /* FIXME; */
        errno = 0;
        nchars = snprintf(absreal, MAX_STR_LEN, "%s/%s", tmpdir, args.real);

        if (nchars < 0) {
            err(TEST_ERROR, "snprintf");
        }

        if ((size_t) nchars >= MAX_STR_LEN) {
            errx(TEST_ERROR, "real path of %s is too long", args.real);
        }

        errno = 0;
        nchars = snprintf(absfname, MAX_STR_LEN, "%s/%s", tmpdir, args.fname);

        if (nchars < 0) {
            err(TEST_ERROR, "snprintf");
        }

        if ((size_t) nchars >= MAX_STR_LEN) {
            errx(TEST_ERROR, "real path of %s is too long", args.fname);
        }

        if (args.create) {
            switch(args.type) {
            case FTW_F:
                errno = 0;
                if (touch_lp(args.fname, S_IRWXU) != 0) {
                    err(TEST_ERROR, "touch_lp %s", args.fname);
                }
                break;
            case FTW_D:
                if (mkdir_lp(args.fname, S_IRWXU) != 0) {
                    err(TEST_ERROR, "mkdir_lp %s", args.fname);
                }
                break;
            case FTW_SL:
                errno = 0;
                if (touch_lp(args.real, S_IRWXU) != 0) {
                    err(TEST_ERROR, "open %s", args.real);
                }
                /* Falls through. */
            case FTW_SLN:
                errno = 0;
                if (symlink_lp(absreal, absfname) != 0) {
                    err(TEST_ERROR, "symlink_lp %s %s", absreal, absfname);
                }
                break;
            default:
                errx(TEST_ERROR, "%s:%d: filetype %d: unimplemented",
                     __FILE__, __LINE__, FTW_F);
            }
        }

        jumpval = sigsetjmp(checkenv, true);

        if (jumpval == 0) {
            checking = 1;
            retval = pathreal(args.fname, &real);
            checking = 0;

            if (!errlstcontains(&args.retvals, retval)) {
                result = TEST_FAILED;
                if (retval == ERR_SYS) {
                    warn("(%s, → %s) → %u [!]", args.fname, real, retval);
                } else {
                    warnx("(%s, → %s) → %u [!]", args.fname, real, retval);
                }
            }

            if (retval == OK && strncmp(absreal, real, MAX_SUFFIX_LEN) != 0) {
                result = TEST_FAILED;
                warnx("(%s, → %s [!]) → %u", args.fname, real, retval);
            }
        }

        if (jumpval != args.signo) {
            result = TEST_FAILED;
            warnx("(%s, → %s) ↑ %s [!]",
                  args.fname, real, strsignal(jumpval));
        }

        if (jumpval == 0 && retval != ERR_SYS && real != NULL) {
            free(real);
        }

        if (args.create) {
            char seg[MAX_FNAME_LEN];

            errno = 0;
            if (topseg(args.fname, sizeof(seg), seg) != 0) {
                err(TEST_ERROR, "topseg %s", args.fname);
            }

            errno = 0;
            if (ftreerm(seg) != 0) {
                err(TEST_ERROR, "ftreerm %s", seg);
            }

            if (args.type == FTW_SL) {
                char linkseg[MAX_FNAME_LEN];

                errno = 0;
                if (topseg(args.real, sizeof(linkseg), linkseg) != 0) {
                    err(TEST_ERROR, "topseg %s", args.real);
                }

                if (strncmp(seg, linkseg, sizeof(seg)) != 0) {
                    errno = 0;
                    if (ftreerm(linkseg) != 0) {
                        err(TEST_ERROR, "ftreerm %s", linkseg);
                    }
                }
            }
        }
    }

    traphandle(cleanup);
    return result;
}
