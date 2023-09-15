/*
 * Detect the maximum value a type can represent at run-time.
 *
 * Copyright 2023 Odin Kroeger
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

#include <sys/types.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>

#define START (CHAR_MAX + 1)

int
main(void)
{
    uintmax_t i;

    for (i = START; i > 0 && (T) (i - 1) == (i - 1); i *= 2) {
        if ((T) -1 < (T) 0) {
            printf("%j\n", (intmax_t) i - 1);
        } else {
            printf("%ju\n", (uintmax_t) i - 1);
        }
    }

    return 0;
}
