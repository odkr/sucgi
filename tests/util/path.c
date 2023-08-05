/*
 * Handle long paths.
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

#define _BSD_SOURCE
#define _DARWIN_C_SOURCE
#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <sys/types.h>
#include <err.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../compat.h"
#include "path.h"
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
 * Functions
 */

void
path_gen(const size_t len, char *const path) {
    assert(_POSIX_NAME_MAX > 0);
    assert(path != NULL);

    (void) memset(path, 'x', len);

    for (size_t i = _POSIX_NAME_MAX - 1; i < len - 1U; i += _POSIX_NAME_MAX) {
        path[i] = '/';
    }

    path[len] = '\0';
}

int
path_join(const size_t size, char *const path,
          const char *const dir, const char *const fname,
          const ErrorFn errh)
{
    int len = 0;

    assert(size == 0U || path != NULL);

    (void) memset(path, '\0', size);

    errno = 0;
    /* RATS: ignore; format is a literal. */
    len = snprintf(path, size, "%s/%s", dir, fname);
    if (len < 0) {
        if (errh != NULL) {
            errh(EXIT_FAILURE, "path_join");
        }
        return len;
    }

    if (size > 0U && (size_t) len >= size) {
        errno = ENAMETOOLONG;
        if (errh != NULL) {
            errh(EXIT_FAILURE, "path_join");
        }
        return -1;
    }

    return len;
}

char *
path_split(const char *const path, const char **const ptr,
           const ErrorFn errh)
{
    char *seg = NULL;
    const char *start = NULL;
    const char *pivot = NULL;
    size_t len = 0U;

    assert(path != NULL);

    pivot = path;
    do {
        const char *end;

        start = pivot;
        len = (*start == '/') ? 1U : strcspn(start, "/");
        end = &start[len];

        pivot = &end[strspn(end, "/")];
    } while (*start == '.' && len == 1U);

    errno = 0;

    if (len > 0U) {
        seg = strndup(start, len);
        if (seg == NULL) {
            if (errh != NULL) {
                errh(EXIT_FAILURE, "strndup");
            }
            return NULL;
        }
    }

    if (ptr != NULL) {
        *ptr = pivot;
    }

    return seg;
}


int
path_walk(const char *const fname,
          const VFileFn dirfn, const VFileFn basefn,
          const ErrorFn errh,
          const size_t nargs, ...)
{
    const char *ptr = NULL;
    char *seg = NULL;
    int retval = 0;
    int fatalerr = 0;
    int oldwd = -1;

    assert(fname != NULL);
    assert(*fname != '\0');
    assert(dirfn != NULL || basefn != NULL);

    /*
     * NOLINTBEGIN(misc-redundant-expression);
     * "O_SEARCH | O_DIRECTORY" is not redundant, POSIX.1-2008 leaves
     * the result of `open(<non-directory>, O_SEARCH)` unspecified.
     */

    errno = 0;
    /* RATS: ignore; filename cannot be wrong. */
    oldwd = open(".", OFLAG_SEARCH | O_DIRECTORY | O_CLOEXEC);
    if (oldwd < 0) {
        return -1;
    }

    /* NOLINTEND(misc-redundant-expression) */

    ptr = fname;
    /* cppcheck-suppress misra-c2012-15.4 */
    while ((errno = 0, seg = path_split(ptr, &ptr, NULL)) != NULL) {
        const bool is_dir = (*ptr != '\0');
        const VFileFn func = is_dir ? dirfn : basefn;

        if (func != NULL) {
            va_list argp;

            va_start(argp, nargs);
            errno = 0;
            retval = func(seg, nargs, argp);
            va_end(argp);

            if (retval != 0) {
                free(seg);
                fatalerr = errno;
                break;
            }
        }

        if (is_dir) {
            errno = 0;
            retval = chdir(seg);
            if (retval != 0) {
                free(seg);
                fatalerr = errno;
                break;
            }
        }

        free(seg);
    }

    /* cppcheck-suppress misra-c2012-22.10; path_split sets errno. */
    if (errno != 0) {
        fatalerr = errno;
        retval = -1;
    }

    if (sigs_retry_int(fchdir, oldwd) != 0) {
        /*
         * Being in another directory than you think is BAD. However,
         * this point is only reached if (a) the permissions of oldwd
         * were changed after path_walk left it or (b) I/O errors
         * prevented reading oldwd.
         */
        warn("%s:%d: chdir %s", __FILE__, __LINE__, fname);
        _exit(EXIT_FAILURE);
    }

    errno = 0;
    if (sigs_retry_int(close, oldwd) != 0) {
        /* NOTREACHED */
        if (retval == 0) {
            fatalerr = errno;
            retval = -1;
        }
    }

    errno = fatalerr;
    if (retval != 0 && errh != NULL) {
        errh(EXIT_FAILURE, "path_walk %s", fname);
    }

    return retval;
}
