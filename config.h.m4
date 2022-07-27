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
#if !defined(CONFIG_H)
#define CONFIG_H


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
 * Smallest UID that may have been assigned to a regular user.
 *
 * On most systems, this will be 500 (e.g., macOS) or 1000 (e.g, Debian).
 */
#define MIN_UID 1000

/*
 * Largest UID that may have been assigned to a regular user.
 *
 * Some privileged users (e.g., nobody) may have been assigned high user IDs.
 * On most systems, the largest UID of a regular user will be 60,000.
 * Although some systems use 32,767 for nobody.
 */
#define MAX_UID 30000

/*
 * Interpreters to run scripts the executable bit of which is NOT set.
 *
 * Array of key-value pairs, where filename endings are given as keys and
 * interpreters as values; must be terminated with a pair of NULLs.
 *
 * Filename endings must be given including the leading dot (".").
 * Interpreters are searched for in SECURE_PATH (see below).
 */
#define SCRIPT_HANDLERS	{					\
	{.key = ".php", .value = "php"},			\
	{NULL, NULL}	/* Array terminator. DO NOT REMOVE. */	\
}

/* A secure $PATH. */
#define SECURE_PATH "/usr/bin:/bin"


/*
 * That's all folks. Don't go beyond this line.
 */

#endif /* !defined(CONFIG_H). */
]