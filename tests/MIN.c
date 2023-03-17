/*
 * Test MIN.
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

#include <assert.h>
#include <err.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>

#include "../macros.h"
#include "result.h"


/*
 * Data types
 */

/* Arguments to test. */
typedef struct {
    long long smaller;
    long long greater;
} Args;


/*
 * Prototypes
 */

/*
 * Check if MIN(A, B) returns MIN.
 */
static bool test(long long a, long long b, long long min);


/*
 * Module variables
 */

/* Test cases */
static Args cases[] = {
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

static bool
test(const long long a, const long long b, const long long min)
{
    long long ret;

    ret = MIN(a, b);
    if (ret != min) {
        warnx("(%lld, %lld) -> %lld [!]", a, b, min);
        return false;
    }

    return true;
}


/*
 * Main
 */

int
main (void) {
    int result = TEST_PASSED;

    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];

        assert(args.smaller <= args.greater);

        if (!test(args.smaller, args.greater, args.smaller)) {
            result = TEST_FAILED;
        };
        if (!test(args.greater, args.smaller, args.smaller)) {
            result = TEST_FAILED;
        }
    }

    return result;
}
