/*
 * Test MIN.
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

#include <assert.h>
#include <err.h>
#include <limits.h>
#include <stdlib.h>

#include "../macros.h"
#include "lib.h"


/*
 * Data types
 */

/* Arguments to test. */
typedef struct {
    long long a;
    long long b;
} Args;


/*
 * Prototypes
 */

/* Check whether MIN returns A, which must be <= B. */
static void test(long long a, long long b);


/*
 * Module variables
 */

/* Test cases */
static Args tests[] = {
    /* Significant values. */
    {LONG_MIN, LONG_MIN},
    {LONG_MIN, LONG_MIN + 1},
    {LONG_MIN, INT_MIN},
    {LONG_MIN, SHRT_MIN},
    {LONG_MIN, -1},
    {LONG_MIN, 0},
    {LONG_MIN, 1},
    {LONG_MIN, SHRT_MAX},
    {LONG_MIN, INT_MAX},
    {LONG_MIN, LONG_MAX},
    {INT_MIN, INT_MIN},
    {INT_MIN, INT_MIN + 1},
    {INT_MIN, SHRT_MIN},
    {INT_MIN, -1},
    {INT_MIN, 0},
    {INT_MIN, 1},
    {INT_MIN, SHRT_MAX},
    {INT_MIN, INT_MAX},
    {INT_MIN, LONG_MAX},
    {SHRT_MIN, SHRT_MIN},
    {SHRT_MIN, SHRT_MIN + 1},
    {SHRT_MIN, -1},
    {SHRT_MIN, 0},
    {SHRT_MIN, 1},
    {SHRT_MIN, SHRT_MAX},
    {SHRT_MIN, INT_MAX},
    {SHRT_MIN, LONG_MAX},
    {-1, -1},
    {-1, 0},
    {-1, 1},
    {-1, SHRT_MAX},
    {-1, INT_MAX},
    {-1, LONG_MAX},
    {0, 0},
    {0, 1},
    {0, SHRT_MAX},
    {0, INT_MAX},
    {0, LONG_MAX},
    {1, 1},
    {1, SHRT_MAX},
    {1, INT_MAX},
    {1, LONG_MAX},
    {SHRT_MAX - 1, SHRT_MAX},
    {SHRT_MAX, SHRT_MAX},
    {SHRT_MAX, INT_MAX},
    {SHRT_MAX, LONG_MAX},
    {INT_MAX - 1, INT_MAX},
    {INT_MAX, INT_MAX},
    {INT_MAX, LONG_MAX},
    {LONG_MAX - 1, LONG_MAX},
    {LONG_MAX, LONG_MAX},

    /* Typical values. */
    {SIGNEDMAX(char), SIGNEDMAX(char)},
    {SIGNEDMAX(char), SIGNEDMAX(short)},
    {SIGNEDMAX(char), SIGNEDMAX(int)},
    {SIGNEDMAX(char), SIGNEDMAX(long)},
    {SIGNEDMAX(char), SIGNEDMAX(long long)},
    {SIGNEDMAX(char), SIGNEDMAX(unsigned char)},
    {SIGNEDMAX(char), SIGNEDMAX(unsigned short)},
    {SIGNEDMAX(char), SIGNEDMAX(unsigned int)},
    {SIGNEDMAX(char), SIGNEDMAX(unsigned long)},
    {SIGNEDMAX(char), SIGNEDMAX(unsigned long long)},
    {SIGNEDMAX(short), SIGNEDMAX(short)},
    {SIGNEDMAX(short), SIGNEDMAX(int)},
    {SIGNEDMAX(short), SIGNEDMAX(long)},
    {SIGNEDMAX(short), SIGNEDMAX(long long)},
    {SIGNEDMAX(short), SIGNEDMAX(unsigned short)},
    {SIGNEDMAX(short), SIGNEDMAX(unsigned int)},
    {SIGNEDMAX(short), SIGNEDMAX(unsigned long)},
    {SIGNEDMAX(short), SIGNEDMAX(unsigned long long)},
    {SIGNEDMAX(int), SIGNEDMAX(int)},
    {SIGNEDMAX(int), SIGNEDMAX(long)},
    {SIGNEDMAX(int), SIGNEDMAX(long long)},
    {SIGNEDMAX(int), SIGNEDMAX(unsigned int)},
    {SIGNEDMAX(int), SIGNEDMAX(unsigned long)},
    {SIGNEDMAX(int), SIGNEDMAX(unsigned long long)},
    {SIGNEDMAX(long), SIGNEDMAX(long)},
    {SIGNEDMAX(long), SIGNEDMAX(long long)},
    {SIGNEDMAX(long), SIGNEDMAX(unsigned long)},
    {SIGNEDMAX(long), SIGNEDMAX(unsigned long long)},
    {SIGNEDMAX(long long), SIGNEDMAX(long long)},
    {SIGNEDMAX(long long), SIGNEDMAX(unsigned long long)}
};


/*
 * Functions
 */

static void
test(long long a, long long b)
{
    assert(a <= b);

    warnx("checking (%lld, %lld) -> %lld ...", a, b, a);

    if (MIN(a, b) != a) {
        errx(TEST_FAILED, "returned %lld", b);
    }

    warnx("checking (%lld, %lld) -> %lld ...", b, a, a);

    if (MIN(b, a) != a) {
        errx(TEST_FAILED, "returned %lld", b);
    }
}


/*
 * Main
 */

int
main (void) {
    for (size_t i = 0; i < NELEMS(tests); ++i) {
        test(tests[i].a, tests[i].b);
    }

    for (long long i = SHRT_MIN; i < SHRT_MAX; ++i) {
        test(i, i + 1);
    }

    warnx("all tests passed");
    return EXIT_SUCCESS;
}
