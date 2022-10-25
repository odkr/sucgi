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
#define ENFORCE_HOME_DIR true

/*
 * The smallest UID that may have been assigned to a regular user.
 * On most systems, this will be 500 (e.g., macOS) or 1,000 (e.g, Debian).
 */
#define MIN_UID 1000

/*
 * The largest UID that may have been assigned to a regular user.
 * On most systems, this will be 60,000 (though some use 32,767 for nobody).
 */
#define MAX_UID 30000

/*
 * The smallest GID that may have been assigned to a regular user.
 * On most systems, this will be 500 or 1,000 (e.g, Debian).
 * However, some systems (e.g., macOS) make no such distinction.
 */
#define MIN_GID 1000

/*
 * The largest GID that may have been assigned to a regular user.
 * On most systems, this will be 30,000 or 60,000 (Debian).
 */
#define MAX_GID 30000

/*
 * Handlers to run CGI scripts with if their executable is NOT set.
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
 * A secure file permission mask.
 * This mask is added to the current umask, rather than replacing it.
 */
#define UMASK 022

/*
 * Maximum number of environment variables.
 * suCGI aborts if it encounters more environment variables.
 */
#define MAX_ENV 256

/*
 * Maximum lenght of strings, including the terminating NUL.
 * suCGI aborts if it encounters a string that exceeds this limit.
 */
#define MAX_STR 1024

/*
 * Maximum number of groups a user can be a member of.
 * suCGI aborts if a user is a member of more groups.
 */
#define MAX_GROUPS 64


/*
 * That's all folks. Don't go beyond this line.
 */

#endif /* !defined(CONFIG_H). */

