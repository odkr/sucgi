/*
 * Test path_real.
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
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../attr.h"
#include "../macros.h"
#include "../params.h"
#include "../path.h"
#include "../types.h"
#include "util/abort.h"
#include "util/array.h"
#include "util/dir.h"
#include "util/path.h"
#include "util/sigs.h"
#include "util/tmpdir.h"
#include "util/types.h"


/*
 * Macros
 */

/* Maximum number of return values to check for. */
#define MAX_NRETVALS 4


/*
 * Data types
 */

/* Types of files to test path_real with. */
typedef enum {
    FT_FILE,                            /* Existing file. */
    FT_NOFILE,                          /* Non-existing file. */
    FT_DIR,                             /* Directory. */
    FT_NODIR,                           /* Non-existing directory. */
    FT_LINK_FILE,                       /* Link to file. */
    FT_LINK_NOFILE,                     /* Link to non-existing file. */
    FT_NOLINK                           /* Non-existing link. */
} FileType;

/* NOLINTBEGIN(clang-analyzer-optin.performance.Padding) */

/* Mapping of filenames to return values. */
typedef struct {
    const FileType type;                /* Filetype. */
    const char *const fname;            /* Filename */
    const char *const real;             /* Real name of that file. */
    const size_t nretvals;              /* Number of legal return values. */
    const Error retvals[MAX_NRETVALS];  /* Legal return values. */
    /* cppcheck-suppress cert-API01-C */
    int signal;                         /* Expected signal. */
} PathRealArgs;

/* NOLINTEND(clang-analyzer-optin.performance.Padding) */


/*
 * Module variables
 */

/* Path of temporary directory. */
static char *tmpdir = NULL;

/* Last signal caught by catch_sig. */
static volatile sig_atomic_t caught = 0;


/*
 * Prototypes
 */

/*
 * Create the given file and every directory along the way.
 * "errh" is an error handler to pass to path_walk.
 *
 * Return value:
 *     Zero      Success.
 *     Non-zero  Something went wrong; errno should be set.
 */
_read_only(1) _nonnull(1)
static int make_file(const char *fname, ErrorFn errh);

/*
 * Create the given directory and every directory along the way.
 * "errh" is an error handler to pass to path_walk.
 *
 * Return value:
 *     Zero      Success.
 *     Non-zero  Something went wrong; errno should be set.
 */
_read_only(1) _nonnull(1)
static int make_path(const char *fname, ErrorFn errh);

/*
 * Create a link to the given file and every directory along the way.
 * "errh" is an error handler to pass to path_walk.
 *
 * Return value:
 *     Zero      Success.
 *     Non-zero  Something went wrong; errno should be set.
 */
_read_only(1) _nonnull(1)
static int make_link(const char *fname, const char *link, ErrorFn errh);

/*
 * Wrapper around open/close to pass to path_walk.
 */
_read_only(1) _nonnull(1) _nodiscard
static int make_file_w(const char *fname, size_t nargs, va_list argp);

/*
 * Wrapper around mkdir to pass to path_walk.
 */
_read_only(1) _nonnull(1) _nodiscard
static int make_path_w(const char *fname, size_t nargs, va_list argp);

/*
 * Wrapper around symlink to pass to path_walk.
 */
_read_only(1) _nonnull(1) _nodiscard
static int make_link_w(const char *fname, size_t nargs, va_list argp);

/*
 * Remove the given file or directory, recursively if necessary.
 * Does not trigger an error if the file does not exist.
 * Otherwise the same as dir_tree_rm.
 *
 * Return value:
 *     See dir_tree_rm.
 */
_read_only(1) _nonnull(1)
static int rm_dir_tree_silently(const char *fname, ErrorFn errh);

/*
 * Check whether two errors are equal.
 *
 * Return value:
 *     Zero      Errors are equal.
 *     Non-zero  Otherwise.
 */
_read_only(1) _read_only(2) _nonnull(1, 2) _nodiscard
static int cmp_errs(const void *ptr1, const void *ptr2);

/*
 * Store the given signal in the global "caught".
 */
static void catch_sig(int signal);

/*
 * Remove the temporary directory unless the global "tmpdir" is NULL.
 */
static void cleanup(void);


/*
 * Main
 */

int
main(void)
{
    /* RATS: ignore; used safely. */
    char hugefname[MAX_FNAME_LEN + 1] = {0};

    /* RATS: ignore; used safely. */
    char longfname[MAX_FNAME_LEN] = {0};

    /* cppcheck-suppress [misra-c2012-9.3, misra-c2012-9.4] */
    const PathRealArgs cases[] = {
#if !defined(NDEBUG)
        /* Illegal arguments. */
        {FT_NOFILE, "", "<n/a>", 1, {OK}, SIGABRT},
#endif

        /* Filename is too long before resolution. */
        {FT_NOFILE, hugefname, "<n/a>", 1, {ERR_LEN}, 0},
        {FT_FILE, hugefname, "<n/a>", 1, {ERR_LEN}, 0},

        /* Long filename. */
        {FT_NOFILE, longfname, "<n/a>", 1, {ERR_SYS}, 0},
        {FT_FILE, longfname, longfname, 1, {OK}, 0},

        /* Simple tests. */
        {FT_FILE, "file", "file", 1, {OK}, 0},
        {FT_NOFILE, "<nosuchfile>", NULL, 1, {ERR_SYS}, 0},
        {FT_DIR, "dir", "dir", 1, {OK}, 0},
        {FT_NODIR, "<nosuchdir>", NULL, 1, {ERR_SYS}, 0},
        {FT_LINK_FILE, "symlink", "linked", 1,{OK}, 0},
        {FT_LINK_NOFILE, "<nosuchlink>", "<n/a>", 1, {ERR_SYS}, 0},
        {FT_LINK_NOFILE, "linktonowhere", "<nosuchfile>", 1, {ERR_SYS}, 0},
        {FT_NOLINK, "<nosuchlink>", "<nosuchfile>", 1, {ERR_SYS}, 0},
        {FT_FILE, "dir/file", "dir/file", 1, {OK}, 0},
        {FT_NOFILE, "dir/<nosuchfile>", "<n/a>", 1, {ERR_SYS}, 0},
        {FT_DIR, "dir/subdir", "dir/subdir", 1, {OK}, 0},
        {FT_NODIR, "dir/<nosuchdir>", "<n/a>", 1, {ERR_SYS}, 0},
        {FT_LINK_FILE, "dir/symlink", "dir/linked", 1, {OK}, 0},
        {FT_NOLINK, "dir/<nosuchlink>", "<n/a>", 1, {ERR_SYS}, 0},
        {FT_LINK_NOFILE, "linktonowhere", "<nosuchfile>", 1, {ERR_SYS}, 0},
        {FT_NOLINK, "<nosuchlink>", "<nosuchfile>", 1, {ERR_SYS}, 0},

        /* Links to filenames that are too long. */
        {FT_LINK_NOFILE, "hugelink", hugefname, 1, {ERR_SYS}, 0},
        {FT_LINK_FILE, "hugelink", hugefname, 2, {ERR_LEN, ERR_SYS}, 0}
    };

    volatile int result = PASS;
    char *realtmpdir;
    size_t tmpdirlen;

    (void) sigs_handle(catch_sig, SIGS_NTERM, sigs_term, NULL, err);

    /* RATS: ignore; umask is restrictive. */
    (void) umask(S_ISUID | S_ISGID | S_ISVTX | S_IRWXG | S_IRWXO);

    errno = 0;
    if (atexit(cleanup) != 0) {
        err(ERROR, "atexit");
    }

    (void) tmpdir_make("tmp-XXXXXX", &tmpdir, err);
    assert(tmpdir != NULL);

    if (strnlen(tmpdir, MAX_FNAME_LEN) >= (size_t) MAX_FNAME_LEN) {
        errx(ERROR, "%s: filename too long", tmpdir);
    }

    errno = 0;
    /* RATS: ignore; used safely. */
    realtmpdir = realpath(tmpdir, NULL);
    if (realtmpdir == NULL) {
        err(ERROR, "realpath %s", tmpdir);
    }

    free(tmpdir);
    tmpdir = realtmpdir;

    if (chdir(tmpdir) != 0) {
        err(ERROR, "chdir %s", tmpdir);
    }

    tmpdirlen = strnlen(tmpdir, MAX_FNAME_LEN);

    path_gen(sizeof(hugefname) - 1U, hugefname);
    /* "- 2U" because of '/' and the terminating '\0'. */
    path_gen(sizeof(longfname) - tmpdirlen - 2U, longfname);

    for (volatile size_t i = 0; i < NELEMS(cases) && caught == 0; ++i) {
        const PathRealArgs args = cases[i];
        char *topseg;

        abort_signal = 0;
        if (sigsetjmp(abort_env, 1) == 0) {
            char *real = NULL;
            size_t fnamelen;
            size_t reallen;
            const Error *retvalp = NULL;
            Error retval;

            fnamelen = strnlen(args.fname, MAX_STR_LEN);
            assert(fnamelen < MAX_STR_LEN);

            switch(args.type) {
            case FT_LINK_FILE:
                errno = 0;
                (void) make_link(args.real, args.fname, err);
                /* cppcheck-suppress misra-c2012-16.3; falls through */
            case FT_FILE:
                errno = 0;
                (void) make_file(args.real, err);
                break;
            case FT_DIR:
                 errno = 0;
                (void) make_path(args.real, err);
                break;
            default:
                break;
            }

            if (args.signal != 0) {
                warnx("the next test should fail an assertion.");
            }

            (void) abort_catch(err);
            retval = path_real(fnamelen, args.fname, &reallen, &real);
            (void) abort_reset(err);

            /* cppcheck-suppress misra-c2012-11.5; this conversion is fine. */
            retvalp = array_find(&retval, args.retvals, args.nretvals,
                                sizeof(*args.retvals), (CompFn) cmp_errs);

            if (retvalp == NULL) {
                result = FAIL;
                warnx("%zu: (<fnamelen>, %s, → %zu, → %s) → %u [!]",
                      i, args.fname, reallen, real, retval);
            }

            if (retval == OK) {
                if (strnlen(real, MAX_FNAME_LEN) != reallen ||
                    reallen >= (size_t) MAX_FNAME_LEN)
                {
                    result = FAIL;
                    warnx("%zu: (<fnamelen>, %s, → %zu [!], → %s) → %u",
                          i, args.fname, reallen, real, retval);
                }

                if (strcmp(args.real, &real[tmpdirlen + 1U]) != 0) {
                    result = FAIL;
                    warnx("%zu: (<fnamelen>, %s, → %zu, → %s [!]) → %u",
                          i, args.fname, reallen, real, retval);
                }
            }

            if (retval != OK && real != NULL) {
                free(real);
            }
        }

        if (abort_signal != args.signal) {
            result = FAIL;
            warnx("%zu: (<fnamelen>, %s, → <reallen>, → <real>) ↑ %s [!]",
                  i, args.fname, strsignal(abort_signal));
        }

        switch(args.type) {
        case FT_LINK_FILE:
            topseg = path_split(args.real, NULL, err);
            (void) rm_dir_tree_silently(topseg, err);
            free(topseg);
            /* cppcheck-suppress misra-c2012-16.3; falls through */
        case FT_FILE:
            /* Falls through */
        case FT_DIR:
            topseg = path_split(args.fname, NULL, err);
            (void) rm_dir_tree_silently(topseg, err);
            free(topseg);
            break;
        default:
            break;
        }
    }

    if (caught != 0) {
        warnx("%s", strsignal(caught));
        cleanup();
        (void) sigs_raise_default(caught, err);
    }

    return result;
}


/*
 * Functions
 */

static int
make_file(const char *const fname, const ErrorFn errh)
{
    int retval = 0;

    assert(fname != NULL);
    assert(*fname != '\0');

    errno = 0;
    retval = path_walk(fname, make_path_w, make_file_w, NULL, 0);
    if (retval != 0 && errh != NULL) {
        errh(ERROR, "make_file %s", fname);
    }

    return retval;
}

static int
make_path(const char *const fname, const ErrorFn errh)
{
    int retval = 0;

    assert(fname != NULL);
    assert(*fname != '\0');

    errno = 0;
    retval = path_walk(fname, make_path_w, make_path_w, NULL, 0);
    if (retval != 0 && errh != NULL) {
        errh(ERROR, "make_path %s", fname);
    }

    return retval;
}

static int
make_link(const char *const fname, const char *const link, const ErrorFn errh)
{
    const char *abslink;
    /* RATS: ignore; used safely. */
    char buffer[MAX_FNAME_LEN] = {0};
    int retval = 0;

    assert(fname != NULL);
    assert(*fname != '\0');
    assert(link != NULL);
    assert(*link != '\0');

    if (*link == '/') {
        abslink = link;
    } else {
        /* RATS: ignore; used safely. */
        char curwd[MAX_FNAME_LEN] = {0};

        errno = 0;
        if (getcwd(curwd, sizeof(curwd)) == NULL) {
            if (errh != NULL) {
                errh(ERROR, "make_link %s %s", fname, link);
            }
            return -1;
        }

        errno = 0;
        retval = path_join(sizeof(buffer), buffer, curwd, link, errh);
        if (retval < 0) {
            if (errh != NULL) {
                errh(ERROR, "make_link %s %s", fname, link);
            }
            return -1;
        }

        abslink = buffer;
    }

    errno = 0;
    retval = path_walk(fname, make_path_w, make_link_w, NULL, 1, abslink);
    if (retval != 0 && errh != NULL) {
        errh(ERROR, "make_link %s %s", fname, link);
    }

    return retval;
}

static int
make_file_w(const char *const fname,
            const size_t nargs _unused, va_list argp _unused)
{
    int fildes = -1;

    assert(fname != NULL);
    assert(*fname != '\0');

    errno = 0;
    /* RATS: ignore; no race */
    fildes = open(fname, O_RDONLY | O_CREAT | O_EXCL | O_CLOEXEC, S_IRWXU);
    if (fildes < 0) {
        return -1;
    }

    return sigs_retry_int(close, fildes);
}

static int
make_path_w(const char *const fname,
            const size_t nargs _unused, va_list argp _unused)
{
    int retval = 0;

    assert(fname != NULL);
    assert(*fname != '\0');

    errno = 0;
    /* RATS: ignore; no race condition. */
    retval = mkdir(fname, S_IRWXU);
    /* cppcheck-suppress misra-c2012-22.10; mkdir sets errno. */
    if (retval != 0 && errno != EEXIST) {
        return retval;
    }

    return 0;
}

static int
make_link_w(const char *const fname, const size_t nargs, va_list argp)
{
    assert(fname != NULL);
    assert(*fname != '\0');
    assert(nargs == 1U);

    errno = 0;
    return symlink(fname, (const char *) va_arg(argp, const char *));
}

static int
rm_dir_tree_silently(const char *const fname, const ErrorFn errh)
{
    int retval = 0;

    errno = 0;
    retval = dir_tree_rm(fname, NULL);
    /* cppcheck-suppress misra-c2012-22.10; mkdir sets errno. */
    if (retval != 0 && errno != ENOENT) {
        errh(ERROR, "dir_tree_rm %s", fname);
    }

    return retval;
}

static int
cmp_errs(const void *const ptr1, const void *const ptr2)
{
    Error err1;
    Error err2;

    /*
     * Cast to pointer-to-object is safe because cmp_errs is only called by
     * array_is_sub, which only calls cmp_errs for an array of Errors.
     */
    err1 = *((const Error *) ptr1); /* cppcheck-suppress misra-c2012-11.5 */
    err2 = *((const Error *) ptr2); /* cppcheck-suppress misra-c2012-11.5 */

    if (err1 < err2) {
        return -1;
    }

    if (err1 > err2) {
        return 1;
    }

    return 0;
}

static void
catch_sig(const int signal)
{
    caught = signal;
}

static void
cleanup(void)
{
    if (tmpdir != NULL) {
        if (dir_tree_rm(tmpdir, NULL) != 0) {
            warn("dir_tree_rm %s", tmpdir);
        }
    }
}
