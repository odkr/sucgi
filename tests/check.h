/*
 * Header for util.c.
 *
 * Copyright 2023 Odin Kroeger.
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

#if !defined(TESTS_LIB_CHECK_H)
#define TESTS_LIB_CHECK_H

#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <unistd.h>

#include "../cattr.h"


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
 * Global variables
 */

/* Is a test running? */
extern volatile sig_atomic_t checking;

/* Environment to run tests in. */
extern sigjmp_buf checkenv;


/*
 * Functions
 */

/*
 * Register signal handler.
 */
void checkinit(void);


#endif /* !defined(TESTS_LIB_CHECK_H) */
