/*
 * C attribute handling.
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

#if !defined(SRC_ATTR_H)
#define SRC_ATTR_H

/* Excise function attributes unless the compiler understands GNU C. */
#if !defined(__GNUC__)
#define __atribute__(attr)
#endif /* !defined(__GNUC__) */

/* Shorthand for access(read_only, ...). */
// This is not a call to the access() function.
// flawfinder: ignore.
#define ACCESS_RO(...) access(read_only, __VA_ARGS__)

#endif /* !defined(SRC_ATTR_H) */
