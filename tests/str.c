/*
 * Utility string functions for testing.
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

#define _BSD_SOURCE
#define _DARWIN_C_SOURCE
#define _DEFAULT_SOURCE
#define _GNU_SOURCE


#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "str.h"


/*
 * Prototypes
 */

/*
 * Concatenate the strings pointed to by DEST and SRC, append a NUL, and
 * return a pointer to the terminating NUL in END, but do not write past
 * LIM, which should be DEST + sizeof(DEST).
 *
 * If NDEBUG is defined, no error checking takes place.
 * Otherwise, aborts if appending SRC to DEST would require to write past LIM.
 */
__attribute__((nonnull(1, 2, 3, 4)))
static void catstrs(char *dest, const char *const src,
                    const char *const lim, char **end);


/*
 * Functions
 */

static void
catstrs(char *const dest, const char *const src,
        const char *const lim, char **const end)
{
    assert(dest != NULL);
    assert(src != NULL);
    assert(lim != NULL);
    assert(lim > dest);
    assert(end != NULL);

    *end = stpncpy(dest, src, (size_t) (lim - dest));
    assert(src[*end - dest] == '\0');
}

void
joinstrs(const size_t nstrs, const char *const *const strs,
         const char *const sep, const size_t size, char *const dest)
{
    char *ptr;
    char *lim;

    assert(strs != NULL);
    assert(sep != NULL);
    assert(dest != NULL);

    ptr = dest;
    lim = dest + size - 1U;

    (void) memset(dest, '\0', size);
    for (size_t i = 0U; i < nstrs && strs[i]; ++i) {
        if (i > 0U) {
            catstrs(ptr, sep, lim, &ptr);
        }

        catstrs(ptr, strs[i], lim, &ptr);
    }
}
