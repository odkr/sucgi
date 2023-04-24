/*
 * Header for str.c.
 *
 * Copyright 2023 Odin Kroeger.
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

#if !defined(TESTS_STR_H)
#define TESTS_STR_H

#include "../cattr.h"


/*
 * Join the first N strings in STRS using the separator SEP and store the
 * result in DEST, which must be large enough to hold SIZE bytes. If STRS
 * contains a NULL pointer, processing stops at that pointer. STRS must
 * either be NULL-terminated or have at least N elements.
 *
 * If NDEBUG is defined, no error checking takes place.
 * Aborts if SIZE is too small to hold the result otherwise.
 */
__attribute__((nonnull(2, 3, 5)))
void joinstrs(size_t nstrs, const char *const *strs,
              const char *sep, size_t size, char *dest);


#endif /* !defined(TESTS_STR_H) */
