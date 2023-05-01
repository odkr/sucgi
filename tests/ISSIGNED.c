/*
 * Test ISSIGNED.
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
#include <setjmp.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../macros.h"
#include "util/check.h"


/*
 * Macros
 */

/* Test whether ISSIGNED returns RET for TYPE. */
#define TEST(type, ret)                                             \
    do {                                                            \
        int _test_jumpval;                                          \
                                                                    \
        _test_jumpval = sigsetjmp(checkenv, true);                  \
        if (_test_jumpval == 0) {                                   \
            checking = 1;                                           \
            bool _test_ret = ISSIGNED(type);                        \
            checking = 0;                                           \
                                                                    \
            if (_test_ret != (ret)) {                               \
                result = TEST_FAILED;                               \
                warnx("(" #type ") → %d [!]", _test_ret);           \
            }                                                       \
        } else {                                                    \
                warnx("(" #type ") ↑ %d [!]", _test_jumpval);       \
        }                                                           \
    } while (false)


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
    if (checkinit() != 0) {
        err(TEST_ERROR, "sigaction");
    }

    TEST(signed char, true);
    TEST(unsigned char, false);
    TEST(signed short, true);
    TEST(unsigned short, false);
    TEST(signed int, true);
    TEST(unsigned int, false);
    TEST(signed long, true);
    TEST(unsigned long long, false);
    TEST(float, true);
    TEST(double, true);
    TEST(long double, true);

    return result;
}
