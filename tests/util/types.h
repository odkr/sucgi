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

#if !defined(TESTS_UTIL_TYPES_H)
#define TESTS_UTIL_TYPES_H

#include <pwd.h>
#include <signal.h>
#include <stdarg.h>

/* Exit statuses for tests. */
typedef enum {  /* cppcheck-suppress misra-c2012-2.4; all tags are used. */
    PASS  =  0,     /* Test passed. */
    ERROR = 69,     /* Test was aborted because of an error. */
    FAIL  = 70,     /* Test failed. */
    SKIP  = 75      /* Test was skipped. */
} Result;

/* Orders in which to traverse trees. */
typedef enum {
    ORDER_PRE,
    ORDER_POST
} Order;

/* Mapping of signals to actions. */
typedef struct {
    int signal; /* cppcheck-suppress unusedStructMember; used by sigs_trap. */
    struct sigaction action;
} Trap;

/* Comparison function. Should obey the same semantics as "strcmp". */
typedef int (*CompFn)(const void *, const void *);

/* Error handling function. Signature is the same as that of "err". */
typedef void (*ErrorFn)(int, const char *, ...);

/* File function. Should obey be the same semantics as "stat". */
typedef int (*FileFn)(const char *);

/* The same as FileFn, but also accepts variadic arguments. */
typedef int (*VFileFn)(const char *, size_t, va_list);

#endif /* !defined(TESTS_UTIL_TYPES_H) */
