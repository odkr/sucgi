changequote([, ])dnl
changecom()dnl
define([default], [ifdef([$1], [ifelse($1, [], [$2], [$1])], [$2])])dnl
[/* 
 * suCGI configuration.
 */

/*
 * This is config.h's include guard.
 * Leave it alone. It's on duty.
 */
#if !defined(INCLUDED_CONFIG)
#define INCLUDED_CONFIG


/*
 * Which document root directories are safe?
 *
 * Shell wildcard pattern.
 * CGI programmes are only run if they are inside a matching document root.
 *
 * This definition should correspond to the UserDir directive of your
 * Apache configuration (or the equivalent of your webserver).
 */
#define DOC_ROOT "/home/*/public_html"

/*
 * Smallest UID that may be assigned to a regular user.
 *
 * On most systems, this will be 500 (e.g., macOS) or 1000 (e.g, Debian).
 */
#define MIN_UID 1000

/*
 * Largest UID that may be assigned to a regular user.
 *
 * Some privileged users (e.g., nobody) tend to be assigned high user IDs.
 * On most systems, the largest UID of a regular user will be 60,000.
 * Note, some systems use 32,767 for nobody.
 */
#define MAX_UID 30000

/*
 * Interpreters to use to run scripts.
 *
 * Array of filename ending-interpreter pairs, where filename endings and
 * interpreters are separated by an equals sign ("="); must be terminated
 * with a NULL-value.
 *
 * Filename endings must be given including the leading dot (".").
 * Interpreters are searched for in the given SECURE_PATH (see below).
 *
 * If a script's owner has permission to execute that script, that is, if the
 * script's executable bits are set appropriately, then the script is called
 * directly, not via an interpreter, and this setting does not apply.
 */
#define SCRIPT_HANDLERS	{				\
	".php=php",					\
	NULL	/* Array terminator. DO NOT REMOVE. */	\
}

/* A secure $PATH. */
#define SECURE_PATH "/usr/bin:/bin"


/*
 * That's all folks. Don't go beyond this line.
 */

#endif /* Include guard. */
]