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

#if !defined(_FORTIFY_SOURCE)
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

#include "build.h"
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
#include "types.h"
#include "userdir.h"


/*
 * Configuration for test builds
 */

#if defined(TESTING) && TESTING

#undef USER_DIR
#define USER_DIR "/tmp/check-sucgi/%s"

#undef MIN_UID
#define MIN_UID 500

#undef MAX_UID
#define MAX_UID 30000

#undef MIN_GID
#define MIN_GID 1

#undef MAX_GID
#define MAX_GID 30000

#undef ALLOW_GROUP
#define ALLOW_GROUP ""

#undef DENY_GROUPS
#define DENY_GROUPS {""}

#undef HANDLERS
#define HANDLERS {{".sh", "sh"}}

#undef LOGGING_LEVEL
#define LOGGING_LEVEL (                             \
    LOG_MASK(LOG_EMERG)   | LOG_MASK(LOG_ALERT)  |  \
    LOG_MASK(LOG_CRIT)    | LOG_MASK(LOG_ERR)    |  \
    LOG_MASK(LOG_WARNING) | LOG_MASK(LOG_NOTICE) |  \
    LOG_MASK(LOG_INFO)    | LOG_MASK(LOG_DEBUG)     \
)

#undef LOGGING_OPTIONS
#if defined(LOG_PERROR)
#define LOGGING_OPTIONS ( LOG_CONS | LOG_PERROR )
#else
#define LOGGING_OPTIONS ( LOG_CONS )
#endif

#endif /* defined(TESTING) && TESTING */


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

/* Print usage information to stderr. */
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

#if defined(NDEBUG)
    (void) printf("NDEBUG=on\n");
#endif
#if defined(TESTING) && TESTING
    (void) printf("TESTING=%d\n", TESTING);
#endif

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
    (void) printf("LOGGING_LEVEL=%d\n", LOGGING_LEVEL);
    (void) printf("LOGGING_OPTIONS=%d\n", LOGGING_OPTIONS);

    (void) printf("PATH=\"%s\"\n", PATH);
    (void) printf("UMASK=0%o\n", (unsigned int) UMASK);

    (void) printf("MAX_STR_LEN=%u\n", MAX_STR_LEN);
    (void) printf("MAX_ERRMSG_LEN=%u\n", MAX_ERRMSG_LEN);
    (void) printf("MAX_FNAME_LEN=%d\n", MAX_FNAME_LEN);
    (void) printf("MAX_GRPNAME_LEN=%lld\n", (long long) MAX_GRPNAME_LEN);
    (void) printf("MAX_SUFFIX_LEN=%u\n", MAX_SUFFIX_LEN);
    (void) printf("MAX_VAR_LEN=%d\n", MAX_VAR_LEN);
    (void) printf("MAX_VARNAME_LEN=%u\n", MAX_VARNAME_LEN);
    (void) printf("MAX_NGROUPS=%u\n", MAX_NGROUPS);
    (void) printf("MAX_NVARS=%u\n", MAX_NVARS);

#if defined(CC)
    (void) printf("CC=\"%s\"\n", CC);
#endif
#if defined(CFLAGS)
    (void) printf("CFLAGS=\"%s\"\n", CFLAGS);
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
    (void) fputs("usage: sucgi [-c|-V|-h]\n", stderr);
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

    ERRORIF(sizeof(GETGRPLST_T) != sizeof(gid_t));
    ERRORIF(SIGNEDMAX(SETGRPNUM_T) < SIGNEDMAX(int));


    /*
     * Check whether configuration values are within bounds.
     */

    ERRORIF(sizeof(USER_DIR) <= 1U);
    ERRORIF(sizeof(USER_DIR) >= (size_t) MAX_FNAME_LEN);
    ERRORIF(sizeof(PATH) >= (size_t) MAX_FNAME_LEN);
    ERRORIF(sizeof(ALLOW_GROUP) < 1U);
    ERRORIF(sizeof(ALLOW_GROUP) >= MAX_GRPNAME_LEN);
    ERRORIF((uint64_t) MAX_UID > SIGNEDMAX(uid_t));
    /* NOLINTNEXTLINE(bugprone-branch-clone); GETGRPLST_T may be gid_t. */
    ERRORIF((uint64_t) MAX_GID > MIN(SIGNEDMAX(gid_t),
                                     SIGNEDMAX(GETGRPLST_T)));


    /*
     * Words of wisdom from the authors of suEXEC:
     *
     * > While cleaning the environment, the environment should be clean.
     * > (E.g. malloc() may get the name of a file for writing debugging
     * > info. Bad news if MALLOC_DEBUG_FILE is set to /etc/passwd.)
     */

    char *const *vars;      /* Backup of the environment. */
    char *null;             /* Pointer to NULL. */

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

    (void) setlogmask(LOGGING_LEVEL);


    /*
     * Drop privileges temporarily.
     */

    ret = priv_suspend();
    switch (ret) {
    case OK:
        break;
    default:
        /* Should be unreachable. */
        error("%d: priv_suspend returned %u.", __LINE__, ret);
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

    /* NOLINTBEGIN(bugprone-not-null-terminated-result);
       strncmp(argv[1], <option>, 3) is fine. */
    switch (argc) {
    case 1:
        break;
    case 2:
        /* Some getopt implementations are insecure. */
        if        (strncmp(argv[1], "-h", 3) == 0) {
            help();
        } else if (strncmp(argv[1], "-C", 3) == 0) {
            config();
        } else if (strncmp(argv[1], "-V", 3) == 0) {
            version();
        } else {
            /* cppcheck-suppress [misra-c2012-15.1, misra-c2012-15.3];
               least bad solution. */
            goto usage;
        }
        return EXIT_SUCCESS;
    default:
        usage:
            usage();
            return EXIT_FAILURE;
    }
    /* NOLINTEND(bugprone-not-null-terminated-result) */


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
            size_t errmsglen;

            errmsglen = regerror(err, &pregs[i], errmsg, sizeof(errmsg));
            if (errmsglen > sizeof(errmsg)) {
                /* RATS: ignore; message is short and a literal. */
                syslog(LOG_NOTICE, "regular expression error truncated.");
            }

            error("regcomp: %s", errmsg);
        }
    }

    ret = env_restore(vars, NELEMS(pregs), pregs);
    switch (ret) {
    case OK:
        break;
    case ERR_LEN:
        error("too many environment variables.");
    case ERR_SYS_SETENV:
        error("setenv: %m.");
    default:
        /* Should be unreachable. */
        error("%d: env_restore returned %u.", __LINE__, ret);
    }


    /*
     * Get the script's filename and filesystem metadata.
     */

    char script_log[MAX_FNAME_LEN];
    const char *script_phys;
    const char *script_env;
    struct stat script_stat;

    errno = 0;
    /* cppcheck-suppress misra-c2012-21.8; PATH_TRANSLATED is sanitised. */
    script_env = getenv("PATH_TRANSLATED");     /* RATS: ignore */
    if (script_env == NULL) {
        /* cppcheck-suppress misra-c2012-22.10; getenv may set errno. */
        if (errno == 0) {
            error("$PATH_TRANSLATED not set.");
        } else {
            /* Should be unreachable. */
            error("getenv: %m.");
        }
    }

    if (*script_env == '\0') {
        error("$PATH_TRANSLATED is empty.");
    }

    ret = str_cp(MAX_FNAME_LEN, script_env, script_log);
    switch (ret) {
    case OK:
        break;
    case ERR_LEN:
        error("$PATH_TRANSLATED is too long.");
    default:
        /* Should be unreachable. */
        error("%d: str_cp returned %u.", __LINE__, ret);
    }

    errno = 0;
    /* RATS: ignore; script_log is <= PATH_MAX, script_phys is checked. */
    script_phys = realpath(script_log, NULL);
    if (script_phys == NULL) {
        error("realpath %s: %m.", script_log);
    }

    if (strnlen(script_phys, MAX_FNAME_LEN) >= (size_t) MAX_FNAME_LEN) {
        error("%s's canonical path is too long.", script_log);
    }

    if (stat(script_phys, &script_stat) != 0) {
        error("stat %s: %m.", script_log);
    }

    if ((script_stat.st_mode & S_IFREG) == 0) {
        error("%s is not a regular file.", script_log);
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
            error("%s has no owner.", script_log);
        } else {
            error("getpwuid: %m.");
        }
    }

    assert(owner->pw_uid == script_stat.st_uid);

    if (owner->pw_uid < MIN_UID || owner->pw_uid > MAX_UID) {
        error("%s is owned by privileged user %s.",
              script_log, owner->pw_name);
    }

    logname = owner->pw_name;
    uid = owner->pw_uid;
    gid = owner->pw_gid;


    /*
     * Check if the owner is a member of a privileged group.
     */

    const char *const deny_groups[] = DENY_GROUPS;
    gid_t groups[MAX_NGROUPS];
    int ngroups;
    bool allowed;

    ngroups = MAX_NGROUPS;
    allowed = strncmp(ALLOW_GROUP, "", 1) == 0;

    /*
     * getgrouplist may take and return group IDs as int's or as gid_t's.
     * GETGRPLST_T refers to the respective type. A compile-time error is
     * raised if int and GETGRPLST_T are of different sizes, if MIN_GID is
     * outside the range 1 to MAX_GID, or if MAX_GID is outside the range
     * MIN_GID to INT_MAX. A run-time error is raised if a group ID is
     * outside the range MIN_GID to MAX_GID. So coercing group IDs to
     * GETGRPLST_T is safe.
     */
    if (getgrouplist(logname, (GETGRPLST_T) gid,
                     /* cppcheck-suppress misra-c2012-11.3; see above. */
                     (GETGRPLST_T *) groups, &ngroups) < 0)
    {
        error("user %s belongs to too many groups.", logname);
    }

    for (int i = 0; i < ngroups; ++i) {
        struct group *grp;

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
                    error("user %s belongs to non-existing group %lld.",
                          logname, (long long) groups[i]);
                } else {
                    error("user %s belongs to non-existing group %llu.",
                          logname, (unsigned long long) groups[i]);
                }
            } else {
                error("getgrgid: %m.");
            }
        }

        if (grp->gr_gid < MIN_GID || grp->gr_gid > MAX_GID) {
            error("user %s belongs to privileged group %s.",
                  logname, grp->gr_name);
        }

        for (size_t j = 0; j < NELEMS(deny_groups); ++j) {
            if (fnmatch(deny_groups[j], grp->gr_name, 0) == 0) {
                error("user %s belongs to denied group %s.",
                      logname, grp->gr_name);
            }
        }

        if (!allowed &&
            strncmp(grp->gr_name, ALLOW_GROUP, MAX_GRPNAME_LEN) == 0)
        {
            allowed = true;
        }
    }

    if (!allowed) {
        error("user %s does not belong to group %s.", logname, ALLOW_GROUP);
    }


    /*
     * Drop privileges for good.
     */

    long ngroups_max;           /* Maximum number of groups per process. */

    ngroups_max = sysconf(_SC_NGROUPS_MAX);
    if (-1L < ngroups_max && ngroups_max < ngroups) {
        /* RATS: ignore; message is short and a literal. */
	syslog(LOG_NOTICE, "can only set %ld of %d groups for user %s.",
               ngroups_max, ngroups, logname);

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
     * setgroups may take the number of groups as size_t or int.
     * SETGRPNUM_T refers to the respective type. A compile-time error
     * is raised if SETGRPNUM_T is too small to hold INT_MAX.
     * So coercing ngroups to SETGRPNUM_T is safe.
     */
    ret = priv_drop(uid, gid, (SETGRPNUM_T) ngroups, groups);
    switch (ret) {
    case OK:
        break;
    case ERR_SYS_SETGROUPS:
        error("setgroups: %m.");
    default:
        /* Should be unreachable. */
        error("%d: priv_drop returned %u.", __LINE__, ret);
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

    /* RATS: ignore; userdir_resolve repects MAX_FNAME_LEN. */
    char userdir_log[MAX_FNAME_LEN];
    char *userdir_phys;

    ret = userdir_resolve(USER_DIR, owner, userdir_log);

    switch (ret) {
    case OK:
        break;
    case ERR_LEN:
        error("%s's user directory is too long.", logname);
    case ERR_SYS_SNPRINTF:
        error("snprintf: %m.");
    default:
        /* Should be unreachable. */
        error("%d: userdir_resolve returned %u.", __LINE__, ret);
    }

    assert(userdir_log);
    assert(*userdir_log != '\0');
    assert(strnlen(userdir_log, MAX_FNAME_LEN) < (size_t) MAX_FNAME_LEN);

    errno = 0;
    /* RATS: ignore; userdir_log is <= PATH_MAX, userdir_phys is checked. */
    userdir_phys = realpath(userdir_log, NULL);
    if (userdir_phys == NULL) {
        error("realpath %s: %m.", userdir_log);
    }

    ret = path_check_in(userdir_phys, script_phys);
    switch (ret) {
        case OK:
            break;
        case ERR_BASEDIR:
            error("%s is not within %s's user directory.",
                  script_log, logname);
        default:
            /* Should be unreachable. */
            error("%d: path_check_in returned %u.", __LINE__, ret);
    }


    /*
     * Check whether the CGI script can only be modified by the user.
     */

    const char *base_dir;

    if (*USER_DIR == '/') {
        base_dir = userdir_phys;
    } else {
        if (strnlen(owner->pw_dir, MAX_FNAME_LEN) >= (size_t) MAX_FNAME_LEN) {
            /*
             * Should only be reachable if a system administrator set the
             * owner's home directory to a path that is longer than PATH_MAX.
             */
            error("%s's home directory is too long", logname);
        }

        errno = 0;
        /* RATS: ignore; owner->pw_dir is <= PATH_MAX, basedir is checked. */
        base_dir = realpath(owner->pw_dir, NULL);
        if (base_dir == NULL) {
            error("realpath %s: %m.", owner->pw_dir);
        }

        if (strnlen(base_dir, MAX_FNAME_LEN) >= (size_t) MAX_FNAME_LEN) {
            /*
             * Should only be reachable if a system administrator set the
             * owner's home directory to a symlink to a path that is longer
             * than PATH_MAX.
             */
            error("%s's canonical path is too long.", owner->pw_dir);
        }
    }

    ret = path_check_wexcl(uid, base_dir, script_phys);
    switch (ret) {
    case OK:
        break;
    case ERR_WEXCL:
        error("%s is writable by users other than %s.", script_log, logname);
    case ERR_SYS_STAT:
        error("stat %s: %m.", script_log);
    default:
        /* Should be unreachable. */
        error("%d: path_check_wexcl returned %u.", __LINE__, ret);
    }


    /*
     * It would be odd for the set-user-ID or the set-group-ID on execute
     * bits to be set for a script that is owned by a regular user. So if
     * one of them is set, this probably indicates a configuration error.
     */

    if ((script_stat.st_mode & S_ISUID) != 0) {
        error("%s's set-user-ID bit is set.", script_phys);
    }

    if ((script_stat.st_mode & S_ISGID) != 0) {
        error("%s's set-group-ID bit is set.", script_phys);
    }


    /*
     * There should be no need to run hidden files or files that reside
     * in hidden directories. So if the path to the script does refer to
     * a hidden file, this probably indicates a configuration error.
     */

    /* script_phys is canonical. */
    if (strstr(script_phys, "/.") != NULL) {
        error("path %s contains hidden files.", script_log);
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

    if (!file_is_exe(uid, gid, script_stat)) {
        const Pair handlers[] = HANDLERS;
        const char *handler;

        ret = handler_lookup(NELEMS(handlers), handlers,
                             script_phys, &handler);

        switch (ret) {
        case OK:
            break;
        case ERR_BAD:
            error("script %s: bad handler.", script_log);
        case ERR_LEN:
            error("script %s: filename suffix too long.", script_log);
        case ERR_SEARCH:
            error("script %s: no handler found.", script_log);
        default:
            /* Should be unreachable. */
            error("%d: handler_lookup returned %u.", __LINE__, ret);
        }

        assert(handler);
        assert(*handler != '\0');
        assert(strnlen(handler, MAX_FNAME_LEN) < (size_t) MAX_FNAME_LEN);

        errno = 0;
        /* RATS: ignore; suCGI's whole point is to do this safely. */
        (void) execlp(handler, handler, script_phys, NULL);

        /* If this point is reached, execution has failed. */
        error("execlp %s %s: %m.", handler, script_phys);
    }

    errno = 0;
    /* RATS: ignore; suCGI's whole point is to do this safely. */
    (void) execl(script_phys, script_phys, NULL);

    /* If this point is reached, execution has failed. */
    error("execl %s: %m.", script_phys);
}
