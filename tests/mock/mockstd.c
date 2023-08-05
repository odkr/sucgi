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

#include "../../attr.h"
#include "../../macros.h"
#include "../../params.h"
#include "mockstd.h"


/*
 * Interposition
 */

#if defined(__MACH__) && __MACH__ &&                            \
    defined(__GNUC__) &&                                        \
    (__GNUC__ > 3 || (__GNUC__ >= 3 && __GNUC_MINOR__ >= 1)) && \
    !defined(__ICC)

#include <unistd.h>

/* NOLINTBEGIN(performance-no-int-to-ptr) */

/* cppcheck-suppress [misra-c2012-20.10, misra-c2012-20.12]; ## is needed. */
#define DYLD_INTERPOSE(_replacement,_replacee)                  \
    __attribute__((used)) static struct {                       \
        const void *replacment;                                 \
        const void *replacee;                                   \
    } _interpose_ ## _replacee                                  \
    __attribute__((section("__DATA, __interpose"))) = {         \
        (const void *) (unsigned long) &(_replacement),         \
        (const void *) (unsigned long) &(_replacee)             \
    };

/* NOLINTEND(performance-no-int-to-ptr) */

#else
#define DYLD_INTERPOSE(_replacement, _replacee)
#endif


/*
 * Constantes
 */

/* The number 10. */
#define BASE_10 10


/*
 * Prototypes
 */

/* Check whether $MOCK_DEBUG is set. */
_no_sanitize_all _nodiscard
static bool debug(void);

/* Check whether a number falls into the range [INT_MIN + 1 .. INT_MAX + 1]. */
_no_sanitize_all _nodiscard
static bool num_is_valid(long long num);

/* Check whether a number is smaller than SIZE_MAX - 1. */
_no_sanitize_all _nodiscard
static bool unum_is_valid(unsigned long long num);

/* Check whether a number is a valid ID. */
_no_sanitize_all _nodiscard
static bool id_is_valid(id_t id);

/* Convert an integer to a string. */
_no_sanitize_all _write_only(3, 2) _nonnull(3)
static void num_to_str(long long num, size_t size, char *str);

/* Convert an unsigned integer to a string. */
_no_sanitize_all _write_only(3, 2) _nonnull(3)
static void unum_to_str(unsigned long long num, size_t size, char *str);

/* Convert an ID to a string. */
_no_sanitize_all _write_only(3, 2) _nonnull(3)
static void id_to_str(id_t id, size_t size, char *str);

/* Convert a string to an integer. */
_no_sanitize_all _read_only(1) _nonnull(1) _nodiscard
static long long str_to_num(const char *str);

/* Convert a string to an unsigned integer. */
_no_sanitize_all _read_only(1) _nonnull(1) _nodiscard
static unsigned long long str_to_unum(const char *str);

/* Convert a string to an ID. */
_no_sanitize_all _read_only(1) _nonnull(1) _nodiscard
static id_t str_to_id(const char *str);

/* Get an ID from an environment variable. */
_no_sanitize_all _read_only(1) _nonnull(1) _nodiscard
static id_t get_id_env(const char *name);

/* Get an error number from an environment variable. */
_no_sanitize_all _read_only(1) _nonnull(1) _nodiscard
static int get_errno(const char *name);

/* Set an environment variable to an integer value. */
_no_sanitize_all _read_only(1) _nonnull(1)
static void set_num_env(const char *name, long long num);

/* Set an environment variable to an unsigned integer value. */
_no_sanitize_all _read_only(1) _nonnull(1)
static void set_unum_env(const char *name, unsigned long long num);

/* Set an environment variable to an ID. */
_no_sanitize_all _read_only(1) _nonnull(1)
static void set_id_env(const char *name, id_t id);

/* Get the saved-user ID from the environment. */
_no_sanitize_all _nodiscard
static uid_t get_suid(void);

/* Get the saved-group ID from the environment. */
_no_sanitize_all _nodiscard
static gid_t get_sgid(void);


/*
 * Functions
 */

static bool
debug(void)
{
    errno = 0;
    /* RATS: ignore; used for debugging. */
    const char *const var = getenv("MOCK_DEBUG");
    if (var == NULL) {
        /* cppcheck-suppress misra-c2012-22.10; getenv sets errno. */
        if (errno == 0) {
            return false;
        }
        err(EXIT_FAILURE, "%s:%d: getenv MOCK_DEBUG", __FILE__, __LINE__);
    }

    return *var != '\0';
}

static bool
num_is_valid(const long long num) {
    return (INT_MIN + 1) < num && num < (INT_MAX - 1);
}

static bool
unum_is_valid(const unsigned long long num) {
    return num < UINT_MAX - 1U;
}

static bool
id_is_valid(const id_t id) {
    return ISSIGNED(id_t) ?
        num_is_valid((long long) id) :
        unum_is_valid((unsigned long long) id);
}

static void
num_to_str(const long long num, const size_t size, char *const str)
{
    int len;

    assert(size > 0U);
    assert(str != NULL);

    (void) memset(str, '\0', size);

    errno = 0;
    /* RATS: ignore; size is correctly given by all callees. */
    len = snprintf(str, size, "%lld", num);

    if (len < 0) {
        err(EXIT_FAILURE, "%s:%d: snprintf", __FILE__, __LINE__);
    }

    if ((size_t) len >= size) {
        errx(EXIT_FAILURE, "%s:%d: %lld has > %zu chars in base-10",
             __FILE__, __LINE__, num, size - 1U);
    }

    assert(str[len] == '\0');
}

static void
unum_to_str(const unsigned long long num, const size_t size, char *const str)
{
    int len;

    assert(size > 0U);
    assert(str != NULL);

    (void) memset(str, '\0', size);

    errno = 0;
    /* RATS: ignore; size is correctly given by all callers. */
    len = snprintf(str, size, "%llu", num);

    if (len < 0) {
        err(EXIT_FAILURE, "%s:%d: snprintf", __FILE__, __LINE__);
    }

    if ((size_t) len >= size) {
        errx(EXIT_FAILURE, "%s:%d: %llu has > %zu digits in base-10",
             __FILE__, __LINE__, num, size - 1U);
    }

    assert(str[len] == '\0');
}

static void
id_to_str(const id_t id, const size_t size, char *const str)
{
    if (ISSIGNED(id_t)) {
        num_to_str((long long) id, size, str);
    } else {
        unum_to_str((unsigned long long) id, size, str);
    }
}

static long long
str_to_num(const char *const str)
{
    long long num;

    assert(str != NULL);
    assert(*str != '\0');

    errno = 0;
    num = strtoll(str, NULL, BASE_10);
    if (num == (long long) 0 && errno != 0) {
        err(EXIT_FAILURE, "%s:%d: strtoll", __FILE__, __LINE__);
    }

    if (!num_is_valid(num)) {
        errx(EXIT_FAILURE, "%s:%d: %lld: not a valid ID",
             __FILE__, __LINE__, num);
    }

    return num;
}

static unsigned long long
str_to_unum(const char *const str)
{
    unsigned long long num;

    assert(str != NULL);
    assert(*str != '\0');

    errno = 0;
    num = strtoull(str, NULL, BASE_10);
    if (num == 0U && errno != 0) {
        err(EXIT_FAILURE, "%s:%d: strtoull", __FILE__, __LINE__);
    }

    if (!unum_is_valid(num)) {
        errx(EXIT_FAILURE, "%s:%d: %llu: not a valid ID",
             __FILE__, __LINE__, num);
    }

    return num;
}

static id_t
str_to_id(const char *const str)
{
    return ISSIGNED(id_t) ?
        (id_t) str_to_num(str) :
        (id_t) str_to_unum(str);
}

static id_t
get_id_env(const char *const name)
{
    assert(name != NULL);
    assert(*name != '\0');
    assert(strchr(name, '=') == NULL);

    errno = 0;
    /* RATS: ignore; used for debugging. */
    const char *const value = getenv(name);
    if (value == NULL) {
        /* cppcheck-suppress misra-c2012-22.10; getenv sets errno. */
        if (errno == 0) {
            return (id_t) 0;
        }

        err(EXIT_FAILURE, "%s:%d: getenv %s", __FILE__, __LINE__, name);
    }

    const id_t id = str_to_id(value);
    if (debug()) {
        if (ISSIGNED(id_t)) {
            warnx("get %s=%lld", name, (long long) id);
        } else {
            warnx("get %s=%llu", name, (unsigned long long) id);
        }
    }

    return id;
}

static void
set_num_env(const char *const name, const long long num)
{
    /* RATS: ignore; num_to_str is bounded by sizeof(value). */
    char value[MAX_STR_LEN];

    assert(name != NULL);
    assert(*name != '\0');
    assert(strchr(name, '=') == NULL);

    num_to_str(num, sizeof(value), value);

    errno = 0;
    if (setenv(name, value, true) != 0) {
        err(EXIT_FAILURE, "%s:%d: setenv %s=%s",
            __FILE__, __LINE__, name, value);
    }
}

static void
set_unum_env(const char *const name, const unsigned long long num)
{
    /* RATS: ignore; unum_to_str is bounded by sizeof(value). */
    char value[MAX_STR_LEN];

    assert(name != NULL);
    assert(*name != '\0');
    assert(strchr(name, '=') == NULL);

    unum_to_str(num, sizeof(value), value);

    errno = 0;
    if (setenv(name, value, true) != 0) {
        err(EXIT_FAILURE, "%s:%d: setenv %s=%s",
            __FILE__, __LINE__, name, value);
    }
}

static void
set_id_env(const char *const name, const id_t id)
{
    if (ISSIGNED(id_t)) {
        set_num_env(name, (long long) id);

        if (debug()) {
            warnx("set %s=%lld", name, (long long) id);
        }
    } else {
        set_unum_env(name, (unsigned long long) id);

        if (debug()) {
            warnx("set %s=%llu", name, (unsigned long long) id);
        }
    }
}

static uid_t
get_suid(void)
{
    return (uid_t) get_id_env("MOCK_SUID");
}

static gid_t
get_sgid(void)
{
    return (gid_t) get_id_env("MOCK_SGID");
}

static int
get_errno(const char *const name)
{
    long long num = 0;

    assert(name != NULL);
    assert(*name != '\0');

    errno = 0;
    /* RATS: ignore; used for debugging. */
    const char *const value = getenv(name);
    if (value == NULL) {
        /* cppcheck-suppress misra-c2012-22.10; getenv sets errno. */
        if (errno == 0) {
            return 0;
        }

        err(EXIT_FAILURE, "%s:%d: getenv %s", __FILE__, __LINE__, name);
    }

    errno = 0;
    num = strtoll(value, NULL, BASE_10);

    if (num == (long long) 0 && errno != 0) {
        err(EXIT_FAILURE, "%s:%d: strtoll %s", __FILE__, __LINE__, value);
    }

    if (num < INT_MIN) {
        err(EXIT_FAILURE, "%s:%d: %lld is smaller than %d",
            __FILE__, __LINE__, num, INT_MIN);
    }

    if (num > INT_MAX) {
        err(EXIT_FAILURE, "%s:%d: %lld is greater than %d",
            __FILE__, __LINE__, num, INT_MAX);
    }

    if (debug()) {
        warnx("%s:%d: get %s=%lld", __FILE__, __LINE__, name, num);
    }

    return (int) num;
}

uid_t
mock_getuid(void)
{
    return get_id_env("MOCK_UID");
}
DYLD_INTERPOSE(mock_getuid, getuid)

uid_t
mock_geteuid(void)
{
    return get_id_env("MOCK_EUID");
}
DYLD_INTERPOSE(mock_geteuid, geteuid)

int
mock_setuid(const uid_t uid)
{
    uid_t euid;
    int mockerr;

    mockerr = get_errno("MOCK_SETUID_ERRNO");
    if (mockerr != 0) {
        errno = mockerr;
        return -1;
    }

    if (!id_is_valid(uid)) {
        errno = EINVAL;
        return -1;
    }

    euid = mock_geteuid();
    if (euid != 0 && get_suid() != 0 && uid != euid && uid != mock_getuid()) {
        errno = EPERM;
        return -1;
    }

    set_id_env("MOCK_UID", uid);
    set_id_env("MOCK_EUID", uid);
    set_id_env("MOCK_SUID", uid);

    return 0;
}
DYLD_INTERPOSE(mock_setuid, setuid)

int
mock_seteuid(const uid_t euid)
{
    int mockerr;

    mockerr = get_errno("MOCK_SETEUID_ERRNO");
    if (mockerr != 0) {
        errno = mockerr;
        return -1;
    }

    if (!id_is_valid(euid)) {
        errno = EINVAL;
        return -1;
    }

    if (mock_geteuid() != 0 && euid != mock_getuid() && euid != get_suid()) {
        errno = EPERM;
        return -1;
    }

    set_id_env("MOCK_EUID", euid);

    return 0;
}
DYLD_INTERPOSE(mock_seteuid, seteuid)

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
mock_setreuid(const uid_t uid, const uid_t euid)
{
    uid_t cur_uid;
    uid_t cur_euid;
    int mockerr;

    cur_uid = mock_getuid();
    cur_euid = mock_geteuid();

    mockerr = get_errno("MOCK_SETREUID_ERRNO");
    if (mockerr != 0) {
        errno = mockerr;
        return -1;
    }

    if (uid != (uid_t) -1 && uid != cur_uid) {
        if (!id_is_valid(uid)) {
            errno = EINVAL;
            return -1;
        }

        if (cur_euid != 0 && uid != cur_euid) {
            errno = EPERM;
            return -1;
        }

        set_id_env("MOCK_UID", uid);
    }

    if (euid != (uid_t) -1 && euid != cur_euid) {
        if (!id_is_valid(euid)) {
            errno = EINVAL;
            return -1;
        }

        if (cur_euid != 0 && euid != cur_uid) {
            errno = EPERM;
            return -1;
        }

        set_id_env("MOCK_EUID", euid);
    }

    return 0;
}
DYLD_INTERPOSE(mock_setreuid, setreuid)

#if defined(__GNUC__) && (__GNUC__ > 2 || \
    (__GNUC__ >= 2 && __GNUC_MINOR__ >= 95))
#pragma GCC diagnostic pop
#endif

gid_t
mock_getgid(void)
{
    return get_id_env("MOCK_GID");
}
DYLD_INTERPOSE(mock_getgid, getgid)

gid_t
mock_getegid(void)
{
    return get_id_env("MOCK_EGID");
}
DYLD_INTERPOSE(mock_getegid, getegid)

int
mock_setgid(const gid_t gid)
{
    int mockerr;

    mockerr = get_errno("MOCK_SETGID_ERRNO");
    if (mockerr != 0) {
        errno = mockerr;
        return -1;
    }

    if (!id_is_valid(gid)) {
        errno = EINVAL;
        return -1;
    }

    if (mock_geteuid() != 0 && get_suid() != 0 &&
        gid != mock_getgid() && gid != mock_getegid())
    {
        errno = EPERM;
        return -1;
    }

    set_id_env("MOCK_GID", gid);
    set_id_env("MOCK_EGID", gid);
    set_id_env("MOCK_SGID", gid);

    return 0;
}
DYLD_INTERPOSE(mock_setgid, setgid)

int
mock_setegid(const gid_t egid)
{
    int mockerr;

    mockerr = get_errno("MOCK_SETEGID_ERRNO");
    if (mockerr != 0) {
        errno = mockerr;
        return -1;
    }

    if (!id_is_valid(egid)) {
        errno = EINVAL;
        return -1;
    }

    if (mock_getegid() != 0 && egid != mock_getgid() && egid != get_sgid()) {
        errno = EPERM;
        return -1;
    }

    set_id_env("MOCK_EGID", egid);

    return 0;
}
DYLD_INTERPOSE(mock_setegid, setegid)

/* See above. */
#if defined(__GNUC__) && (__GNUC__ > 2 || \
    (__GNUC__ >= 2 && __GNUC_MINOR__ >= 95))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif

int
mock_setregid(const gid_t gid, const gid_t egid)
{
    uid_t cur_euid;
    gid_t cur_gid;
    gid_t cur_egid;
    int mockerr;

    cur_euid = mock_geteuid();
    cur_gid = mock_getgid();
    cur_egid = mock_getegid();

    mockerr = get_errno("MOCK_SETREGID_ERRNO");
    if (mockerr != 0) {
        errno = mockerr;
        return -1;
    }

    if (gid != (gid_t) -1 && gid != cur_gid) {
        if (!id_is_valid(gid)) {
            errno = EINVAL;
            return -1;
        }

        if (cur_euid != 0 && gid != cur_egid) {
            errno = EPERM;
            return -1;
        }

        set_id_env("MOCK_GID", gid);
    }

    if (egid != (gid_t) -1 && egid != cur_egid) {
        if (!id_is_valid(egid)) {
            errno = EINVAL;
            return -1;
        }

        if (cur_euid != 0) {
            errno = EPERM;
            return -1;
        }

        set_id_env("MOCK_EGID", egid);
    }

    return 0;
}
DYLD_INTERPOSE(mock_setregid, setregid)

#if defined(__GNUC__) && (__GNUC__ > 2 || \
    (__GNUC__ >= 2 && __GNUC_MINOR__ >= 95))
#pragma GCC diagnostic pop
#endif

int
mock_getgroups(int gidsetlen, gid_t *gidset) {
    /* RATS: ignore; stpncpy is bounded by sizeof(groups) - 1U. */
    char groups[MAX_STR_LEN] = {0};
    const char *envgroups;
    char *end;
    char *sep;
    char *group;
    size_t nbytes;
    int ngroups;

    ngroups = 0;

    errno = 0;
    /* RATS: ignore; used for debugging. */
    envgroups = getenv("MOCK_GROUPS");
    if (envgroups == NULL) {
        /* cppcheck-suppress misra-c2012-22.10; getenv sets errno. */
        if (errno == 0) {
            if (debug()) {
                warnx("get MOCK_GROUPS=");
            }
            return 0;
        }
        err(EXIT_FAILURE, "%s:%d: getenv MOCK_GROUPS", __FILE__, __LINE__);
    }

    if (debug()) {
        warnx("get MOCK_GROUPS=%s", envgroups);
    }

    end = stpncpy(groups, envgroups, sizeof(groups) - 1U);

    assert(end != NULL);
    assert(end >= groups);
    assert(*end == '\0');

    /*
     * The cast is safe and portable.
     * And MISRA C permits pointer subtraction.
     */
    nbytes = (size_t) /* cppcheck-suppress misra-c2012-10.8 */
             (end - groups); /* cppcheck-suppress misra-c2012-18.4 */

    if (envgroups[nbytes] != '\0') {
        err(EXIT_FAILURE, "%s:%d: MOCK_GROUPS is too long",
            __FILE__, __LINE__);
    }

    group = groups;
    do {
        if (ngroups >= gidsetlen) {
            return -1;
        }

        sep = strchr(groups, ',');
        if (sep != NULL) {
            *sep = '\0';
        }

        gidset[ngroups] = str_to_id(group);
        ++ngroups;

        if (sep == NULL) {
            break;
        }

        group = &sep[1];
    } while (true);

    return ngroups;
}
DYLD_INTERPOSE(mock_getgroups, getgroups)

int
mock_setgroups(const NGRPS_T ngroups, const gid_t *const gidset)
{
    /* RATS: ignore; stpncpy is bounded by lim. */
    char groupsstr[MAX_STR_LEN];
    char *ptr;
    char *lim;
    int mockerr;

    assert(gidset != NULL);

    (void) memset(groupsstr, '\0', sizeof(groupsstr));

    mockerr = get_errno("MOCK_SETGROUPS_ERRNO");
    if (mockerr != 0) {
        errno = mockerr;
        return -1;
    }

    if (mock_geteuid() != 0) {
        errno = EPERM;
        return -1;
    }

    if ((size_t) ngroups > (size_t) MAX_NGROUPS) {
        errno = EINVAL;
        return -1;
    }

    ptr = groupsstr;
    lim = &groupsstr[sizeof(groupsstr) - 1U];
    /* cppcheck-suppress misra-c2012-14.2; for-loop is well-formed. */
    for (NGRPS_T i = 0; i < ngroups; ++i) {
        /* RATS: ignore; id_to_str is bounded by sizeof(group). */
        char group[MAX_STR_LEN];
        size_t nbytes;
        size_t len;

        if (ptr > groupsstr) {
            *ptr = ',';
            ++ptr;
        }

        id_to_str(gidset[i], sizeof(group), group);

        assert(lim >= ptr);

        /*
         * These casts are safe and portable.
         * And MISRA C permits pointer subtraction.
         */
        len = (size_t)          /* cppcheck-suppress misra-c2012-10.8 */
              (lim - ptr);      /* cppcheck-suppress misra-c2012-18.4 */

        ptr = stpncpy(ptr, group, len);

        nbytes = (size_t)           /* cppcheck-suppress misra-c2012-10.8 */
                 (ptr - groupsstr); /* cppcheck-suppress misra-c2012-18.4 */

        if (group[nbytes] != '\0') {
            err(EXIT_FAILURE, "%s:%d: too many GIDs", __FILE__, __LINE__);
        }
    }

    if (setenv("MOCK_GROUPS", groupsstr, true) != 0) {
        err(EXIT_FAILURE, "%s:%d: setenv MOCK_GROUPS=%s",
            __FILE__, __LINE__, groupsstr);
    }

    if (debug()) {
        warnx("set MOCK_GROUPS=%s", groupsstr);
    }

    return 0;
}
DYLD_INTERPOSE(mock_setgroups, setgroups)
