/*
 * Limits for suCGI.
 *
 * Copyright 2022 Odin Kroeger
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

#if !defined(MAX_H)
#define MAX_H

#include <limits.h>


/* Maximum size for filenames. */
#if defined(PATH_MAX) && PATH_MAX > _POSIX_PATH_MAX
#define MAX_FNAME ((size_t) PATH_MAX)
#else
#define MAX_FNAME ((size_t) 4096)
#endif

/* Maximum number of groups a user may be a member of. */
#define MAX_NGROUPS 4096

/* Maximum number of environment variables. */
#define MAX_NVARS 256U

/* Maximum length of an environment variable name. */
#define MAX_VARNAME 64U

#endif /* !defined(MAX_H) */
