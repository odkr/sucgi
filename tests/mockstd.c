/*
 * Mock-ups of user and group ID functions.
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
/* _DARWIN_C_SOURCE must NOT be set, or else getgroups won't be interposed. */
#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <sys/types.h>
#include <assert.h>
#include <err.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../compat.h"
#include "../macros.h"
#include "../params.h"
#include "mockstd.h"


/*
 * Interposition
 */

#if defined(__MACH__) && __MACH__ &&                            \
    defined(__GNUC__) &&                                        \
    (__GNUC__ > 3 || (__GNUC__ >= 3 && __GNUC_MINOR__ >= 1)) && \
    !defined(__ICC)
#include <unistd.h>
#define DYLD_INTERPOSE(_replacment,_replacee)                   \
    __attribute__((used)) static struct {                       \
        const void* replacment;                                 \
        const void* replacee;                                   \
    } _interpose_ ## _replacee                                  \
    __attribute__ ((section ("__DATA, __interpose"))) = {       \
        (const void*) (unsigned long) &_replacment,             \
        (const void*) (unsigned long) &_replacee                \
    };
#else
#define DYLD_INTERPOSE(_replacement, _replacee)
#endif


/*
 * Prototypes
 */

NO_SANITIZE
static bool debug(void);

NO_SANITIZE
static bool isvalid(id_t id);

NO_SANITIZE
static void numtostr(long long num, char str[MAX_STR_LEN]);

NO_SANITIZE
static void unumtostr(unsigned long long num, char str[MAX_STR_LEN]);

NO_SANITIZE
static id_t strtoid(const char *str);

NO_SANITIZE
static void idtostr(id_t id, char str[MAX_STR_LEN]);

NO_SANITIZE
static int getnumenv(const char *const name, long long *num);

NO_SANITIZE
static id_t getidenv(const char *const name);

NO_SANITIZE
static int geterrno(const char *const name);

NO_SANITIZE
static void setnumenv(const char *const name, long long num);

NO_SANITIZE
static void setunumenv(const char *const name, unsigned long long num);

NO_SANITIZE
static void setidenv(const char *const name, const id_t id);

NO_SANITIZE
static uid_t getsuid(void);

NO_SANITIZE
static gid_t getsgid(void);


/*
 * Functions
 */

static bool
debug(void)
{
    errno = 0;
    char *var = getenv("MOCK_DEBUG");
    if (var == NULL) {
        if (errno == 0) {
            return false;
        }
        err(EXIT_FAILURE, "getenv MOCK_DEBUG");
    }

    return *var != '\0';
}

static bool
numisvalid(long long num) {
    return (INT_MIN + 1) < num && num < (INT_MAX - 1);
}

static bool
unumisvalid(unsigned long long num) {
    return num < UINT_MAX - 1U;
}

static bool
isvalid(id_t id) {
    if (ISSIGNED(id_t)) {
        return numisvalid((long long) id);
    } else {
        return unumisvalid((unsigned long long) id);
    }
}

static void
numtostr(long long num, char str[MAX_STR_LEN])
{
    int len;

    (void) memset(str, '\0', MAX_STR_LEN);

    errno = 0;
    len = snprintf(str, MAX_STR_LEN, "%lld", num);

    if (len < 0) {
        err(EXIT_FAILURE, "snprintf");
    }

    if ((size_t) len >= (size_t) MAX_STR_LEN) {
        errx(EXIT_FAILURE, "%lld is too large", num);
    }
}

static void
unumtostr(unsigned long long num, char str[MAX_STR_LEN])
{
    int len;

    (void) memset(str, '\0', MAX_STR_LEN);

    errno = 0;
    len = snprintf(str, MAX_STR_LEN, "%llu", num);

    if (len < 0) {
        err(EXIT_FAILURE, "snprintf");
    }

    if ((size_t) len >= (size_t) MAX_STR_LEN) {
        errx(EXIT_FAILURE, "%llu is too large", num);
    }
}

static void
idtostr(id_t id, char str[MAX_STR_LEN])
{
    if (ISSIGNED(gid_t)) {
        numtostr((long long) id, str);
    } else {
        unumtostr((unsigned long long) id, str);
    }
}

static id_t
strtoid(const char *const str)
{
    errno = 0;
    if (ISSIGNED(id_t)) {
        long long num;

        num = (long long) strtoll(str, NULL, 10);
        if (num == 0 && errno != 0) {
            err(EXIT_FAILURE, "strtoll");
        }

        if (!numisvalid(num)) {
            errx(EXIT_FAILURE, "%lld: not a valid GID", num);
        }

        return (id_t) num;
    } else {
        unsigned long long num;

        num = (unsigned long long) strtoull(str, NULL, 10);
        if (num == 0 && errno != 0) {
            err(EXIT_FAILURE, "strtoull");
        }

        if (!unumisvalid(num)) {
            errx(EXIT_FAILURE, "%llu: not a valid GID", num);
        }

        return (id_t) num;
    }
}

static id_t
getidenv(const char *const name)
{
    id_t id;

    assert(name != NULL);
    assert(*name != '\0');
    assert(strchr(name, '=') == NULL);

    errno = 0;
    char *value = getenv(name);
    if (value == NULL) {
        if (errno == 0) {
            id = 0;
        } else {
            err(EXIT_FAILURE, "getenv %s", name);
        }
    } else {
        id = strtoid(value);
    }

    if (debug()) {
        if (ISSIGNED(id_t)) {
            warnx("get %s=%lld", name, (long long) id);
        } else {
            warnx("get %s=%llu", name, (unsigned long long) id);
        }
    }

    return id;
}

static int
getnumenv(const char *const name, long long *num)
{
    assert(name != NULL);
    assert(*name != '\0');
    assert(strchr(name, '=') == NULL);
    assert(num != NULL);

    errno = 0;
    char *value = getenv(name);
    if (value == NULL) {
        if (errno == 0) {
            return -1;
        } else {
            err(EXIT_FAILURE, "getenv %s", name);
        }
    } else {
        errno = 0;
        *num = strtoll(value, NULL, 10);
        if (*num == 0 && errno != 0) {
            err(EXIT_FAILURE, "strtoll %s", value);
        }
    }

    return 0;
}

static void
setnumenv(const char *const name, long long num)
{
    char value[MAX_STR_LEN];

    assert(name != NULL);
    assert(*name != '\0');
    assert(strchr(name, '=') == NULL);

    numtostr(num, value);

    errno = 0;
    if (setenv(name, value, true) != 0) {
        err(EXIT_FAILURE, "setenv %s=%s", name, value);
    }
}

static void
setunumenv(const char *const name, unsigned long long num)
{
    char value[MAX_STR_LEN];

    assert(name != NULL);
    assert(*name != '\0');
    assert(strchr(name, '=') == NULL);

    unumtostr(num, value);

    errno = 0;
    if (setenv(name, value, true) != 0) {
        err(EXIT_FAILURE, "setenv %s=%s", name, value);
    }
}

static void
setidenv(const char *const name, const id_t id)
{
    if (ISSIGNED(id_t)) {
        setnumenv(name, (long long) id);

        if (debug()) {
            warnx("set %s=%lld", name, (long long) id);
        }
    } else {
        setunumenv(name, (unsigned long long) id);

        if (debug()) {
            warnx("set %s=%llu", name, (unsigned long long) id);
        }
    }
}

static uid_t
getsuid(void)
{
    return (uid_t) getidenv("MOCK_SUID");
}

static gid_t
getsgid(void)
{
    return (gid_t) getidenv("MOCK_SGID");
}

static int
geterrno(const char *const name)
{
    /* FIXME */
    long long num;

    if (getnumenv(name, &num) == 0) {
        if (num < INT_MIN) {
            err(EXIT_FAILURE, "%lld is smaller than %d", num, INT_MIN);
        }
        if (num > INT_MAX) {
            err(EXIT_FAILURE, "%lld is greater than %d", num, INT_MAX);
        }

        if (debug()) {
            warnx("get %s=%lld", name, num);
        }

        return (int) num;
    }

    return 0;
}

uid_t
mockgetuid(void)
{
    return getidenv("MOCK_UID");
}
DYLD_INTERPOSE(mockgetuid, getuid)

uid_t
mockgeteuid(void)
{
    return getidenv("MOCK_EUID");
}
DYLD_INTERPOSE(mockgeteuid, geteuid)

int
mocksetuid(const uid_t uid)
{
    uid_t euid;
    int mockerr;

    mockerr = geterrno("MOCK_SETUID_ERRNO");
    if (mockerr != 0) {
        errno = mockerr;
        return -1;
    }

    if (!isvalid(uid)) {
        errno = EINVAL;
        return -1;
    }

    euid = mockgeteuid();
    if (euid != 0 && getsuid() != 0 && uid != euid && uid != mockgetuid()) {
        errno = EPERM;
        return -1;
    }

    setidenv("MOCK_UID", uid);
    setidenv("MOCK_EUID", uid);
    setidenv("MOCK_SUID", uid);

    return 0;
}
DYLD_INTERPOSE(mocksetuid, setuid)

int
mockseteuid(const uid_t euid)
{
    int mockerr;

    mockerr = geterrno("MOCK_SETEUID_ERRNO");
    if (mockerr != 0) {
        errno = mockerr;
        return -1;
    }

    if (!isvalid(euid)) {
        errno = EINVAL;
        return -1;
    }

    if (mockgeteuid() != 0 && euid != mockgetuid() && euid != getsuid()) {
        errno = EPERM;
        return -1;
    }

    setidenv("MOCK_EUID", euid);

    return 0;
}
DYLD_INTERPOSE(mockseteuid, seteuid)

/*
 * setreuid and setregid accept -1 as ID. So -1 is a valid, if weird, ID.
 * But POSIX.1-2008 allows for uid_t, gid_t, and id_t to be defined as
 * unsigned integers, and that is how they are defined on most systems.
 * So IDs must be compared against -1 even if uid_t, gid_t, and id_t
 * are unsigned; in other words, the sign change is intentional.
 */
#if defined(__GNUC__) && (__GNUC__ > 2 || \
    (__GNUC__ >= 2 && __GNUC_MINOR__ >= 95))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif

int
mocksetreuid(const uid_t uid, const uid_t euid)
{
    uid_t curuid, cureuid;
    int mockerr;

    curuid = mockgetuid();
    cureuid = mockgeteuid();

    mockerr = geterrno("MOCK_SETREUID_ERRNO");
    if (mockerr != 0) {
        errno = mockerr;
        return -1;
    }

    if (uid != (uid_t) -1 && uid != curuid) {
        if (!isvalid(uid)) {
            errno = EINVAL;
            return -1;
        }

        if (cureuid != 0 && uid != cureuid) {
            errno = EPERM;
            return -1;
        }

        setidenv("MOCK_UID", uid);
    }

    if (euid != (uid_t) -1 && euid != cureuid) {
        if (!isvalid(euid)) {
            errno = EINVAL;
            return -1;
        }

        if (cureuid != 0 && euid != curuid) {
            errno = EPERM;
            return -1;
        }

        setidenv("MOCK_EUID", euid);
    }

    return 0;
}
DYLD_INTERPOSE(mocksetreuid, setreuid)

#if defined(__GNUC__) && (__GNUC__ > 2 || \
    (__GNUC__ >= 2 && __GNUC_MINOR__ >= 95))
#pragma GCC diagnostic pop
#endif

gid_t
mockgetgid(void)
{
    return getidenv("MOCK_GID");
}
DYLD_INTERPOSE(mockgetgid, getgid)

gid_t
mockgetegid(void)
{
    return getidenv("MOCK_EGID");
}
DYLD_INTERPOSE(mockgetegid, getegid)

int
mocksetgid(const gid_t gid)
{
    int mockerr;

    mockerr = geterrno("MOCK_SETGID_ERRNO");
    if (mockerr != 0) {
        errno = mockerr;
        return -1;
    }

    if (!isvalid(gid)) {
        errno = EINVAL;
        return -1;
    }

    if (mockgeteuid() != 0 && getsuid() != 0 &&
        gid != mockgetgid() && gid != mockgetegid())
    {
        errno = EPERM;
        return -1;
    }

    setidenv("MOCK_GID", gid);
    setidenv("MOCK_EGID", gid);
    setidenv("MOCK_SGID", gid);

    return 0;
}
DYLD_INTERPOSE(mocksetgid, setgid)

int
mocksetegid(const gid_t egid)
{
    int mockerr;

    mockerr = geterrno("MOCK_SETEGID_ERRNO");
    if (mockerr != 0) {
        errno = mockerr;
        return -1;
    }

    if (!isvalid(egid)) {
        errno = EINVAL;
        return -1;
    }

    if (mockgetegid() != 0 && egid != mockgetgid() && egid != getsgid()) {
        errno = EPERM;
        return -1;
    }

    setidenv("MOCK_EGID", egid);

    return 0;
}
DYLD_INTERPOSE(mocksetegid, setegid)

/* See above. */
#if defined(__GNUC__) && (__GNUC__ > 2 || \
    (__GNUC__ >= 2 && __GNUC_MINOR__ >= 95))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif

int
mocksetregid(const gid_t gid, const gid_t egid)
{
    uid_t cureuid;
    gid_t curgid, curegid;
    int mockerr;

    cureuid = mockgeteuid();
    curgid = mockgetgid();
    curegid = mockgetegid();

    mockerr = geterrno("MOCK_SETREGID_ERRNO");
    if (mockerr != 0) {
        errno = mockerr;
        return -1;
    }

    if (gid != (gid_t) -1 && gid != curgid) {
        if (!isvalid(gid)) {
            errno = EINVAL;
            return -1;
        }

        if (cureuid != 0 && gid != curegid) {
            errno = EPERM;
            return -1;
        }

        setidenv("MOCK_GID", gid);
    }

    if (egid != (gid_t) -1 && egid != curegid) {
        if (!isvalid(egid)) {
            errno = EINVAL;
            return -1;
        }

        if (cureuid != 0 && egid != curegid) {
            errno = EPERM;
            return -1;
        }

        setidenv("MOCK_EGID", egid);
    }

    return 0;
}
DYLD_INTERPOSE(mocksetregid, setregid)

#if defined(__GNUC__) && (__GNUC__ > 2 || \
    (__GNUC__ >= 2 && __GNUC_MINOR__ >= 95))
#pragma GCC diagnostic pop
#endif

int
mockgetgroups(int gidsetlen, gid_t *gidset) {
    char grps[MAX_STR_LEN] = {0};
    char *end, *sep;
    char *envgrps;
    char *grp;
    size_t nbytes;
    int ngrps;

    ngrps = 0;

    errno = 0;
    envgrps = getenv("MOCK_GROUPS");
    if (envgrps == NULL) {
        if (errno == 0) {
            if (debug()) {
                warnx("get MOCK_GROUPS=");
            }
            return 0;
        }
        err(EXIT_FAILURE, "getenv MOCK_GROUPS");
    }

    if (debug()) {
        warnx("get MOCK_GROUPS=%s", envgrps);
    }

    end = stpncpy(grps, envgrps, MAX_STR_LEN - 1U);
    nbytes = (size_t) (end - grps);
    if (envgrps[nbytes] != '\0') {
        err(EXIT_FAILURE, "MOCK_GROUPS is too long");
    }

    grp = grps;
    do {
        if (ngrps >= gidsetlen) {
            return -1;
        }

        sep = strchr(grps, ',');
        if (sep != NULL) {
            *sep = '\0';
        }

        gidset[ngrps++] = strtoid(grp);

        if (sep == NULL) {
            break;
        }

        grp = sep + 1U;
    } while (true);

    return ngrps;
}
DYLD_INTERPOSE(mockgetgroups, getgroups)

int
mocksetgroups(const NGRPS_T ngroups, const gid_t *const gidset)
{
    char grpsstr[MAX_STR_LEN];
    char *ptr, *lim;
    int mockerr;

    mockerr = geterrno("MOCK_SETGROUPS_ERRNO");
    if (mockerr != 0) {
        errno = mockerr;
        return -1;
    }

    if (mockgeteuid() != 0) {
        errno = EPERM;
        return -1;
    }

    if ((size_t) ngroups > (size_t) MAX_NGROUPS) {
        errno = EINVAL;
        return -1;
    }

    ptr = grpsstr;
    lim = ptr + MAX_STR_LEN - 1U;
    for (NGRPS_T i = 0; i < ngroups; ++i) {
        char grp[MAX_STR_LEN];
        size_t nbytes;

        if (ptr > grpsstr) {
            *(ptr++) = ',';
        }

        idtostr(gidset[i], grp);
        ptr = stpncpy(ptr, grp, (size_t) (lim - ptr));

        nbytes = (size_t) (ptr - grpsstr);
        if (grp[nbytes] != '\0') {
            err(EXIT_FAILURE, "too many GIDs");
        }
    }

    if (setenv("MOCK_GROUPS", grpsstr, true) != 0) {
        err(EXIT_FAILURE, "setenv %s=%s", "MOCK_GROUPS", grpsstr);
    }

    if (debug()) {
        warnx("set MOCK_GROUPS=%s", grpsstr);
    }

    return 0;
}
DYLD_INTERPOSE(mocksetgroups, setgroups)
