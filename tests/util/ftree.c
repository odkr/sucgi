/*
 * File tree handling.
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
#include <dirent.h>
#include <err.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../../attr.h"
#include "../../params.h"
#include "ftree.h"


/*
 * Macros
 */

/* Maximum recursion depth. */
#define MAX_DEPTH 512


/*
 * Prototypes
 */

/*
 * Remove PATH. Wrapper around remove(3) for use with ftreewalk(3).
 */
__attribute__((nonnull(1), warn_unused_result))
static int remove_w(const char *fname, const int);

/*
 * Apply FUNC to FNAME in the given ORDER and call wrecuse.
 *
 * Return value:
 *     Zero      Success.
 *     Non-zero  Something went wrong; errno should be set.
 */
__attribute__((nonnull(1, 2), warn_unused_result))
static int wapply(const char *fname, FTreeFunc func, const FTreeOrder order,
                  DIR *prevp, dev_t dev, int depth);

/*
 * Change to the directory DIRP, call wapply for each file in that
 * directory, and then change back to the directory PREVP.
 *
 * Return value:
 *     Zero      Success.
 *     Non-zero  Something went wrong; errno should be set.
 */
__attribute__((nonnull(1, 2), warn_unused_result))
static int wloop(DIR *dirp, FTreeFunc func, const FTreeOrder order,
                 DIR *prevp, dev_t dev, int depth);

/*
 * Open FNAME as directory, refusing symbolic links, check if that directory
 * resides on DEV, process it with wloop, and then close it again.
 *
 * Return value:
 *     Zero      Success.
 *     Non-zero  Something went wrong; errno should be set.
 */
__attribute__((nonnull(1, 2), warn_unused_result))
static int wrecurse(const char *fname, FTreeFunc func, const FTreeOrder order,
                    DIR *prevp, dev_t dev, int depth);


/*
 * Functions
 */

static int
remove_w(const char *const fname,
         const int depth __attribute__((unused)))
{
    assert(fname != NULL);
    assert(*fname != '\0');

    errno = 0;
    return remove(fname);
}

static int
wapply(const char *fname, const FTreeFunc func, const FTreeOrder order,
       DIR *const prevp, const dev_t dev, const int depth)
{
    int retval;

    assert(fname != NULL);
    assert(*fname != '\0');
    assert(func != NULL);
    assert(prevp != NULL);

    retval = 0;

    if (order == FTREE_PRE) {
        retval = func(fname, depth);
        if (retval != 0) {
            return retval;
        }
    }

    errno = 0;
    retval = wrecurse(fname, func, order, prevp, dev, depth);

    if (retval != 0 && errno != ENOTDIR && errno != ELOOP) {
        return retval;
    }

    if (order == FTREE_POST) {
        retval = func(fname, depth);

        if (retval != 0) {
            return retval;
        }
    }

    return retval;
}

static int
wloop(DIR *const dirp, const FTreeFunc func, const FTreeOrder order,
      DIR *const prevp, const dev_t dev, const int depth)
{
    struct dirent *dirent;
    int retval, olderr;

    assert(dirp != NULL);
    assert(func != NULL);
    assert(prevp != NULL);

    retval = 0;

    if (depth >= MAX_DEPTH) {
        errno = ENAMETOOLONG;
        return -1;
    }

    do {
        retval = fchdir(dirfd(dirp));
    } while (retval != 0 && errno == EINTR);

    if (retval != 0 && errno != ENOTDIR) {
        return retval;
    }

    while ((dirent = readdir(dirp)) != NULL) {
        if (strncmp(dirent->d_name, ".", sizeof(".")) == 0) {
            continue;
        }

        if (strncmp(dirent->d_name, "..", sizeof("..")) == 0) {
            continue;
        }

        errno = 0;
        retval = wapply(dirent->d_name, func, order, dirp, dev, depth + 1);
        if (retval != 0) {
            break;
        }
    }

    if (errno != 0) {
        retval = -1;
    }

    olderr = errno;
    do {
        if (fchdir(dirfd(prevp)) != 0) {
            if (errno == EINTR) {
                continue;
            } else if (retval == 0) {
                retval = -1;
            } else {
                errno = olderr;
            }
        }
        break;
    } while (true);

    return retval;
}

static int
wrecurse(const char *fname, const FTreeFunc func, const FTreeOrder order,
         DIR *const prevp, const dev_t dev, const int depth)
{
    DIR *dirp;
    struct stat dstat;
    int fd, retval, olderr;

    assert(fname != NULL);
    assert(*fname != '\0');
    assert(func != NULL);
    assert(prevp != NULL);

    do {
        fd = open(fname, O_DIRECTORY| O_NOFOLLOW | O_CLOEXEC | O_RDONLY);
    } while (fd < 0 && errno == EINTR);

    if (fd < 0) {
        return fd;
    }

    dirp = fdopendir(fd);
    if (dirp == NULL) {
        return -1;
    }

    retval = fstat(fd, &dstat);
    if (retval != 0) {
        return retval;
    }

    if (dstat.st_dev != dev) {
        errno = EXDEV;
        return retval;
    }

    retval = wloop(dirp, func, order, prevp, dev, depth);

    olderr = errno;
    do {
        if (closedir(dirp) != 0) {
            if (errno == EINTR) {
                continue;
            } else if (retval == 0) {
                retval = -1;
            } else {
                errno = olderr;
            }
        }
        break;
    } while (true);

    return retval;
}

int
ftreerm(const char *const fname)
{
    assert(fname != NULL);
    assert(*fname != '\0');

    return ftreewalk(fname, remove_w, FTREE_POST);
}

int
ftreewalk(const char *const fname,
          const FTreeFunc func, const FTreeOrder order)
{
    DIR *oldwdp;
    struct stat dstat;
    int retval;

    assert(fname != NULL);
    assert(*fname != '\0');
    assert(func != NULL);

    retval = 0;

    retval = lstat(fname, &dstat);
    if (retval != 0) {
        return retval;
    }

    oldwdp = opendir(".");
    if (oldwdp == NULL) {
        return -1;
    }

    retval = wapply(fname, func, order, oldwdp, dstat.st_dev, 0);

    return retval;
}
