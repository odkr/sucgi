/*
 * Test ISSIGNED.
 *
 * Copyright 2022 and 2023 Odin Kroeger
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

#include <err.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../macros.h"
#include "lib.h"


/*
 * Macros
 */

/* Test whether ISSIGNED returns RET for TYPE. */
#define TEST(type, ret)                                             \
    do {                                                            \
        bool _test_ret = (ret);                                     \
                                                                    \
        warnx("checking (" #type ") -> %d ...", _test_ret);         \
                                                                    \
        if (ISSIGNED(type) != _test_ret) {                          \
            errx(TEST_FAILED, "returned %d", _test_ret);            \
        }                                                           \
    } while (0)


/*
 * Main
 */

int
main (void) {
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

    warnx("all tests passed");
    return EXIT_SUCCESS;
}
