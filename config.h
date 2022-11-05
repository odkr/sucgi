/*
 * suCGI configuration.
 */

/* This is config.h's include guard. Leave it alone. It's on duty. */
#if !defined(CONFIG_H)
#define CONFIG_H


/*
 * Jail directory. Filename.
 * CGI scripts are only run if they are with the jail.
 *
 * Should correspond to the UserDir directive of your Apache configuation
 * (or the equivalent directive of the webserver you use).
 *
 * For example:
 *
 *    Apache               suCGI
 *    -------------------  ---------------------------
 *    UserDir public_html  #define JAIL_DIR "/home"
 *    UserDir /srv/web     #define JAIL_DIR "/srv/web"
 *
 */
#define JAIL_DIR "/home"

/*
 * The document root of user websites. Filename pattern. 
 * CGI scripts are only run if they are with the user's document root.
 *
 * Mirrors Apache's *UserDir* directive, save for that suCGI uses a "%s"
 * printf(3) string conversion specifier as placeholder for the user's
 * login name, not at "*". That is:
 *
 * (1) If USER_DIR is an absolute filename and contains a "%s", the "%s" is
 *     replaced with the user's login name; for example, "/srv/web/%s/html"
 *     translates to "/srv/web/jdoe/html". There must be precisely one
 *     conversion specifier. printf's escaping rules apply.
 *
 * (2) If USER_DIR is an absolute filename but does *not* contain a "%s",
 *     a path separator and the user's login name are appended to the given
 *     directory; for example, "/srv/web" becomes "/srv/web/jdoe".
 *
 * (3) If USER_DIR is a relative filename, it is prefixed with the user's
 *     home directory and a path separator; for example, "public_html"
 *     becomes "/home/jdoe/public_html".
 *
 * printf conversion specifiers carry no special meaning in (2) and (3);
 * nor do its escaping rules apply.
 *
 * For example:
 *
 *    Apache               suCGI
 *    -------------------  ---------------------------
 *    UserDir public_html  #define USER_DIR "public_html"
 *    UserDir /srv/web     #define USER_DIR "/srv/web"
 */
#define USER_DIR "public_html"

/*
 * Smallest UID that may be assigned to a regular user. Unsigned integer.
 * On most systems, this will be 500 (e.g., macOS) or 1,000 (e.g, Debian).
 */
#define MIN_UID 1000

/*
 * Largest UID that may be assigned to a regular user. Unsigned integer.
 * On most systems, this will be 60,000 (though some use 32,767 for nobody).
 */
#define MAX_UID 30000

/*
 * Smallest GID that may be assigned to a regular user. Unsigned integer.
 * On most systems, this will be 500 or 1,000 (e.g, Debian).
 * However, some systems (e.g., macOS) make no such distinction.
 */
#define MIN_GID 1000

/*
 * Largest GID that may be assigned to a regular user. Unsigned integer.
 * On most systems, this will be 30,000 or 60,000 (Debian).
 */
#define MAX_GID 30000

/*
 * Handlers to run CGI scripts with if their executable bit is unset.
 * Array of filename suffix-script interpreter pairs.
 * 
 * The filename suffix must be given including the leading dot (e.g., ".php").
 * The interpreter is looked up in $PATH if its name is relative (e.g., "php"),
 * but keep in mind that $PATH is set to SEC_PATH (see below).
 *
 * The array must be terminated with a pair of NULLs.
 */
#define HANDLERS	{					\
	{".php", "php"},					\
	{NULL, NULL}	/* Array terminator. DO NOT REMOVE. */	\
}

/*
 * A secure $PATH. Colon-separated list of directories.
 */
#define SEC_PATH "/usr/bin:/bin"

/* 
 * A secure file permission mask. Unsigned integer.
 * The leading "0" is significant! Added to the current umask.
 */
#define UMASK 077U


#endif /* !defined(CONFIG_H). */
