/*
 * Test NELEMS.
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
#include <stdlib.h>

#include "../macros.h"
#include "lib.h"


/*
 * Macros
 */

/* Test NELEMS for the given TYPE for up to N elements. */
#define TESTN(type, n)                                          \
    do {                                                        \
        warnx("testing " #type " ...");                         \
                                                                \
        for (size_t _test_i = 1; _test_i < (n); ++_test_i) {    \
            type _test_arr[_test_i];                            \
            size_t _test_n;                                     \
                                                                \
            _test_n = NELEMS(_test_arr);                        \
            if (_test_n != _test_i) {                           \
                errx(TEST_FAILED, "[%zu] -> counted %zu!",      \
                     _test_i, _test_n);                         \
            }                                                   \
        }                                                       \
    } while (0)

/* Test NELEMS for the given TYPE for up to SHRT_MAX elements. */
#define TEST(type) TESTN(type, SHRT_MAX)


/*
 * Main
 */

int
main (void) {
    TEST(signed char);
    TEST(unsigned char);
    TEST(signed short);
    TEST(unsigned short);
    TEST(signed int);
    TEST(unsigned int);
    TEST(signed long);
    TEST(unsigned long long);
    TEST(float);
    TEST(double);
    TEST(long double);

    warnx("all tests passed");
    return EXIT_SUCCESS;
}
