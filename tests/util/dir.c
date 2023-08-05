/*
 * Handle directory trees with long names.
 *
 * Copyright 2023 Odin Kroeger.
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

/* macOS doesn't export O_NOFOLLOW with _XOPEN_SOURCE defined as 700. */
#define _BSD_SOURCE
#define _DARWIN_C_SOURCE
#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <sys/stat.h>
#include <dirent.h>
#include <err.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../attr.h"
#include "dir.h"
#include "sigs.h"
#include "types.h"


/*
 * Macros
 */

/* Open flag for entering directories. */
#if defined(O_SEARCH)
#define OFLAG_SEARCH O_SEARCH
#elif defined(O_EXEC)
#define OFLAG_SEARCH O_EXEC
#elif defined(O_PATH)
#define OFLAG_SEARCH O_PATH
#else
#define OFLAG_SEARCH O_RDONLY
#endif


/*
 * Data types
 */

/* Information about a file. */
typedef struct {
    struct stat *fstatus;
    int fildes;
} FileInfo;


/*
 * Prototypes
 */

/*
 * Walk the given directory tree in the given order and call the given
 * function for each file except for the root of the directory tree itself.
 * The function is called for symlinks, but symlinks are not followed.
 * Filesystems boundaries are not crossed.
 *
 * walk_dir_contents requires information about the parent directory; if the
 * given directory is the root of the tree, "parent" must be set to NULL.
 *
 * If the original working directory cannot be restored, the calling
 * process is terminated immediately by calling _exit.
 *
 * Return value:
 *     Zero      Success.
 *     Non-zero  Something went wrong; errno should be set.
 */
_read_only(1) _read_only(4) _nonnull(1, 2) _nodiscard
static int walk_dir_contents(const char *dirname, FileFn func,
                             Order order, const FileInfo *parent);

/*
 * The same as walk_dir_contents, but also applies the
 * given function to the directory tree itself.
 */
_read_only(1) _read_only(4) _nonnull(1, 2) _nodiscard
static int walk_dir_entry(const char *fname, FileFn func,
                          Order order, const FileInfo *parent);


/*
 * Functions
 */

/*
 * NOLINTBEGIN(misc-no-recursion); recursion is intended and needed.
 */

static int
walk_dir_contents(const char *const dirname, const FileFn func,
                  const Order order, const FileInfo *const parent)
{
    DIR *dirp;
    struct stat fstatus;
    struct dirent *dirent;
    int fildes = -1;
    int oldwd = -1;
    int retval = 0;
    int fatalerr = 0;

    assert(dirname != NULL);
    assert(*dirname != '\0');
    assert(func != NULL);
    assert(order == ORDER_PRE || ORDER_POST);

    errno = 0;
    /* RATS: ignore; verifying dirname is the application's job. */
    fildes = open(dirname, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    if (fildes < 0) {
        return fildes;
    }

    errno = 0;
    dirp = fdopendir(fildes);
    if (dirp == NULL) {
        fatalerr = errno;
        retval = -1;
        /* cppcheck-suppress misra-c2012-15.2; false positive. */
        goto cleanup;
    }

    errno = 0;
    retval = fstat(fildes, &fstatus);
    if (retval != 0) {
        fatalerr = errno;
        /* cppcheck-suppress misra-c2012-15.2; false positive. */
        goto cleanup;
    }

    const FileInfo curdir = {
        .fstatus = &fstatus,
        .fildes = fildes,
    };

    if (parent == NULL) {

        /*
         * NOLINTBEGIN(misc-redundant-expression);
         * "O_SEARCH | O_DIRECTORY" is *not* redundant, POSIX.1-2008 leaves
         * the result of `open(<non-directory>, O_SEARCH)` unspecified.
         */

        errno = 0;
        /* RATS: ignore; filename cannot be wrong. */
        oldwd = open(".", OFLAG_SEARCH | O_DIRECTORY | O_CLOEXEC);

        /* NOLINTEND(misc-redundant-expression) */

        if (oldwd < 0) {
            fatalerr = errno;
            retval = oldwd;
            goto cleanup;
        }
    } else {
        oldwd = parent->fildes;

        if (fstatus.st_dev != parent->fstatus->st_dev) {
            fatalerr = EXDEV;
            retval = -1;
            goto cleanup;
        }
    }

    errno = 0;
    retval = fchdir(fildes);
    if (retval != 0) {
        fatalerr = errno;
        goto cleanup;
    }

    /* cppcheck-suppress readdirCalled; function need not be async-safe. */
    while ((errno = 0, dirent = readdir(dirp)) != NULL) {
        if (*dirent->d_name == '\0') {
            /* NOTREACHED */
            continue;
        }

        if (strncmp(dirent->d_name, ".", sizeof(".")) == 0) {
            continue;
        }

        if (strncmp(dirent->d_name, "..", sizeof("..")) == 0) {
            continue;
        }

        errno = 0;
        retval = walk_dir_entry(dirent->d_name, func, order, &curdir);
        if (retval != 0) {
            fatalerr = errno;
            break;
        }
    }

    /* cppcheck-suppress misra-c2012-22.10; readdir sets errno. */
    if (dirent == NULL && errno != 0) {
        fatalerr = errno;
        retval = -1;
    }

    if (sigs_retry_int(fchdir, oldwd) != 0) {
        /*
         * Being in another directory than you think is BAD. However,
         * this point is only reached if (a) the permissions of oldwd
         * were changed after walk_dir_contents left it or (b) I/O
         * errors prevented reading oldwd.
         */
        warn("%s:%d: chdir %d", __FILE__, __LINE__, oldwd);
        _exit(EXIT_FAILURE);
    }

    cleanup:
        if (parent == NULL && oldwd > -1) {
            errno = 0;
            if (sigs_retry_int(close, oldwd) != 0) {
                /* NOTREACHED */
                if (retval == 0) {
                    fatalerr = errno;
                    retval = -1;
                }
            }
        }

        if (dirp == NULL) {
            errno = 0;
            if (sigs_retry_int(close, fildes) != 0) {
                /* NOTREACHED */
                if (retval == 0) {
                    fatalerr = errno;
                    retval = -1;
                }
            }
        } else {
            errno = 0;
            if (sigs_retry_ptr((int (*)(void *)) closedir, dirp) != 0) {
                /* NOTREACHED */
                if (retval == 0) {
                    fatalerr = errno;
                    retval = -1;
                }
            }
        }

    errno = fatalerr;
    return retval;
}

static int
walk_dir_entry(const char *const fname, int (*const func)(const char *),
               const Order order, const FileInfo *const parent)
{
    int retval = 0;

    assert(fname != NULL);
    assert(*fname != '\0');
    assert(func != NULL);
    assert(order == ORDER_PRE || order == ORDER_POST);

    if (order == ORDER_PRE) {
        errno = 0;
        retval = func(fname);
        if (retval != 0) {
            return retval;
        }
    }

    errno = 0;
    retval = walk_dir_contents(fname, func, order, parent);
    /* cppcheck-suppress misra-c2012-22.10; walk_dir_contents sets errno. */
    if (retval != 0 && errno != ENOTDIR && errno != ELOOP) {
        return retval;
    }

    if (order == ORDER_POST) {
        errno = 0;
        retval = func(fname);
        if (retval != 0) {
            return retval;
        }
    }

    return retval;
}

/*
 * NOLINTEND(misc-no-recursion)
 */


int
dir_walk(const char *const fname, const FileFn func,
         const Order order, const ErrorFn errh)
{
    int retval = 0;

    assert(fname != NULL);
    assert(*fname != '\0');
    assert(func != NULL);
    assert(order == ORDER_PRE || order == ORDER_POST);

    errno = 0;
    retval = walk_dir_entry(fname, func, order, NULL);
    if (retval != 0) {
        if (errh != NULL) {
            errh(EXIT_FAILURE, "dir_walk %s", fname);
        }
    }

    return retval;
}

int
dir_tree_rm(const char *const fname, const ErrorFn errh)
{
    int retval = 0;

    assert(fname != NULL);
    assert(*fname != '\0');

    errno = 0;
    retval = dir_walk(fname, remove, ORDER_POST, NULL);
    if (retval != 0) {
        if (errh != NULL) {
            errh(EXIT_FAILURE, "dir_tree_rm %s", fname);
        }
    }

    return retval;
}
