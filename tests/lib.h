/*
 * Header for tests.c.
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

#if !defined(TESTS_TESTS_H)
#define TESTS_TESTS_H


/*
 * Data types
 */

/* Exit statuses for tests. */
typedef enum {
    TEST_PASSED  =  0,  /* Test passed. */
    TEST_ERROR   = 69,  /* An error occurred. */
    TEST_FAILED  = 70,  /* Test failed. */
    TEST_SKIPPED = 75   /* Test was skipped. */
} Result;


/*
 * Functions
 */

/*
 * Concatenate the strings pointed to by DEST and SRC, append a NUL, and
 * return a pointer to the terminating NUL in END, but do not write past
 * LIM, which should be DEST + sizeof(DEST).
 *
 * Return value:
 *     0         Success.
 *     Non-zero  Appending SRC to DEST would require to write past LIM.
 */
__attribute__((nonnull(1, 2, 3, 4), warn_unused_result))
int cat_strs(char *dest, const char *const src,
             const char *const lim, char **end);

/*
 * Join the first N strings in STRS using the separator SEP and store the
 * result in DEST, which must be large enough to hold SIZE bytes. If STRS
 * contains a NULL pointer, processing stops at that pointer. STRS must
 * either be NULL-terminated or have at least N elements.
 *
 * Return value:
 *     0         Success.
 *     Non-zero  SIZE is too small to hold the result.
 */
__attribute__((nonnull(2, 3, 5), warn_unused_result))
int join_strs(size_t n, const char *const *strs, const char *sep,
              size_t size, char *dest);

/*
 * Fill the first N - 1 bytes of DEST with CH and then NUL-terminate it.
 */
__attribute__((nonnull(3)))
void fill_str(const char ch, const size_t n, char *dest);

/*
 * Generate a big endian representation of NUM in BASE, using the given
 * DIGITS, and store it in STR, which must be at least SIZE bytes long.
 * SIZE must be >= floor(log(NUM) / log(BASE)) + 2.
 *
 * Return value:
 *     0         Success.
 *     Non-zero  SIZE is too small to hold the result.
 */
__attribute__((nonnull(3, 5), warn_unused_result))
int to_str(unsigned int num, unsigned int base, const char *digits,
           size_t size, char *dest);


#endif /* defined(TESTS_TESTS_H) */
