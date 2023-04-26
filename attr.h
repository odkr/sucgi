/*
 * Disable C attributes if the compiler does not support them.
 *
 * Copyright 2022 and 2023 Odin Kroeger.
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

#if !defined(ATTR_H)
#define ATTR_H

/*
 * Excise function attributes if the compiler does not
 * understand GNU C or attributes have been disabled.
 */
#if !defined(__GNUC__) || __GNUC__ < 3 || defined(NATTR) && NATTR
#define __attribute__(attr)
#endif


#endif /* !defined(ATTR_H) */
