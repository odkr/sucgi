/*
 * Header file for handler.c.
 *
 * Copyright 2022 and 2023 Odin Kroeger.
 *
 * This file is part of suCGI.
 *
 * suCGI is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * suCGI is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General
 * Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with suCGI. If not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(HANDLER_H)
#define HANDLER_H

#include <sys/types.h>

#include "attr.h"
#include "types.h"

/*
 * Search for the first handler matching the suffix of the given filename in
 * the given array of filename suffix-handler pairs and return a pointer to
 * that handler in the variable pointed to by the parameter "handler"; if no
 * matching handler was found or an error occurred, return a null pointer
 * instead.
 *
 * The array of filename suffix-handler pairs must contain at least
 * "nhandlers" pairs; supernumery pairs are ignored.
 *
 * The filename must be of the given length.
 *
 * Return value:
 *     OK          Success.
 *     ERR_BAD     The handler that was found is NULL.
 *     ERR_LEN     The filename suffix is too long.
 *     ERR_SEARCH  None of the given suffixes matches the filename.
 *     ERR_SUFFIX  The filename has no suffix.
 */

_read_only(2, 1) _read_only(4, 3) _write_only(5) _nonnull(2, 4, 5) _nodiscard
Error handler_find(size_t nhandlers, const Pair *handlerdb,
                   size_t fnamelen, const char *fname,
                   const char **handler);

#endif /* !defined(HANDLER_H) */
