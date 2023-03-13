/*
 * Build configurations for testing.
 *
 * Copyright 2022 and 2023 Odin Kroeger
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

#if !defined(TESTING_H) && defined(TESTING) && TESTING
#define TESTING_H


/*
 * Dependencies
 */

#if !defined(CONFIG_H)
#error config.h must be included before testing.h.
#endif


/*
 * Configuration IDs
 */

#define BC_CHKWEXCL 1
#define BC_ENVCLEAN 2
#define BC_HANDLER  3
#define BC_HIDDEN   4
#define BC_PRIVDROP 5
#define BC_SETID    6
#define BC_USERDIR  7


/*
 * Build configurations
 */

#undef USER_DIR

#if defined(BUILDCONF)
#if   BUILDCONF == BC_CHKWEXCL
#define USER_DIR "/tmp/sucgi-chkwexcl/%s"
#elif BUILDCONF == BC_ENVCLEAN
#define USER_DIR "/tmp/sucgi-envclean/%s"
#elif BUILDCONF == BC_HANDLER
#define USER_DIR "/tmp/sucgi-handler/%s"
#elif BUILDCONF == BC_HIDDEN
#define USER_DIR "/tmp/sucgi-hidden/%s"
#elif BUILDCONF == BC_PRIVDROP
#define USER_DIR "/tmp/sucgi-privdrop/%s"
#elif BUILDCONF == BC_SETID
#define USER_DIR "/tmp/sucgi-setid/%s"
#elif BUILDCONF == BC_USERDIR
#define USER_DIR "/tmp/sucgi-userdir/%s"
#endif
#else /* !defined(BUILDCONF) */
#define USER_DIR "/tmp/sucgi-check/%s"
#endif /* defined(BUILDCONF) */


/*
 * Global settings
 */

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
#define HANDLERS {{".sh", "sh"}, {".empty", ""}}

#undef LOGGING_LEVEL
#define LOGGING_LEVEL (                             \
    LOG_MASK(LOG_EMERG)   | LOG_MASK(LOG_ALERT)  |  \
    LOG_MASK(LOG_CRIT)    | LOG_MASK(LOG_ERR)    |  \
    LOG_MASK(LOG_WARNING) | LOG_MASK(LOG_NOTICE) |  \
    LOG_MASK(LOG_INFO)    | LOG_MASK(LOG_DEBUG)     \
)

#undef LOGGING_OPTIONS
#ifdef LOG_PERROR
#define LOGGING_OPTIONS ( LOG_CONS | LOG_PERROR )
#else
#define LOGGING_OPTIONS ( LOG_CONS )
#endif

#endif /* !defined(TESTING_H) */
