/*
 * Detect system features.
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

#if !defined(SYSDEFS_H)
#define SYSDEFS_H

#include <limits.h>


/* Excise function attributes unless the compiler understands GNU C. */
#if !defined(__GNUC__) || !__GNUC__
#define __attribute__(attr)
#endif

/* Size for arrays that hold filenames. */
#if defined(PATH_MAX) && PATH_MAX > _POSIX_PATH_MAX
#define PATH_MAX_LEN ((size_t) PATH_MAX)
#else
#define PATH_MAX_LEN ((size_t) 4096)
#endif

#endif /* !defined(SYSDEFS_H) */
