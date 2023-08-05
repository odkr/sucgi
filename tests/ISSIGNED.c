/*
 * Test ISSIGNED.
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

#define _BSD_SOURCE
#define _DARWIN_C_SOURCE
#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <err.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../macros.h"
#include "util/types.h"


/*
 * Macros
 */

/* Test whether ISSIGNED returns "ret" for the given type. */
#define TEST(type, ret)                                             \
    do {                                                            \
        bool _test_ret = ISSIGNED(type);                            \
        if (_test_ret != (ret)) {                                   \
            warnx("(" #type ") → %d [!]", _test_ret);               \
            result = FAIL;                                          \
        }                                                           \
    } while (0)


/*
 * Module variables
 */

/* cppcheck-suppress misra-c2012-8.9; TEST need not not be local to main. */
static int result = PASS;


/*
 * Main
 */

int
main (void) {
    TEST(char, true);
    TEST(unsigned char, false);
    TEST(short, true);
    TEST(unsigned short, false);
    TEST(int, true);
    TEST(unsigned int, false);
    TEST(long, true);
    TEST(unsigned long long, false);
    TEST(float, true);
    TEST(double, true);
    TEST(long double, true);
    TEST(intmax_t, true);
    TEST(uintmax_t, false);

    return result;
}
