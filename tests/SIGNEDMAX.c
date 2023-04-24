/*
 * Test SIGNEDMAX.
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

#define _BSD_SOURCE
#define _DARWIN_C_SOURCE
#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <err.h>
#include <limits.h>
#include <stdlib.h>

#include "../macros.h"
#include "check.h"


/*
 * Macros
 */

/* Check whether SIGNEDMAX returns N for TYPE. */
#define TEST(type, n)                                   \
    do {                                                \
        size_t _test_n = (n);                           \
        size_t _test_m = SIGNEDMAX(type);               \
                                                        \
        if (_test_n != _test_m) {                       \
            result = TEST_FAILED;                       \
            warnx("(" #type ") -> %zu [!]", _test_m);   \
        }                                               \
    } while (0)


/*
 * Module variables
 */

/* The result. */
static int result = TEST_PASSED;


/*
 * Main
 */

int
main (void) {
    TEST(signed char, CHAR_MAX);
    TEST(unsigned char, CHAR_MAX);
    TEST(signed short, SHRT_MAX);
    TEST(unsigned short, SHRT_MAX);
    TEST(signed int, INT_MAX);
    TEST(unsigned int, INT_MAX);
    TEST(signed long, LONG_MAX);
    TEST(unsigned long long, LONG_MAX);

    return result;
}
