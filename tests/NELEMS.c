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

#define _XOPEN_SOURCE 700

#include <err.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdlib.h>

#include "../macros.h"
#include "util/abort.h"
#include "util/types.h"


/*
 * Macros
 */

/* Test NELEMS for the given type for up to "n" elements. */
#define TESTN(type, n)                                                      \
    do {                                                                    \
        for (volatile size_t _test_i = 1; _test_i < (n); ++_test_i) {       \
            type _test_arr[_test_i];                                        \
            size_t _test_n;                                                 \
                                                                            \
            if (sigsetjmp(abort_env, 1) == 0) {                             \
                (void) abort_catch(err);                                    \
                abort_signal = 0;                                           \
                _test_n = NELEMS(_test_arr);                                \
                (void) abort_reset(err);                                    \
                                                                            \
                if (_test_n != _test_i) {                                   \
                    result = FAIL;                                          \
                    warnx("(" #type "[%zu]) → %zu [!]", _test_i, _test_n);  \
                }                                                           \
            } else {                                                        \
                warnx("(" #type "[%zu]) ↑ %d [!]", _test_i, abort_signal);  \
            }                                                               \
        }                                                                   \
    } while (false)

/* Test NELEMS for the given type for up to SHRT_MAX elements. */
#define TEST(type) TESTN(type, SHRT_MAX)


/*
 * Module variables
 */

/* cppcheck-suppress misra-c2012-8.9; TEST need not not be local to main. */
static int result = PASS;


/*
 * Main
 */

int
main(void)
{
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

    return result;
}
