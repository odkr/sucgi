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

#if !defined(TESTING_H) && defined(CHECK) && CHECK
#define TESTING_H

#if !defined(CONFIG_H)
#error config.h must be included before testing.h.
#endif

#undef USER_DIR
#define USER_DIR "/tmp/sucgi-check/%s"

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

#undef LOGGING_MASK
#define LOGGING_MASK LOG_UPTO(LOG_DEBUG)

#undef LOGGING_OPTIONS
#ifdef LOG_PERROR
#define LOGGING_OPTIONS (LOG_CONS | LOG_PERROR)
#else
#define LOGGING_OPTIONS LOG_CONS
#endif

#endif /* !defined(TESTING_H) */
