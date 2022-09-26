/*
 * suCGI configuration.
 */

/* This is config.h's include guard. Leave it alone. It's on duty. */
#if !defined(CONFIG_H)
#define CONFIG_H


/* FIXME */
#define DOC_ROOT_BASE "/home"


#define DOC_ROOT_PATH "~/public_html"

/*
 * CGI scripts are only executed if they are inside a directory matching
 * this shell wildcard pattern. See fnmatch(3) for the sytax. '*' matches
 * neither ('/') nor leading dots ('.'). Should correspond to the UserDir FIXME
 * directive of your Apache configuration (or its equivalent).
 */

#define DOC_ROOT_PATTERN "~/public_html"
/*
 * Smallest UID that may have been assigned to a regular user.
 * On most systems, this will be 500 (e.g., macOS) or 1,000 (e.g, Debian).
 */
#define MIN_UID 1000U

/*
 * Largest UID that may have been assigned to a regular user.
 * On most systems, this will be 60,000 (though some use 32,767 for nobody).
 */
#define MAX_UID 30000U

/*
 * Smallest GID that may have been assigned to a regular user.
 * On most systems, this will be 500 or 1,000 (e.g, Debian).
 * However, some systems (e.g., macOS) make no such distinction.
 */
#define MIN_GID 1000U

/*
 * Largest GID that may have been assigned to a regular user.
 * On most systems, this will be 30,000 or 60,000 (Debian).
 */
#define MAX_GID 30000U

/*
 * Handlers to run CGI scripts with if their executable is NOT set.
 * Array of filename suffix-handler pairs. The filename suffix must
 * be given including the leading dot (e.g., ".php"). The handler is
 * looked up in $PATH if its filename is relative (e.g., "php");
 * keep in mind that $PATH is overriden with SECURE_PATH (see below).
 * The array must be terminated with a pair of NULLs.
 */
#define SCRIPT_HANDLERS	{					\
	{".php", "php"},					\
	{NULL, NULL}	/* Array terminator. DO NOT REMOVE. */	\
}

/* Secure $PATH. */
#define SECURE_PATH "/usr/bin:/bin"


/* 
 * Secure file permission mask.
 * Does NOT replace the current umask, but is added to it.
 */
#define UMASK 022


/*
 * That's all folks. Don't go beyond this line.
 */

#endif /* !defined(CONFIG_H). */

