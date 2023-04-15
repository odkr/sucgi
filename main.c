/*
 * Run CGI scripts with the permissions of their owner.
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

#if defined(__OPTIMIZE__) && !defined(_FORTIFY_SOURCE)
#define _FORTIFY_SOURCE 3
#endif

#include <sys/stat.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <limits.h>
#include <grp.h>
#include <pwd.h>
#include <regex.h>
#include <stdbool.h>
#include <stdlib.h>
/* cppcheck-suppress misra-c2012-21.6; needed for -h, -C, and -V. */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include "compat.h"
#include "config.h"
#include "env.h"
#include "error.h"
#include "file.h"
#include "handler.h"
#include "max.h"
#include "macros.h"
#include "path.h"
#include "priv.h"
#include "str.h"
#include "testing.h"
#include "types.h"
#include "userdir.h"


/*
 * Configuration checks
 */

#if MIN_UID <= 0
#error MIN_UID must be greater than 0.
#endif

#if MAX_UID < MIN_UID
#error MAX_UID is smaller than MIN_UID.
#endif

#if MAX_UID > INT_MAX
#error MAX_UID is greater than INT_MAX.
#endif

#if MIN_GID <= 0
#error MIN_GID must be greater than 0.
#endif

#if MAX_GID < MIN_GID
#error MAX_GID is smaller than MIN_GID.
#endif

#if MAX_GID > INT_MAX
#error MAX_GID is greater than INT_MAX.
#endif


/*
 * Constants
 */

/* suCGI version. */
#define VERSION "0"


/*
 * Prototypes
 */

/* Print help. */
static void help(void);

/* Print build configuration. */
static void config(void);

/* Print version. */
static void version(void);

/* Print usage information to stderr and exit with EXIT_FAILURE. */
__attribute__((noreturn))
static void usage(void);


/*
 * Functions
 */

static void
help(void)
{
    (void) puts(
"suCGI - run CGI scripts with the permissions of their owner\n\n"
"Usage:  sucgi\n"
"        sucgi [-C|-V|-h]\n\n"
"Options:\n"
"    -C  Print build configuration.\n"
"    -V  Print version and license.\n"
"    -h  Print this help screen."
    );
}

static void
config(void)
{
    /* NOLINTNEXTLINE(bugprone-suspicious-missing-comma); literals intended. */
    const char *const env_patterns[] = ENV_PATTERNS;
    const char *const deny_groups[] = DENY_GROUPS;
    const Pair handlers[] = HANDLERS;

    (void) printf("#\n# Configuration\n#\n\n");

    (void) printf("USER_DIR=\"%s\"\n", USER_DIR);

    (void) printf("MIN_UID=%d\n", MIN_UID);
    (void) printf("MAX_UID=%d\n", MAX_UID);
    (void) printf("MIN_GID=%d\n", MIN_GID);
    (void) printf("MAX_GID=%d\n", MAX_GID);

    (void) printf("ALLOW_GROUP=\"%s\"\n", ALLOW_GROUP);

    (void) printf("DENY_GROUPS=\"");
    for (size_t i = 0; i < NELEMS(deny_groups); ++i) {
        if (i > 0U) {
            (void) printf(",");
        }
        (void) printf("%s", deny_groups[i]);
    }
    (void) printf("\"\n");

    (void) printf("ENV_PATTERNS=\"\n");
    for (size_t i = 0; i < NELEMS(env_patterns); ++i) {
        (void) printf("\t%s\n", env_patterns[i]);
    }
    (void) printf("\"\n");

    (void) printf("HANDLERS=\"");
    for (size_t i = 0; i < NELEMS(handlers); ++i) {
        /* cppcheck-suppress knownConditionTrueFalse;
           there could be more handlers. */
        if (i > 0U) {
            (void) printf(",");
        }
        (void) printf("%s=%s", handlers[i].key, handlers[i].value);
    }
    (void) printf("\"\n");

    (void) printf("LOGGING_FACILITY=%d\n", LOGGING_FACILITY);
    (void) printf("LOGGING_MASK=%d\n", LOGGING_MASK);
    (void) printf("LOGGING_OPTIONS=%d\n", LOGGING_OPTIONS);

    (void) printf("PATH=\"%s\"\n", PATH);
    (void) printf("UMASK=0%o\n", (unsigned int) UMASK);

    (void) printf("\n\n#\n# Limits\n#\n\n");

    (void) printf("MAX_STR_LEN=%u\n", MAX_STR_LEN);
    (void) printf("MAX_ERRMSG_LEN=%u\n", MAX_ERRMSG_LEN);
    (void) printf("MAX_FNAME_LEN=%d\n", MAX_FNAME_LEN);
    (void) printf("MAX_GRPNAME_LEN=%lld\n", (long long) MAX_GRPNAME_LEN);
    (void) printf("MAX_SUFFIX_LEN=%u\n", MAX_SUFFIX_LEN);
    (void) printf("MAX_VAR_LEN=%d\n", MAX_VAR_LEN);
    (void) printf("MAX_VARNAME_LEN=%u\n", MAX_VARNAME_LEN);
    (void) printf("MAX_NGROUPS=%u\n", MAX_NGROUPS);
    (void) printf("MAX_NVARS=%u\n", MAX_NVARS);

    (void) printf("\n\n#\n# Debugging\n#\n\n");

#if defined(NDEBUG)
    (void) printf("NDEBUG=%d\n", NDEBUG);
#else
    (void) printf("#NDEBUG=\n");
#endif
#if defined(CHECK) && CHECK
    (void) printf("CHECK=%d\n", CHECK);
#else
    (void) printf("#CHECK=\n");
#endif

    (void) printf("\n\n#\n# System\n#\n\n");

#if defined(__GLIBC__)
    (void) printf("LIBC=glibc\n");
#elif defined(__GNU_LIBRARY__)
    (void) printf("LIBC=glibc\n");
#elif defined(__KLIBC__)
    (void) printf("LIBC=klibc\n");
#elif defined(__UCLIBC__)
    (void) printf("LIBC=uClibc\n");
#elif defined(__bsdi__)
    (void) printf("LIBC=BSDi\n");
#elif defined(__DragonFly__)
    (void) printf("LIBC=DragonFly\n");
#elif defined(__FreeBSD__)
    (void) printf("LIBC=FreeBSD\n");
#elif defined(__NetBSD__)
    (void) printf("LIBC=NetBSD\n");
#elif defined(__OpenBSD__)
    (void) printf("LIBC=OpenBSD\n");
#elif defined(__MACH__)
    (void) printf("LIBC=Apple\n");
#else
    (void) printf("#LIBC=?\n");
#endif
}

static void
version(void)
{
    (void) printf(
"suCGI v%s.\n"
"Copyright 2022 and 2023 Odin Kroeger.\n"
"Released under the GNU General Public License.\n"
"This programme comes with ABSOLUTELY NO WARRANTY.\n",
           VERSION);
}

static void
usage(void)
{
    (void) fputs("usage: sucgi [-C|-V|-h]\n", stderr);
    /* cppcheck-suppress misra-c2012-21.8; exit is the least bad option. */
    exit(EXIT_FAILURE);
}


/*
 * Main
 */

int
main(int argc, char **argv) {
    Error ret;


    /*
     * Check system-dependent types.
     */

    ERRORIF(sizeof(GRP_T) != sizeof(gid_t));
    ERRORIF(SIGNEDMAX(NGRPS_T) < SIGNEDMAX(int));


    /*
     * Check whether configuration values are within bounds.
     */

    ERRORIF(sizeof(USER_DIR) <= 1U);
    ERRORIF(sizeof(USER_DIR) >= (size_t) MAX_FNAME_LEN);
    ERRORIF(sizeof(PATH) >= (size_t) MAX_FNAME_LEN);
    ERRORIF(sizeof(ALLOW_GROUP) < 1U);
    ERRORIF(sizeof(ALLOW_GROUP) >= MAX_GRPNAME_LEN);
    ERRORIF((uint64_t) MAX_UID > (uint64_t) SIGNEDMAX(uid_t));
    ERRORIF((uint64_t) MAX_GID > (uint64_t) SIGNEDMAX(gid_t));
    ERRORIF((uint64_t) MAX_GID > (uint64_t) SIGNEDMAX(GRP_T));


    /*
     * Words of wisdom from the authors of suEXEC:
     *
     * > While cleaning the environment, the environment should be clean.
     * > (E.g. malloc() may get the name of a file for writing debugging
     * > info. Bad news if MALLOC_DEBUG_FILE is set to /etc/passwd.)
     */

    char *const *vars;
    char *null;

    null = NULL;
    vars = environ;
    environ = &null;


    /*
     * Set up logging.
     */

    openlog("sucgi", LOGGING_OPTIONS, LOGGING_FACILITY);
    errno = 0;
    if (atexit(closelog) != 0) {
        error("atexit: %m");
    }

    (void) setlogmask(LOGGING_MASK);


    /*
     * Drop privileges temporarily.
     */

    ret = privsuspend();
    switch (ret) {
    case OK:
        break;
    default:
        /* Should be unreachable. */
        error("%d: privsuspend returned %d.", __LINE__, ret);
    }

    assert(getuid() == geteuid());
    assert(getgid() == getegid());
    /*
     * getgroups is unreliable on some systems,
     * so supplementary groups cannot be verified.
     */


    /*
     * Parse arguments.
     */

    /* Systems differ on whether argc may be 0. */
    if (argc == 0 || *argv == NULL || **argv == '\0') {
        error("empty argument vector.");
    }

    switch (argc) {
    case 1:
        break;
    case 2:
        /* Some getopt implementations are insecure. */
        if        (strncmp(argv[1], "-h", sizeof("-h")) == 0) {
            help();
        } else if (strncmp(argv[1], "-C", sizeof("-C")) == 0) {
            config();
        } else if (strncmp(argv[1], "-V", sizeof("-V")) == 0) {
            version();
        } else {
            usage();
        }
        return EXIT_SUCCESS;
    default:
        usage();
    }


    /*
     * Restore the environment variables used by CGI scripts.
     */

    /* NOLINTNEXTLINE(bugprone-suspicious-missing-comma); literals intended. */
    const char *const patterns[] = ENV_PATTERNS;
    regex_t pregs[NELEMS(patterns)];

    for (size_t i = 0; i < NELEMS(patterns); ++i) {
        int err;

        err = regcomp(&pregs[i], patterns[i], REG_EXTENDED | REG_NOSUB);
        if (err != 0) {
            /* RATS: ignore; regerror respects the size of errmsg. */
            char errmsg[MAX_ERRMSG_LEN];

            /*
             * TODO: Document that regular expression error
             *       messages may get truncated.
             */
            (void) regerror(err, &pregs[i], errmsg, sizeof(errmsg));
            error("regcomp: %s", errmsg);
        }
    }

    ret = envrestore((const char *const *) vars, NELEMS(pregs), pregs);
    switch (ret) {
    case OK:
        break;
    case ERR_LEN:
        error("too many environment variables.");
    case ERR_SYS:
        error("setenv: %m.");
    default:
        /* Should be unreachable. */
        error("%d: envrestore returned %d.", __LINE__, ret);
    }


    /*
     * Get the script's filename and filesystem metadata.
     */

    /* RATS: ignore; copystr is bounded by MAX_FNAME_LEN. */
    char script_log[MAX_VAR_LEN];
    const char *script_phys;
    struct stat script_stat;

    ret = envcopyvar("PATH_TRANSLATED", script_log);
    switch (ret) {
    case OK:
        break;
    case ERR_SEARCH:
        error("$PATH_TRANSLATED: not set.");
    default:
        /* Should be unreachable. */
        error("%d: envcopyvar returned %d.", __LINE__, ret);
    }

    if (*script_log == '\0') {
        error("$PATH_TRANSLATED: empty.");
    }

    ret = pathreal(script_log, &script_phys);
    switch (ret) {
    case OK:
        break;
    case ERR_SYS:
        error("realpath %s: %m.", script_log);
    default:
        /* Should be unreachable. */
        error("%d: pathreal returned %d.", __LINE__, ret);
    }

    assert(script_phys != NULL);
    assert(*script_phys != '\0');
    assert(strnlen(script_phys, MAX_FNAME_LEN) < (size_t) MAX_FNAME_LEN);

    if (stat(script_phys, &script_stat) != 0) {
        /* Only reachable if the script was deleted after realpath. */
        error("stat %s: %m.", script_log);
    }

    if ((script_stat.st_mode & S_IFREG) == 0) {
        error("script %s: not a regular file.", script_log);
    }


    /*
     * Check if the script is owned by a regular user.
     */

    const struct passwd *owner;
    const char *logname;
    uid_t uid;
    gid_t gid;

    errno = 0;
    /* cppcheck-suppress getpwuidCalled; suCGI need not be async-safe. */
    owner = getpwuid(script_stat.st_uid);
    if (owner == NULL) {
        /* cppcheck-suppress misra-c2012-22.10; getpwuid may set errno. */
        if (errno == 0) {
            error("script %s: no owner.", script_log);
        } else {
            error("getpwuid: %m.");
        }
    }

    assert(owner->pw_uid == script_stat.st_uid);

    if (owner->pw_uid < MIN_UID || owner->pw_uid > MAX_UID) {
        error("script %s: owned by privileged user.", script_log);
    }

    logname = owner->pw_name;
    uid = owner->pw_uid;
    gid = owner->pw_gid;


    /*
     * Check the owner's group memberships.
     */

    gid_t groups[MAX_NGROUPS];
    int ngroups;

    /*
     * GRP_T refers to the type that getgrouplist takes and returns GIDs as;
     * namely, int (on older systems and macOS) or gid_t (modern systems).
     *
     * Casting gid_t to GRP_T is guaranteed to be safe because:
     * (1) a compile-time error is raised if sizeof(GRP_T) != sizeof(gid_t)
     *     (so GRP_T[i] and gid_t[i] cannot refer to different addresses).
     * (2) gid_t and GRP_T use the same integer representation for any value
     *     in [MIN_GID .. MAX_GID] (so type-casting cannot change values).
     * (3) a run-time error is raised if a GID falls outside that range.
     *
     * gid_t and GRP_T are guaranteed to use the same integer representation
     * for any value in that range because a compile-time error is raised if:
     * (1) MIN_GID < 1 (so values cannot change sign);
     * (2) MAX_GID > the greatest signed value that gid_t/GRP_T could hold
     *     if they were signed data types (so values cannot overflow).
     */

    ngroups = MAX_NGROUPS;
    if (getgrouplist(logname, (GRP_T) gid, (GRP_T *) groups, &ngroups) < 0) {
        error("user %s: belongs to too many groups.", logname);
    }

    if (ngroups < 0) {
        /* Should be unreachable. */
        error("%d: getgrouplist returned %d groups for user %s.",
              __LINE__, ngroups, logname);
    }

    if (strncmp(ALLOW_GROUP, "", 1) != 0) {
        const struct group *allowedgrp;
        bool allowed;

        errno = 0;
        /* cppcheck-suppress getgrnamCalled; suCGI need not be async-safe. */
        allowedgrp = getgrnam(ALLOW_GROUP);
        if (allowedgrp == NULL) {
            /* cppcheck-suppress misra-c2012-22.10; getgrnam may set errno. */
            if (errno == 0) {
                error("group %s: no such group.", ALLOW_GROUP);
            } else {
                error("getgrnam: %m.");
            }
        }

        allowed = false;
        for (int i = 0; i < ngroups; ++i) {
            if (groups[i] == allowedgrp->gr_gid) {
                allowed = true;
                break;
            }
        }

        if (!allowed) {
            error("user %s: not a member of group %s.", logname, ALLOW_GROUP);
        }
    }

    for (int i = 0; i < ngroups; ++i) {
        const char *const deniedgrps[] = DENY_GROUPS;
        const struct group *grp;

        errno = 0;
        /* cppcheck-suppress getgrgidCalled; suCGI need not be async-safe. */
        grp = getgrgid(groups[i]);
        if (grp == NULL) {
            /* cppcheck-suppress misra-c2012-22.10; getgrgid may set errno. */
            if (errno == 0) {
                /*
                 * Should only be reachable if a group was deleted between
                 * the time getgrouplist returned and getgrgid was called.
                 */
                if (ISSIGNED(gid_t)) {
                    error("user %s: member of non-existing group %lld.",
                          logname, (long long) groups[i]);
                } else {
                    error("user %s: member of non-existing group %llu.",
                          logname, (unsigned long long) groups[i]);
                }
            } else {
                error("getgrgid: %m.");
            }
        }

        if (grp->gr_gid < MIN_GID || grp->gr_gid > MAX_GID) {
            error("user %s: member of privileged group %s.",
                  logname, grp->gr_name);
        }

        for (size_t j = 0; j < NELEMS(deniedgrps); ++j) {
            if (fnmatch(deniedgrps[j], grp->gr_name, 0) == 0) {
                error("user %s: member of denied group %s.",
                      logname, grp->gr_name);
            }
        }
    }


    /*
     * Drop privileges for good.
     */

    long ngroups_max;

    ngroups_max = sysconf(_SC_NGROUPS_MAX);
    if (-1L < ngroups_max && ngroups_max < ngroups) {
        /* RATS: ignore; message is short and a literal. */
        syslog(LOG_NOTICE, "user %s: can only set %ld of %d groups.",
               logname, ngroups_max, ngroups);

        /* ngroups_max cannot be larger than INT_MAX. */
        ngroups = (int) ngroups_max;
    }

    errno = 0;
    if (seteuid(0) != 0) {
        /*
         * Should only be reachable if sucgi's set-user-ID on
         * execute bit is unset or suCGI is not owned by root.
         */
        error("seteuid: %m.");
    }

    /*
     * NGRPS_T refers to the data type that setgroups takes the number of
     * groups as; that type may be size_t (Linux) or int (any other system).
     *
     * Casting int to NGRPS_T is guaranteed to be safe because:
     * (1) ngroups cannot be negative (so values cannot change sign);
     * (2) a compile-time error is raised if NGRPS_T is too small
     *     to hold INT_MAX (so values cannot overflow).
     */
    ret = privdrop(uid, gid, (NGRPS_T) ngroups, groups);
    switch (ret) {
    case OK:
        break;
    case ERR_SYS:
        error("privilege drop: %m.");
    default:
        /* Should be unreachable. */
        error("%d: privdrop returned %d.", __LINE__, ret);
    }

    assert(geteuid() == uid);
    assert(getegid() == gid);
    assert(getuid() == uid);
    assert(getgid() == gid);
    /*
     * getgroups is unreliable on some systems,
     * so supplementary groups cannot be verified.
     */


    /*
     * Check whether the script is within the user directory.
     */

    /* RATS: ignore; userdirexp repects MAX_FNAME_LEN. */
    char userdir_log[MAX_FNAME_LEN];
    const char *userdir_phys;

    ret = userdirexp(USER_DIR, owner, userdir_log);
    switch (ret) {
    case OK:
        break;
    case ERR_LEN:
        error("user %s: user directory is too long.", logname);
    case ERR_SYS:
        error("snprintf: %m.");
    default:
        /* Should be unreachable. */
        error("%d: userdirexp returned %d.", __LINE__, ret);
    }

    assert(userdir_log != NULL);
    assert(*userdir_log != '\0');
    assert(strnlen(userdir_log, MAX_FNAME_LEN) < (size_t) MAX_FNAME_LEN);

    ret = pathreal(userdir_log, &userdir_phys);
    switch (ret) {
    case OK:
        break;
    case ERR_LEN:
        error("user %s: user directory is too long.", logname);
    case ERR_SYS:
        error("realpath %s: %m.", userdir_log);
    default:
        /* Should be unreachable. */
        error("%d: pathreal returned %d.", __LINE__, ret);
    }

    assert(userdir_phys != NULL);
    assert(*userdir_phys != '\0');
    assert(strnlen(userdir_phys, MAX_FNAME_LEN) < (size_t) MAX_FNAME_LEN);

    ret = pathchkloc(userdir_phys, script_phys);
    switch (ret) {
        case OK:
            break;
        case ERR_BASEDIR:
            error("script %s: not in %s's user directory.",
                  script_log, logname);
        default:
            /* Should be unreachable. */
            error("%d: pathchkloc returned %d.", __LINE__, ret);
    }


    /*
     * Check whether the CGI script can only be modified by the user.
     */

    const char *basedir;

    if (*USER_DIR == '/') {
        basedir = userdir_phys;
    } else {
        ret = pathreal(owner->pw_dir, &basedir);
        switch (ret) {
        case OK:
            break;
        case ERR_LEN:
            error("user %s: home directory is too long.", logname);
        case ERR_SYS:
            error("realpath %s: %m.", owner->pw_dir);
        default:
            /* Should be unreachable. */
            error("%d: pathreal returned %d.", __LINE__, ret);
        }
    }

    ret = pathchkperm(uid, basedir, script_phys);
    switch (ret) {
    case OK:
        break;
    case ERR_PERM:
        error("script %s: writable by non-owner.", script_log);
    case ERR_SYS:
        /* Only reachable if the script was deleted after realpath. */
        error("stat %s: %m.", script_log);
    default:
        /* Should be unreachable. */
        error("%d: pathchkperm returned %d.", __LINE__, ret);
    }


    /*
     * It would be odd for the set-user-ID or the set-group-ID on execute
     * bits to be set for a script that is owned by a regular user. So if
     * one of them is set, this probably indicates a configuration error.
     */

    if ((script_stat.st_mode & S_ISUID) != 0) {
        error("script %s: set-user-ID on execute bit is set.", script_log);
    }

    if ((script_stat.st_mode & S_ISGID) != 0) {
        error("script %s: set-group-ID on execute bit is set.", script_log);
    }


    /*
     * There should be no need to run hidden files or files that reside
     * in hidden directories. So if the path to the script does refer to
     * a hidden file, this probably indicates a configuration error.
     */

    /* script_phys is canonical. */
    if (strstr(script_phys, "/.") != NULL) {
        error("path %s: contains hidden files.", script_log);
    }


    /*
     * Set up a safer environment.
     */

    errno = 0;
    if (setenv("DOCUMENT_ROOT", userdir_phys, true) != 0) {
        error("setenv: %m.");
    }

    errno = 0;
    if (setenv("HOME", owner->pw_dir, true) != 0) {
        error("setenv: %m.");
    }

    errno = 0;
    if (setenv("PATH", PATH, true) != 0) {
        error("setenv: %m.");
    }

    errno = 0;
    if (setenv("PATH_TRANSLATED", script_phys, true) != 0) {
        error("setenv: %m.");
    }

    errno = 0;
    if (setenv("SCRIPT_FILENAME", script_phys, true) != 0) {
        error("setenv: %m.");
    }

    errno = 0;
    if (setenv("USER_NAME", logname, true) != 0) {
        error("setenv: %m.");
    }

    errno = 0;
    if (chdir(userdir_phys) != 0) {
        error("chdir %s: %m.", userdir_phys);
    }

    /* RATS: ignore; the permission mask is set by the administrator. */
    umask(UMASK);


    /*
     * Run the script.
     */

    const Pair handlers[] = HANDLERS;
    const char *handler;

    ret = handlerfind(NELEMS(handlers), handlers, script_phys, &handler);
    switch (ret) {
    case OK:
        assert(handler != NULL);
        assert(*handler != '\0');
        assert(strnlen(handler, MAX_FNAME_LEN) < (size_t) MAX_FNAME_LEN);

        errno = 0;
        /* RATS: ignore; suCGI's whole point is to do this safely. */
        (void) execlp(handler, handler, script_phys, NULL);

        /* If this point is reached, execution has failed. */
        error("execlp %s %s: %m.", handler, script_phys);
    case ERR_SEARCH:
        break;
    case ERR_SUFFIX:
        break;
    case ERR_BAD:
        error("script %s: bad handler.", script_log);
    case ERR_LEN:
        error("script %s: filename suffix too long.", script_log);
    default:
        /* Should be unreachable. */
        error("%d: handlerfind returned %d.", __LINE__, ret);
    }

    errno = 0;
    /* RATS: ignore; suCGI's whole point is to do this safely. */
    (void) execl(script_phys, script_phys, NULL);

    /* If this point is reached, execution has failed. */
    error("execl %s: %m.", script_phys);
}
