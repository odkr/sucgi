/*
 * Test MAXSVAL.
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

#define _XOPEN_SOURCE 700

#include <err.h>
#include <inttypes.h>
#include <limits.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdlib.h>

#include "../macros.h"
#include "libutil/abort.h"
#include "libutil/types.h"


/*
 * Macros
 */

/* Check whether MAXSVAL returns "n" for the given type. */
#define TEST(type, n)                                           \
    do {                                                        \
        const unsigned long long _test_n = (n);                 \
                                                                \
        if (sigsetjmp(abort_env, 1) == 0) {                     \
                                                                \
            (void) abort_catch(err);                            \
            const unsigned long long _test_m = MAXSVAL(type);   \
            (void) abort_reset(err);                            \
                                                                \
            if (_test_m != _test_n) {                           \
                result = FAIL;                                  \
                warnx("(" #type ") → %llu [!]", _test_m);       \
            }                                                   \
        } else {                                                \
            warnx("(" #type ") ↑ %d [!]", abort_signal);        \
        }                                                       \
    } while (false)


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
    warnx("the following tests will fail for types with padding bits.");

    TEST(signed char, CHAR_MAX);
    TEST(unsigned char, CHAR_MAX);
    TEST(signed short, SHRT_MAX);
    TEST(unsigned short, SHRT_MAX);
    TEST(signed int, INT_MAX);
    TEST(unsigned int, INT_MAX);
    TEST(signed long, LONG_MAX);
    TEST(unsigned long, LONG_MAX);
    TEST(signed long long, LLONG_MAX);
    TEST(unsigned long long, LLONG_MAX);
    TEST(intmax_t, INTMAX_MAX);
    TEST(uintmax_t, INTMAX_MAX);

    return result;
}
