/*
 * suCGI configuration.
 */

/* This is config.h's include guard. Leave it alone. It's on duty. */
#if !defined(CONFIG_H)
#define CONFIG_H


/*
 * CGI scripts are only executed if they are inside this path.
 *
 * Should correspond to the UserDir directive of your Apache configuation
 * (or the equivalent directive of the webserver you use).
 *
 * For example:
 *     - UserDir public_html -> "/home"
 *     - UserDir /srv/web -> "/srv/web"
 */
#define JAIL_DIR "/home"

/*
 * The document root of user websites.
 *
 * "%1$s" or "%s" is replaced with the home directory of the script's owner.
 * "%2$s" is replaced with their login name.
 *
 * Should match the UserDir directive of your Apache configuration
 * (or the equivalent directive of the webserver you use).
 *
 * For example:
 *     - UserDir public_html -> "%s/public_html"
 *     - UserDir /srv/web -> "/srv/web/%2$s"
 */
#define USER_DIR "%s/public_html"

/*
 * Must the user directory reside in the user's home directory?
 * Boolean value.
 */
#define FORCE_HOME true

/*
 * Smallest UID that may be assigned to a regular user. Unsigned integer.
 * On most systems, this will be 500 (e.g., macOS) or 1,000 (e.g, Debian).
 */
#define MIN_UID 1000U

/*
 * Largest UID that may be assigned to a regular user. Unsigned integer.
 * On most systems, this will be 60,000 (though some use 32,767 for nobody).
 */
#define MAX_UID 30000U

/*
 * Smallest GID that may be assigned to a regular user. Unsigned integer.
 * On most systems, this will be 500 or 1,000 (e.g, Debian).
 * However, some systems (e.g., macOS) make no such distinction.
 */
#define MIN_GID 1000U

/*
 * Largest GID that may be assigned to a regular user. Unsigned integer.
 * On most systems, this will be 30,000 or 60,000 (Debian).
 */
#define MAX_GID 30000U

/*
 * Handlers to run CGI scripts with if their executable bit is unset.
 * Array of filename suffix-handler pairs.
 * 
 * The filename suffix must be given including the leading dot (e.g., ".php").
 * The handler is looked up in $PATH if its name is relative (e.g., "php");
 * keep in mind that $PATH is set to PATH (see below).
 *
 * The array must be terminated with a pair of NULLs.
 */
#define HANDLERS	{					\
	{".php", "php"},					\
	{NULL, NULL}	/* Array terminator. DO NOT REMOVE. */	\
}

/*
 * A secure $PATH.
 */
#define PATH "/usr/bin:/bin"

/* 
 * A secure file permission mask. The leading "0" is significant.
 * Added to the current umask.
 */
#define UMASK 077

/*
 * Maximum number of groups a user can be a member of. Signed integer.
 * suCGI rejects users who belong to more groups.
 */
#define MAX_GROUPS 32

/*
 * Maximum number of environment variables. Unsigned integer.
 * suCGI aborts if the environment contains more variables.
 * Setting this lower than 192 may break some CGI scripts.
 */
#define MAX_ENV 256U

/*
 * Maximum string length in bytes, including the terminating NUL. 
 * Unsigned integer. suCGI aborts if an environment variables is longer.
 * The length of an environment variable includes its name and the "=".
 * Must be at most as long as PATH_MAX.
 */
#define MAX_STR 1024U


/******************************************************
 * That's all folks. Do not venture beyond this line. *
 *****************************************************/

#include <limits.h>


/*
 * Test builds
 */

#if !defined(NDEBUG) && defined(TESTING) && TESTING

#undef JAIL_DIR
#define JAIL_DIR "/"

#undef USER_DIR 
#define USER_DIR "%s"

#undef FORCE_HOME
#define FORCE_HOME false

#undef MIN_UID
#define MIN_UID 500U

#undef MAX_UID
#define MAX_UID 30000U

#undef MIN_GID
#define MIN_GID 1U

#undef MAX_GID
#define MAX_GID 30000U

#undef HANDLERS
#define HANDLERS {{".sh", "sh"}, {NULL, NULL}}

#endif /* !defined(NDEBUG) && defined(TESTING) && TESTING */


/*
 * Verification
 */

#if !defined(JAIL_DIR)
#error JAIL_DIR is undefined.
#endif /* !defined(JAIL_DIR) */

#if !defined(USER_DIR)
#error USER_DIR is undefined.
#endif /* !defined(USER_DIR) */

#if !defined(FORCE_HOME)
#error FORCE_HOME is undefined.
#endif /* !defined(FORCE_HOME) */

#if !defined(MIN_UID)
#error MIN_UID is undefined.
#endif /* !defined(MIN_UID) */

#if !defined(MAX_UID)
#error MIN_UID is undefined.
#endif /* !defined(MIN_UID) */

#if MIN_UID <= 0 
#error MIN_UID must be greater than 0.
#endif /* MIN_UID <= 0 */

#if MAX_UID < MIN_UID
#error MAX_UID is smaller than MIN_UID.
#endif /* MAX_UID <= MIN_UID */

#if defined(UID_MAX)
#if MAX_UID > UID_MAX
#error MAX_UID is greater than UID_MAX.
#endif /* MAX_UID > UID_MAX */
#else /* defined(MAX_UID) */
#if MAX_UID > UINT_MAX
#error MAX_UID is greater than UINT_MAX.
#endif /* MAX_UID > UINT_MAX */
#endif /* defined(MAX_UID) */

#if !defined(MIN_GID)
#error MIN_GID is undefined.
#endif /* !defined(MIN_GID) */

#if !defined(MAX_GID)
#error MIN_GID is undefined.
#endif /* !defined(MIN_GID) */

#if MIN_GID <= 0
#error MIN_GID must be greater than 0.
#endif /* MIN_GID <= 0 */

#if MAX_GID < MIN_GID
#error MAX_GID is smaller than MIN_GID.
#endif /* MAX_GID <= MIN_GID */

#if defined(GID_MAX)
#if MAX_GID > GID_MAX
#error MAX_GID is greater than GID_MAX.
#endif /* MAX_GID > GID_MAX */
#else /* defined(MAX_GID) */
#if MAX_GID > UINT_MAX
#error MAX_GID is greater than UINT_MAX.
#endif /* MAX_GID > UINT_MAX */
#endif /* defined(MAX_GID) */

#if !defined(PATH)
#error PATH is undefined.
#endif /* !defined(PATH) */

#if !defined(HANDLERS)
#error HANDLERS is undefined.
#endif /* !defined(HANDLERS) */

#if defined(PATH_MAX) && PATH_MAX > -1 && PATH_MAX < MAX_STR
#error MAX_STR is greater than PATH_MAX.
#endif /* defined(PATH_MAX) && PATH_MAX > -1 && PATH_MAX < MAX_STR */


/*
 * Include guard off duty.
 */

#endif /* !defined(CONFIG_H). */

