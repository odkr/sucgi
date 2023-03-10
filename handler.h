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
 * Search for the handler that matches the filename suffix of SCRIPT
 * in the array of filename suffix-handler pairs HANDLERDB and return
 * it in HANDLER. HANDLERDB must contain at least NELEMS elements;
 * supernumery elements are ignored.
 *
 * Return value:
 *     OK             Success.
 *     ERR_BAD        Matching handler is NULL or the empty string.
 *     ERR_LEN        Filename suffix is too long.
 *     ERR_SEARCH   No handler found.
 */
__attribute__((nonnull(2, 3, 4), warn_unused_result))
Error handler_lookup(size_t nelems, const Pair *handlerdb,
                     const char *script, const char **handler);

#endif /* !defined(HANDLER_H) */
