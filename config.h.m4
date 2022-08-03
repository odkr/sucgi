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
 * CGI programmes are only executed if they are inside a directory matching
 * this shell wildcard pattern. See fnmatch(3) for the sytax. '*' matches
 * neither ('/') nor leading dots ('.'). Should correspond to the UserDir
 * directive of your Apache configuration (or its equivalent).
 */
#define DOC_ROOT "/home/*/public_html"

/*
 * Smallest UID that may have been assigned to a regular user.
 * On most systems, this will be 500 (e.g., macOS) or 1000 (e.g, Debian).
 */
#define MIN_UID 1000

/*
 * Largest UID that may have been assigned to a regular user.
 * On most systems, this will be 60,000 (though some use 32,767 for nobody).
 */
#define MAX_UID 30000

/*
 * Interpreters to run scripts the executable bit of which is NOT set.
 * Array of key-value pairs, where filename endings are given as keys and
 * interpreters as values; must be terminated with a pair of NULLs.
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
