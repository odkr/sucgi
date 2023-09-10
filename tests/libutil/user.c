/*
 * Utility functions for searching users.
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

#define _XOPEN_SOURCE 700

#include <sys/types.h>
#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <pwd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include "../../attr.h"
#include "../../macros.h"
#include "user.h"
#include "types.h"


/*
 * Prototypes
 */

/*
 * Convert the given UID into a string and copy it to the memory
 * area pointed to by "str", which must be of the given size, and
 * return the length of the string. The memory area must be large
 * enough to accomodate the resulting string, including the
 * terminating NUL. "str" may be NULL iff the given size is 0.
 *
 * Return value:
 *     Non-negative  Length of the string.
 *     Negative      Something went wrong; errno may be set.
 */
_write_only(3, 2) _nodiscard
static int uid_to_str(id_t id, size_t size, char *str);

/*
 * Functions
 */

static int
uid_to_str(id_t id, const size_t size, char *const str)
{
    if (str != NULL) {
        /* Some versions of snprintf fail to NUL-terminate strings. */
        (void) memset(str, '\0', size);
    }

    errno = 0;
    return ISSIGNED(id_t) ?
        /* RATS: ignore; format is a literal and expansion is bounded. */
        snprintf(str, size, "%lld", (long long) id) :
        /* RATS: ignore */
        snprintf(str, size, "%llu", (unsigned long long) id);
}

int
user_get_gid(const uid_t uid, gid_t *const gid, const ErrorFn errh)
{
    errno = 0;
    long bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufsize < 0) {
        if (errh != NULL) {
            errh(EXIT_FAILURE, "sysconf");
        }

        return -1;
    }

    assert((uintmax_t) bufsize <= (uintmax_t) SIZE_MAX);

    errno = 0;
    /* cppcheck-suppress misra-c2012-11.5; bad advice for malloc. */
    char *buf = malloc((size_t) bufsize);
    if (buf == NULL) {
        if (errh != NULL) {
            errh(EXIT_FAILURE, "malloc");
        }

        return -1;
    }

    struct passwd pwd;
    struct passwd *result = NULL;
    errno = getpwuid_r(uid, &pwd, buf, (size_t) bufsize, &result);
    free(buf);

    if (result == NULL) {
        /* cppcheck-suppress misra-c2012-22.10; errno was just set. */
        if (errno != 0) {
            if (errh != NULL) {
                errh(EXIT_FAILURE, "getpwuid");
            }

            return -1;
        }

        return -1;
    }

    *gid = pwd.pw_gid;

    return 0;
}


int
user_get_regular(struct passwd *const pwd, const ErrorFn errh)
{
    assert(pwd != NULL);

    int fatalerr = 0;
    int retval = 1;

    errno = 0;
    setpwent();
    /* cppcheck-suppress misra-c2012-22.10; setpwent sets errno. */
    if (errno != 0) {
        if (errh != NULL) {
            errh(EXIT_FAILURE, "setpwent");
        }
        return -1;
    }

    struct passwd *ptr;
    /* cppcheck-suppress getpwentCalled; getpwent_r is not portable. */
    while ((errno = 0, ptr = getpwent()) != NULL) {
        if (ptr->pw_uid > 0) {
            /* RATS: ignore; can neither over- nor underflow. */
            (void) memcpy(pwd, ptr, sizeof(*ptr));
            retval = 0;
            break;
        }
    }

    /* cppcheck-suppress misra-c2012-22.10; getpwent sets errno. */
    if (ptr == NULL && errno != 0) {
        fatalerr = errno;
        retval = -1;
    }

    errno = 0;
    endpwent();
    /* cppcheck-suppress misra-c2012-22.10; endpwent sets errno. */
    if (errno != 0) {
        if (retval != -1) {
            if (errh != NULL) {
                errh(EXIT_FAILURE, "endpwent");
            }
            retval = -1;
        } else {
            /* RATS: ignore; format string is short and a literal. */
            syslog(LOG_ERR, "endpwent");
        }
    }

    errno = fatalerr;
    if (retval == -1 && errh != NULL) {
        errh(EXIT_FAILURE, "getpwent");
    }

    return retval;
}

char *
user_id_to_str(const id_t id, const ErrorFn errh)
{
    int olderr = errno;

    int len = uid_to_str(id, 0, NULL);
    if (len < 0) {
        /* NOTREACHED */
        if (errh != NULL) {
            errh(EXIT_FAILURE, "%s:%d: snprintf", __FILE__, __LINE__);
        }
        return NULL;
    }

    size_t size = (size_t) len + 1U;

    errno = 0;
    /* cppcheck-suppress misra-c2012-11.5; bad advice for malloc. */
    char *str = malloc(size);
    if (str == NULL) {
        if (errh != NULL) {
            errh(EXIT_FAILURE, "malloc");
        }
        return NULL;
    }

    len = uid_to_str(id, size, str);
    if (len < 0) {
        /* NOTREACHED */
        free(str);

        if (errh != NULL) {
            errh(EXIT_FAILURE, "%s:%d: snprintf", __FILE__, __LINE__);
        }
        return NULL;
    }

    errno = olderr;
    return str;
}
