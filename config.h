/*
 * suCGI configuration.
 *
 * Copyright 2022 and 2023 Odin Kroeger.
 *
 * This file is part of suCGI.
 *
 * SuCGI is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * SuCGI is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General
 * Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with suCGI. If not, see <https://www.gnu.org/licenses/>.
 */

/* This is config.h's include guard. Leave it alone. It's on duty. */
#if !defined(CONFIG_H)
#define CONFIG_H


/*
 * Just in case your C is rusty:
 *
 *  - #define statements are terminated with a linefeed, not a semicolon.
 *  - For a #define to span multiple lines,
 *    the linefeed must be escaped with a backslash.
 *  - Strings are given in double quotes ("..."), not single quotes ('...').
 */


/*
 * The document root of user websites. Filename pattern.
 * CGI scripts are only run if they reside under their owner's document root.
 *
 * This constant mirrors Apache's UserDir directive, save for that a "%s"
 * printf conversion specifier is used instead of an "*". That is:
 *
 * (1) If USER_DIR is an absolute filename and contains a single "%s",
 *     "%s" is replaced with the user's login name.
 *     For example: "/srv/web/%s/html" -> "/srv/web/jdoe/html".
 *     There must be at most one format specifier, and it must be "%s".
 *     printf's escaping rules apply.
 *
 * (2) If USER_DIR is an absolute filename but does not contain a "%s",
 *     a "/" and the user's login name are appended to the filename.
 *     For example: "/srv/web" -> "/srv/web/jdoe".
 *     printf's escaping rules do not apply.
 *
 * (3) If USER_DIR is a relative filename,
 *     the filename is prefixed with the user's home directory and a "/".
 *     For example: "public_html" -> "/home/jdoe/public_html.
 *     Format specifiers carry no special meaning and are used as is.
 *     printf's escaping rules do not apply.
 */
/* #define USER_DIR "public_html" */


/*
 * Range of user IDs reserverd for non-system users. Integers.
 *
 * Usually starts at 100, 500, or 1,000 and ends at 30,000 or 60,000.
 * You MUST set [START_UID, STOP_UID] to a subset of that range.
 *
 * Only CGI scripts owned by non-system users can be executed with suCGI.
 */
/* #define START_UID 1000 */
/* #define STOP_UID 30000 */


/*
 * Range of group IDs reserved for non-system groups. Integers.
 *
 * Usually starts at 100, 500, or 1,000 and ends at 30,000 or 60,000.
 * You MUST set [START_GID .. STOP_GID] to a subset of that range.
 *
 * On systems that make no such reservation,
 * exclude as many system groups as feasible.
 *
 * Only CGI scripts owned by users who are only members of
 * non-system groups can be executed with suCGI.
 */
/* #define START_GID 1000 */
/* #define STOP_GID 30000 */


/*
 * Handlers to run CGI scripts with.
 * Array of filename suffix-handler pairs.
 *
 * The filename suffix must include the leading dot (e.g., ".php").
 * The handler is looked up in $PATH if its name is relative (e.g., "php").
 * Keep in mind that $PATH is set to PATH (see below).
 *
 * If no handler can be found, suCGI will execute the CGI script itself.
 */
/*
#define HANDLERS { \
    {".php", "php"}, \
}
*/


/*
 * Secure $PATH. String literal. Colon-separated list of directories.
 */
/*
#define PATH "/usr/bin:/bin"
 */


/*
 * Secure file permission mask. Unsigned integer.
 *
 * Permission masks are often given as octal numbers (e.g., 022 for go-w).
 * For a number to be interpreted as octal by the C compiler, it must be
 * prefixed with a zero (i.e., match the regular expression /^0[0-9]+/).
 */
/*
#define UMASK (S_ISUID | S_ISGID | S_ISVTX | S_IRWXG | S_IRWXO)
 */


/*
 * Priorities to log. Syslog constant.
 * See syslog(3) for details.
 */
/*
#define SYSLOG_MASK LOG_UPTO(LOG_ERR)
 */


#endif /* !defined(CONFIG_H) */
