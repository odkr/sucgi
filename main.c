/*
 * Run CGI scripts with the permissions of their owner.
 *
 * Copyright 2022 and 2023 Odin Kroeger.
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
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "attr.h"
#include "compat.h"
#include "env.h"
#include "error.h"
#include "handler.h"
#include "macros.h"
#include "params.h"
#include "path.h"
#include "priv.h"
#include "str.h"
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

/* Regular expressions that define which environment variables are kept. */
static const char *const safe_var_patterns[] = /* cppcheck-suppress misra-c2012-9.2 */
    SAFE_ENV_VARS;

/* Filename suffix-script interpreter pairs. */
static const Pair handlers[] = HANDLERS;


/*
 * Prototypes
 */

/* Print help and exit with EXIT_SUCCESS. */
_noreturn
static void help(void);

/* Print build configuration and exit with EXIT_SUCCESS. */
_noreturn
static void config(void);

/* Print version and exit with EXIT_SUCCESS. */
_noreturn
static void version(void);

/* Print usage information to stderr and exit with EXIT_FAILURE. */
_noreturn
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
"    -C  Print the build configuration.\n"
"    -V  Print version and license information.\n"
"    -h  Print this help screen."
    );

    /* cppcheck-suppress misra-c2012-21.8; least bad option. */
    exit(EXIT_SUCCESS);
}

static void
config(void)
{
    (void) printf("# Configuration\n");

    (void) printf("USER_DIR='%s'\n", USER_DIR);
    (void) printf("START_UID=%d\n",  START_UID);
    (void) printf("STOP_UID=%d\n",   STOP_UID);
    (void) printf("START_GID=%d\n",  START_GID);
    (void) printf("STOP_GID=%d\n",   STOP_GID);

    (void) printf("HANDLERS='");
    for (size_t i = 0; i < NELEMS(handlers); ++i) {
        /* cppcheck-suppress knownConditionTrueFalse; there could be more. */
        if (i > 0U) {
            (void) printf(",");
        }
        (void) printf("%s=%s", handlers[i].key, handlers[i].value);
    }
    (void) printf("'\n");

    (void) printf("SAFE_ENV_VARS='\n");
    for (size_t i = 0; i < NELEMS(safe_var_patterns); ++i) {
        (void) printf("\t%s\n", safe_var_patterns[i]);
    }
    (void) printf("'\n");

    (void) printf("SYSLOG_FACILITY=%d\n", SYSLOG_FACILITY);
    (void) printf("SYSLOG_MASK=%d\n",     SYSLOG_MASK);
    (void) printf("SYSLOG_OPTS=%d\n",     SYSLOG_OPTS);

    (void) printf("PATH='%s'\n", PATH);
    (void) printf("UMASK=0%o\n", (unsigned) UMASK);

    (void) printf("\n# Limits\n");

    (void) printf("INVALID_GID=%llu\n",
                  (unsigned long long) INVALID_GID);
    (void) printf("MAX_STR_LEN=%llu\n",
                  (unsigned long long) MAX_STR_LEN);
    (void) printf("MAX_ERRMSG_LEN=%llu\n",
                  (unsigned long long) MAX_ERRMSG_LEN);
    (void) printf("MAX_FNAME_LEN=%llu\n",
                  (unsigned long long) MAX_FNAME_LEN);
    (void) printf("MAX_SUFFIX_LEN=%llu\n",
                  (unsigned long long) MAX_SUFFIX_LEN);
    (void) printf("MAX_VAR_LEN=%llu\n",
                  (unsigned long long) MAX_VAR_LEN);
    (void) printf("MAX_VARNAME_LEN=%llu\n",
                  (unsigned long long) MAX_VARNAME_LEN);
    (void) printf("MAX_NGROUPS=%llu\n",
                  (unsigned long long) MAX_NGROUPS);
    (void) printf("MAX_NVARS=%llu\n",
                  (unsigned long long) MAX_NVARS);
    (void) printf("MAX_UID_VAL=%llu\n",
                  (unsigned long long) MAX_UID_VAL);
    (void) printf("MAX_GID_VAL=%llu\n",
                  (unsigned long long) MAX_GID_VAL);
    (void) printf("MAX_GRP_VAL=%llu\n",
                  (unsigned long long) MAX_GRP_VAL);
    (void) printf("MAX_NGRPS_VAL=%llu\n",
                  (unsigned long long) MAX_NGRPS_VAL);

    (void) printf("\n# System\n");

#if defined(LIBC)
    (void) printf("LIBC=%s\n", LIBC);
#else
    (void) printf("#LIBC=???\n");
#endif

    (void) printf("\n# Debugging\n");

#if defined(TESTING)
    (void) printf("TESTING=%d\n", TESTING);
#else
    (void) printf("TESTING=0\n");
#endif

#if defined(NATTR)
    (void) printf("NATTR=%d\n", NATTR);
#else
    (void) printf("NATTR=0\n");
#endif

#if defined(NDEBUG)
    (void) printf("NDEBUG=%d\n", NDEBUG);
#else
    (void) printf("NDEBUG=0\n");
#endif

    /* cppcheck-suppress misra-c2012-21.8; least bad option. */
    exit(EXIT_SUCCESS);
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

    /* cppcheck-suppress misra-c2012-21.8; least bad option. */
    exit(EXIT_SUCCESS);
}

static void
usage(void)
{
    (void) fputs("usage: sucgi [-C|-V|-h]\n", stderr);

    /* cppcheck-suppress misra-c2012-21.8; least bad option. */
    exit(EXIT_FAILURE);
}


/*
 * Main
 */

int
main(int argc, char **argv) {
    Error ret;


    /*
     * Check build configuration.
     */

    /*
     * The maximum values below should have been determined by ./configure.
     * But if that failed, they were approximated at compile-time; and that
     * approximation cannot take padding bits into account, so it needs to
     * be checked whether each maximum value can, in fact, be represented
     * by the type that it's supposed to be the maximum value of.
     */

    /* cppcheck-suppress [misra-c2012-10.4, misra-c2012-10.8] */
    ASSERT((uid_t) MAX_UID_VAL == MAX_UID_VAL);
    /* cppcheck-suppress [misra-c2012-10.4, misra-c2012-10.8] */
    ASSERT((gid_t) MAX_GID_VAL == MAX_GID_VAL);
    ASSERT((GRP_T) MAX_GRP_VAL == MAX_GRP_VAL);
    ASSERT((NGRPS_T) MAX_NGRPS_VAL == MAX_NGRPS_VAL);

    /* System-dependent types. */
    ASSERT(sizeof(GRP_T) == sizeof(gid_t));
    ASSERT(MAX_NGRPS_VAL >= INT_MAX);

    /* NOLINTBEGIN(bugprone-sizeof-expression); yes, I am that paranoid. */

    /* Are configuration values are within bounds? */
    ASSERT(sizeof(USER_DIR) > 1U);
    ASSERT(sizeof(USER_DIR) <= (size_t) MAX_FNAME_LEN);
    ASSERT(sizeof(PATH) > 0U);
    ASSERT(sizeof(PATH) <= (size_t) MAX_FNAME_LEN);

    /* NOLINTEND(bugprone-sizeof-expression). */

    /*
     * setreuid and setregid accept -1 as ID. So -1 is a valid, if weird, ID.
     * However, POSIX.1-2008 allows for uid_t, gid_t, and id_t to be defined
     * as unsigned integers. Moreover, this is how they are defined on most
     * systems; on such systems, -1 just wraps around to the maximum value
     * that ID types can represent. So that value is *not* a valid ID.
     */

    ASSERT((uintmax_t) STOP_UID <= ((uintmax_t) MAX_UID_VAL - (uintmax_t) 1));
    ASSERT((uintmax_t) STOP_GID <= ((uintmax_t) MAX_GID_VAL - (uintmax_t) 1));
    ASSERT((uintmax_t) STOP_GID <= ((uintmax_t) MAX_GRP_VAL - (uintmax_t) 1));

    /* Make sure the start and stop UIDs and GIDs can be represented. */
    ASSERT((uid_t) START_UID == START_UID);
    ASSERT((uid_t) STOP_UID == STOP_UID);
    ASSERT((gid_t) START_GID == START_GID);
    ASSERT((gid_t) STOP_GID == STOP_GID);
    ASSERT((GRP_T) START_GID == START_GID);
    ASSERT((GRP_T) STOP_GID == STOP_GID);

    /* Make sure the number of groups can be represented. */
    ASSERT((NGRPS_T) INT_MAX == INT_MAX);


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

    ret = priv_suspend();
    if (ret != OK) {
        /* NOTREACHED */
        BUG("priv_suspend() -> %d [!]", ret);
    }


    /*
     * Options.
     */

    if (argc == 0 || argv == NULL || *argv == NULL || **argv == '\0') {
        error("empty argument vector.");
    }

    if (argc > 2) {
        usage();
    }

    /* Some versions of getopt are insecure. */
    for (int i = 1; i < argc; ++i) {
        char *arg = argv[i];

        if (arg == NULL) {
            /* NOTREACHED */
            error("argument no. %d is the null pointer.", i);
        }

        if (strnlen(arg, 3U) == 2U && arg[0] == '-') {
            switch (arg[1]) {
            case 'C':
                config();
            case 'V':
                version();
            case 'h':
                help();
            default:
                ; /* Empty on purpose. */
            }
        }

        usage();
    }


    /*
     * Restore environment.
     */

/* FIXME: Make nicer. */
#if defined(REQ_ENV_CS)
    char *envbuf[MAX_STR_LEN];

    if (confstr(REQ_ENV_CS, envbuf, sizeof(envbuf)) > sizeof(envbuf)) {
        error("minimal conforming environment is too large");
    }

    ret = env_init(envbuf);
    switch (ret) {
    case OK:
        break;
    case ERR_SYS:
        error("setenv: %m.");
    default:
        /* NOTREACHED */
        BUG("env_init(%s) -> %d [!]", envbuf, ret);
    }
#endif

    regex_t safe_var_pregs[NELEMS(safe_var_patterns)];

    for (size_t i = 0; i < NELEMS(safe_var_patterns); ++i) {
        int err;

        err = regcomp(&safe_var_pregs[i], safe_var_patterns[i],
                      REG_EXTENDED | REG_NOSUB);
        if (err != 0) {
            /* RATS: ignore; regerror respects the size of errmsg. */
            char errmsg[MAX_ERRMSG_LEN];

            /*
             * TODO: Document that regular expression error
             *       messages may get truncated.
             */
            (void) regerror(err, &safe_var_pregs[i], errmsg, sizeof(errmsg));
            error("regcomp: %s", errmsg);
        }
    }

    ret = env_restore((const char *const *) vars,
                      NELEMS(safe_var_pregs), safe_var_pregs);
    switch (ret) {
    case OK:
        break;
    case ERR_LEN:
        error("too many environment variables.");
    case ERR_SYS:
        error("setenv: %m.");
    default:
        /* NOTREACHED */
        BUG("env_restore(%p, %zu, %p) -> %d [!]",
            vars, NELEMS(safe_var_pregs), safe_var_pregs, ret);
    }


    /*
     * Get the script's filename and filesystem metadata.
     */

    /* RATS: ignore; env_copy_var respects the size of scriptname. */
    char scriptname[MAX_VAR_LEN];
    char *realscriptname;
    struct stat scriptstatus;
    size_t scriptnamelen;
    size_t realscriptnamelen;

    ret = env_copy_var("PATH_TRANSLATED", sizeof(scriptname) - 1U,
                       &scriptnamelen, scriptname);
    switch (ret) {
    case OK:
        break;
    case ERR_LEN:
        error("$PATH_TRANSLATED: too long.");
    case ERR_SEARCH:
        error("$PATH_TRANSLATED: not set.");
    default:
        /* NOTREACHED */
        BUG("env_copy_var(PATH_TRANSLATED, %zu, -> %zu, -> %s) -> %d [!]",
            sizeof(scriptname) - 1U, scriptnamelen, scriptname, ret);
    }

    if (*scriptname == '\0') {
        error("$PATH_TRANSLATED: empty.");
    }

    ret = path_real(scriptnamelen, scriptname,
                    &realscriptnamelen, &realscriptname);
    switch (ret) {
    case OK:
        break;
    case ERR_SYS:
        error("realpath %s: %m.", scriptname);
    default:
        /* NOTREACHED */
        BUG("path_real(%zu, %s, -> %zu, -> %s) -> %d [!]",
            scriptnamelen, scriptname,
            realscriptnamelen, realscriptname, ret);
    }

    if (stat(realscriptname, &scriptstatus) != 0) {
        error("stat %s: %m.", scriptname);
    }

    if ((scriptstatus.st_mode & S_IFREG) == 0) {
        error("script %s: not a regular file.", scriptname);
    }


    /*
     * Check if the script is owned by a non-system user.
     */

    const struct passwd *owner;
    const char *logname;
    uid_t uid;
    gid_t gid;

    errno = 0;
    owner = getpwuid(scriptstatus.st_uid);
    if (owner == NULL) {
        /* cppcheck-suppress misra-c2012-22.10; getpwuid sets errno. */
        if (errno == 0) {
            error("script %s: no owner.", scriptname);
        } else {
            error("getpwuid: %m.");
        }
    }

    assert(owner->pw_uid == scriptstatus.st_uid);

    if (owner->pw_uid < START_UID || owner->pw_uid > STOP_UID) {
        error("script %s: owned by a non-system user.", scriptname);
    }

    logname = owner->pw_name;
    uid = owner->pw_uid;
    gid = owner->pw_gid;


    /*
     * Check the owner's group memberships.
     */

    gid_t groups[MAX_NGROUPS];
    long maxngroups;
    int ngroups;

    for (size_t i = 0; i < NELEMS(groups); ++i) {
        groups[i] = (gid_t) INVALID_GID;
    }

    maxngroups = sysconf(_SC_NGROUPS_MAX);
    if (maxngroups < 0L || (uintmax_t) maxngroups > (uintmax_t) MAX_NGROUPS) {
        maxngroups = (long) MAX_NGROUPS;
    }

    /*
     * GRP_T names the data type that getgrouplist takes and returns GIDs as.
     * On older systems and macOS this is int, on modern systems gid_t.
     *
     * Casting gid_t to GRP_T is guaranteed to be safe because:
     * (1) a compile-time error is raised if sizeof(GRP_T) != sizeof(gid_t)
     *     (so GRP_T[i] and gid_t[i] cannot refer to different addresses).
     * (2) gid_t and GRP_T use the same integer representation for any value
     *     in [START_GID .. STOP_GID] (so type-casting cannot change values).
     * (3) a run-time error is raised if a GID falls outside that range.
     *
     * gid_t and GRP_T are guaranteed to use the same integer representation
     * for any value in that range because a compile-time error is raised if:
     * (1) START_GID < 1 (so values cannot change sign);
     * (2) STOP_GID > the highest value that both gid_t and GRP_T
     *     can represent - 1 (so values cannot overflow).
     */
    ngroups = MAX_NGROUPS;
    (void) getgrouplist(logname, (GRP_T) gid, (GRP_T *) groups, &ngroups);

    if (ngroups < 0) {
        /* NOTREACHED */
        if (ISSIGNED(gid_t)) {
            BUG("getgrouplist(%s, %lld, -> %p, -> %d [!])",
                logname, (long long) gid, groups, ngroups);
        } else {
            BUG("getgrouplist(%s, %llu, -> %p, -> %d [!])",
                logname, (unsigned long long) gid, groups, ngroups);
        }
    }

    if (maxngroups < ngroups) {
        /* maxngroups must be <= INT_MAX if this point is reached. */
        assert((uintmax_t) maxngroups <= (uintmax_t) INT_MAX);

        /* RATS: ignore; message is short and a literal. */
        syslog(LOG_NOTICE, "user %s: joining %ld out of %d groups only.",
               logname, maxngroups, ngroups);

        ngroups = (int) maxngroups;
    }

    for (int i = 0; i < ngroups; ++i) {
        if (groups[i] < START_GID || groups[i] > STOP_GID) {
            const struct group *grp;

            grp = getgrgid(groups[i]);
            if (grp != NULL) {
                error("user %s: member of system group %s.",
                      logname, grp->gr_name);
            } else if (ISSIGNED(gid_t)) {
                error("user %s: member of system group %lld.",
                      logname, (unsigned long long) groups[i]);
            } else {
                error("user %s: member of system group %llu.",
                      logname, (unsigned long long) groups[i]);
            }
        }
    }


    /*
     * Drop privileges for good.
     */

    errno = 0;
    if (seteuid(0) != 0) {
        error("seteuid: %m.");
    }

    /*
     * NGRPS_T names the data type of setgroups third argument, the number of
     * groups given. On GNU-like systems this is size_t, on others int.
     *
     * Casting ngroups to NGRPS_T is guaranteed to be safe because:
     * (1) ngroups cannot be negative (so values cannot change sign);
     * (2) ngroups is capped at MAX_NGROUPS and a compile-time error
     *     is raised if MAX_NGROUPS > INT_MAX or NGRPS_T cannot represent
     *     INT_MAX (so values cannot overflow).
     */
    ret = priv_drop(uid, gid, (NGRPS_T) ngroups, groups);
    switch (ret) {
    case OK:
        break;
    case ERR_SYS:
        error("could not drop privileges: %m.");
    default:
        /* NOTREACHED */
        if (ISSIGNED(id_t)) {
            BUG("priv_drop(%lld, %lld, %d, %p) -> %d [!]",
                (long long) uid, (long long) gid,
                (int) ngroups, groups, ret);
        } else {
            BUG("priv_drop(%llu, %llu, %d, %p) -> %d [!]",
                (unsigned long long) uid, (unsigned long long) gid,
                (int) ngroups, groups, ret);
        }
    }


    /*
     * Check whether the script is within the user directory.
     */

    /* RATS: ignore; userdir_exp respects the size of userdir. */
    char userdir[MAX_FNAME_LEN];
    char *realuserdir;
    size_t userdirlen;
    size_t realuserdirlen;

    ret = userdir_exp(USER_DIR, owner, sizeof(userdir), &userdirlen, userdir);
    switch (ret) {
    case OK:
        break;
    case ERR_LEN:
        error("user %s: user directory is too long.", logname);
    case ERR_SYS:
        error("snprintf: %m.");
    default:
        /* NOTREACHED */
        BUG("userdir_exp(%s, %s -> %zu, -> %p) -> %d [!]",
            USER_DIR, owner->pw_name, userdirlen, userdir, ret);
    }

    ret = path_real(userdirlen, userdir, &realuserdirlen, &realuserdir);
    switch (ret) {
    case OK:
        break;
    case ERR_LEN:
        error("user %s: user directory is too long.", logname);
    case ERR_SYS:
        error("realpath %s: %m.", userdir);
    default:
        /* NOTREACHED */
        BUG("path_real(%zu, %s, -> %zu, -> %s) -> %d [!]",
            userdirlen, userdir, realuserdirlen, realuserdir, ret);
    }

    if (!path_within(realscriptnamelen, realscriptname,
                     realuserdirlen, realuserdir))
    {
        error("script %s: not in %s's user directory.", scriptname, logname);
    }


    /*
     * It would be odd for the set-user-ID or the set-group-ID on execute
     * bits to be set for a script that is owned by a regular user. So if
     * one of them is set, this probably indicates a configuration error.
     */

    if ((scriptstatus.st_mode & S_ISUID) != 0) {
        error("script %s: set-user-ID on execute bit is set.", scriptname);
    }

    if ((scriptstatus.st_mode & S_ISGID) != 0) {
        error("script %s: set-group-ID on execute bit is set.", scriptname);
    }


    /*
     * There should be no need to run hidden files or files that reside
     * in hidden directories. So if the path to the script does refer to
     * a hidden file, this probably indicates a configuration error.
     */

    if (strstr(realscriptname, "/.") != NULL) {
        error("path %s: contains hidden files.", scriptname);
    }


    /*
     * Set up a safer environment.
     */

    errno = 0;
    if (setenv("DOCUMENT_ROOT", realuserdir, true) != 0) {
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
    if (setenv("PATH_TRANSLATED", realscriptname, true) != 0) {
        error("setenv: %m.");
    }

    errno = 0;
    if (setenv("SCRIPT_FILENAME", realscriptname, true) != 0) {
        error("setenv: %m.");
    }

    errno = 0;
    if (setenv("USER_NAME", logname, true) != 0) {
        error("setenv: %m.");
    }

    errno = 0;
    if (chdir(realuserdir) != 0) {
        error("chdir %s: %m.", realuserdir);
    }

    /* RATS: ignore; the permission mask is set by the administrator. */
    umask(UMASK);


    /*
     * Run the script.
     */

    const char *handler;

    ret = handler_find(NELEMS(handlers), handlers,
                       realscriptnamelen, realscriptname, &handler);
    switch (ret) {
    case OK:
        errno = 0;
        /* RATS: ignore; suCGI's whole point is to do this safely. */
        (void) execlp(handler, handler, realscriptname, NULL);

        error("execlp %s %s: %m.", handler, scriptname);
    case ERR_BAD:
        error("script %s: bad handler.", scriptname);
    case ERR_LEN:
        error("script %s: filename suffix too long.", scriptname);
    case ERR_SEARCH:
        ; /* Falls through. */
    case ERR_SUFFIX:
        break;
    default:
        /* NOTREACHED */
        BUG("handler_find(%zu, %p, %s, -> %p) -> %d [!]",
            NELEMS(handlers), handlers, realscriptname, handler, ret);
    }

    errno = 0;
    /* RATS: ignore; suCGI's whole point is to do this safely. */
    (void) execl(realscriptname, realscriptname, NULL);

    error("execl %s: %m.", scriptname);
}
