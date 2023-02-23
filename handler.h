/*
 * Header file for handler.c.
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

#if !defined(HANDLER_H)
#define HANDLER_H

#include <sys/types.h>

#include "cattr.h"
#include "types.h"


/*
 * Search for the filename suffix-handler pair matching SCRIPT, but only
 * among the first N elements in the array of filename suffix-handler pairs
 * DB, and return the handler in HDL. DB must have a least N elements.
 *
 * Return value:
 *     OK             Success.
 *     ERR_BAD        The handler for SCRIPT is NULL or the empty string.
 *     ERR_LEN        The filename suffix is too long.
 *     ERR_NO_MATCH   No handler found.
 *     ERR_NO_SUFFIX  SCRIPT has no filename suffix.
 */
__attribute__((nonnull(2, 3, 4), warn_unused_result))
/* cppcheck-suppress misra-c2012-8.2; declaration is in prototype form. */
Error handler_lookup(size_t n, const Pair db[n],
                     const char *script, const char **hdl);

#endif /* !defined(HANDLER_H) */
