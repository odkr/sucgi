/*
 * Headers for utils.c.
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

#if !defined(INCLUDED_UTILS)
#define INCLUDED_UTILS

#include "attr.h"


/*
 * Run script with the first matching interpreter in pairs.
 *
 * pairs is a list of filename ending-interpreter pairs, where filename
 * endings and interpreters are separated by an equals sign ("=").
 *
 * Filename endings must be given including the leading dot (".").
 * Interpreters are searched for in $PATH.
 *
 * The first interpreter that is associated with the script's filename ending
 * is executed with execlp(3) and given the script as first and only argument.
 * 
 * run_script only returns if an error occurred.
 * errno(2) should be set in this case.
 */
// This is not a call to access.
// flawfinder: ignore
__attribute__((access(read_only, 1), access(read_only, 2)))
void run_script (const char *script, const char **pairs);


#endif /* Include guard. */
