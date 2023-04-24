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
#include <inttypes.h>
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
#include "env.h"
#include "error.h"
#include "handler.h"
#include "max.h"
#include "macros.h"
#include "params.h"
#include "path.h"
#include "priv.h"
#include "str.h"
#include "testing.h"
#include "types.h"
#include "userdir.h"


/*
 * Constants
 */

/* suCGI version. */
#define VERSION "0"


/*
 * Module variables
 */

/* Environment variables to keep. */
/* cppcheck-suppress [misra-c2012-9.2, misra-c2012-9.3];
   array of strings, double braces would be wrong, false positive.
   NOLINTNEXTLINE(bugprone-suspicious-missing-comma); literals intended. */
static const char *const allowedvars[] = ENV_PATTERNS;

/* Script handlers. */
static const Pair handlers[] = HANDLERS;


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
    (void) printf("#\n# Configuration\n#\n\n");

    (void) printf("USER_DIR=\"%s\"\n", USER_DIR);

    (void) printf("MIN_UID=%d\n", MIN_UID);
    (void) printf("MAX_UID=%d\n", MAX_UID);
    (void) printf("MIN_GID=%d\n", MIN_GID);
    (void) printf("MAX_GID=%d\n", MAX_GID);

    (void) printf("ENV_PATTERNS=\"\n");
    for (size_t i = 0; i < NELEMS(allowedvars); ++i) {
        (void) printf("\t%s\n", allowedvars[i]);
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

    (void) printf("SYSLOG_FACILITY=%d\n", SYSLOG_FACILITY);
    (void) printf("SYSLOG_MASK=%d\n", SYSLOG_MASK);
    (void) printf("SYSLOG_OPTS=%d\n", SYSLOG_OPTS);

    (void) printf("PATH=\"%s\"\n", PATH);
    (void) printf("UMASK=0%o\n", (unsigned) UMASK);

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

    (void) printf("\n\n#\n# System\n#\n\n");

#if defined(__GLIBC__)
    (void) printf("LIBC=glibc\n");
#elif defined(__GNU_LIBRARY__)
    (void) printf("LIBC=glibc\n");
#elif defined(__KLIBC__)
    (void) printf("LIBC=klibc\n");
#elif defined(__UCLIBC__)
    (void) printf("LIBC=uClibc\n");
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

    /*
     * setreuid and setregid accept -1 as ID. So -1 is a valid, if weird, ID.
     * But POSIX.1-2008 allows for uid_t, gid_t, and id_t to be defined as
     * unsigned integers, and that is how they are defined on most systems.
     * So IDs must be compared against -1 even if uid_t, gid_t, and id_t
     * are unsigned; in other words, the sign change is intentional.
     */
    ERRORIF((uint64_t) MAX_UID > (uint64_t) (SIGNEDMAX(uid_t) - 1U));
    ERRORIF((uint64_t) MAX_GID > (uint64_t) (SIGNEDMAX(gid_t) - 1U));
    ERRORIF((uint64_t) MAX_GID > (uint64_t) (SIGNEDMAX(GRP_T) - 1U));


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

    openlog("sucgi", SYSLOG_OPTS, SYSLOG_FACILITY);
    errno = 0;
    if (atexit(closelog) != 0) {
        error("atexit: %m");
    }

    (void) setlogmask(SYSLOG_MASK);


    /*
     * Drop privileges temporarily.
     */

    ret = privsuspend();
    switch (ret) {
    case OK:
        break;
    default:
        /* Should be unreachable. */
        error("%s:%d: privsuspend() -> %d [!]",
              __FILE__, __LINE__, ret);
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
        /* cppcheck-suppress misra-c2012-21.8;
           return segfaults when compiled with gcc and --coverage. */
        exit(EXIT_SUCCESS);
    default:
        usage();
    }


    /*
     * Restore the environment variables used by CGI scripts.
     */

    regex_t pregs[NELEMS(allowedvars)];

    for (size_t i = 0; i < NELEMS(allowedvars); ++i) {
        int err;

        err = regcomp(&pregs[i], allowedvars[i], REG_EXTENDED | REG_NOSUB);
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
        error("%s:%d: envrestore(%p, %zu, %p) -> %d [!]",
              __FILE__, __LINE__, vars, NELEMS(pregs), pregs, ret);
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
        error("%s:%d: envcopyvar(PATH_TRANSLATED, -> %p) -> %d [!]",
              __FILE__, __LINE__, script_log, ret);
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
        error("%s:%d: pathreal(%s, -> %s) -> %d [!]",
              __FILE__, __LINE__, script_log, script_phys, ret);
    }

    assert(script_phys != NULL);
    assert(*script_phys != '\0');
    assert(strnlen(script_phys, MAX_FNAME_LEN) < (size_t) MAX_FNAME_LEN);

    if (stat(script_phys, &script_stat) != 0) {
        /* Only reachable if the script was deleted after pathreal. */
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
    long ngroups_max;
    int ngroups;

    ngroups_max = sysconf(_SC_NGROUPS_MAX);
    if (ngroups_max < 0L || (uint64_t) ngroups_max > (uint64_t) MAX_NGROUPS) {
        ngroups_max = (long) MAX_NGROUPS;
    }

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
     * (2) MAX_GID > max-signed-value-that-would-fit(gid_t/GRP_T) - 1
     *     (so values cannot overflow).
     */

    ngroups = MAX_NGROUPS;
    (void) getgrouplist(logname, (GRP_T) gid, (GRP_T *) groups, &ngroups);

    if (ngroups < 0) {
        /* Should be unreachable. */
        if (ISSIGNED(gid_t)) {
            error("%s:%d: getgrouplist(%s, " PRId64 ", -> %p, -> %d [!])",
                  __FILE__, __LINE__, logname, (int64_t) gid, groups, ngroups);
        } else {
            error("%s:%d: getgrouplist(%s, " PRIu64 ", -> %p, -> %d [!])",
                  __FILE__, __LINE__, logname, (uint64_t) gid, groups, ngroups);
        }
    }

    if (ngroups_max < ngroups) {
        /* RATS: ignore; message is short and a literal. */
        syslog(LOG_NOTICE, "user %s: can only join %ld of %d groups.",
               logname, ngroups_max, ngroups);

        /* ngroups_max must be <= INT_MAX if this point is reached. */
        ngroups = (int) ngroups_max;
    }

    for (int i = 0; i < ngroups; ++i) {
        if (groups[i] < MIN_GID || groups[i] > MAX_GID) {
            const struct group *grp;

            /* cppcheck-suppress getgrgidCalled;
               suCGI need not be async-safe. */
            grp = getgrgid(groups[i]);
            if (grp != NULL) {
                error("user %s: member of %s.",
                      logname, grp->gr_name);
            } else if (ISSIGNED(gid_t)) {
                error("user %s: member of group " PRId64 ".",
                      logname, (int64_t) groups[i]);
            } else {
                error("user %s: member of group " PRIu64 ".",
                      logname, (uint64_t) groups[i]);
            }
        }
    }


    /*
     * Drop privileges for good.
     */

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
        if (ISSIGNED(id_t)) {
            error("%s:%d: privdrop(" PRId64 ", " PRId64 ", %d, %p) -> %d [!]",
                  __FILE__, __LINE__, (int64_t) uid, (int64_t) gid,
                  ngroups, groups, ret);
        } else {
            error("%s:%d: privdrop(" PRIu64 ", " PRIu64 ", %d, %p) -> %d [!]",
                  __FILE__, __LINE__, (uint64_t) uid, (uint64_t) gid,
                  ngroups, groups, ret);
        }
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
        error("%s:%d: userdirexp(%s, %s -> %p) -> %d [!]",
              __FILE__, __LINE__, USER_DIR, owner->pw_name, userdir_log, ret);
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
        error("%s:%d: pathreal(%s, -> %p) -> %d [!]",
              __FILE__, __LINE__, userdir_log, userdir_phys, ret);
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
            error("%s:%d: pathchkloc(%s, %s) -> %d [!]",
                  __FILE__, __LINE__, userdir_phys, script_phys, ret);
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
        error("execlp %s %s: %m.", handler, script_log);
    case ERR_SEARCH:
        ; /* Empty on purpose. */
        __attribute__((fallthrough));
    case ERR_SUFFIX:
        break;
    case ERR_BAD:
        error("script %s: bad handler.", script_log);
    case ERR_LEN:
        error("script %s: filename suffix too long.", script_log);
    default:
        /* Should be unreachable. */
        error("%s:%d: handlerfind(%zu, %p, %s, -> %p) -> %d [!]",
              __FILE__, __LINE__, NELEMS(handlers), handlers,
              script_phys, handler, ret);
    }

    errno = 0;
    /* RATS: ignore; suCGI's whole point is to do this safely. */
    (void) execl(script_phys, script_phys, NULL);

    /* If this point is reached, execution has failed. */
    error("execl %s: %m.", script_log);
}
