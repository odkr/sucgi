/*
 * Test copystr.
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
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../macros.h"
#include "../params.h"
#include "../str.h"
#include "check.h"
#include "str.h"


/*
 * Constants
 */

/* Maximum array length. */
#define MAX_ARRAY_LEN 32U


/*
 * Data types
 */

/* Mapping of arguments to return values. */
typedef struct {
    const char *const str;
    const size_t maxnspecs;
    const size_t nspecs;
    const char *const specs[100];
    const Error retval;
    const int signo;
} Args;


/*
 * Module variables
 */

#if !defined(NDEBUG)
/* A string that that exceeds MAX_STR_LEN. */
static char hugestr[MAX_STR_LEN + 1U] = {0};
#endif

/* A string just within MAX_STR_LEN */
static char longstr[MAX_STR_LEN] = {0};

/* Tests. */
static const Args cases[] = {
#if !defined(NDEBUG)
    /* Invalid argument. */
    {hugestr, MAX_ARRAY_LEN, 0, {NULL}, OK, SIGABRT},
#endif

    /* Long string. */
    {longstr, MAX_ARRAY_LEN, 0, {NULL}, OK, 0},

    /* Empty string. */
    {"", MAX_ARRAY_LEN, 0, {NULL}, OK, 0},

    /* Simple tests. */
    {"foo", MAX_ARRAY_LEN, 0, {NULL}, OK, 0},
    {"%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
    {"foo%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
    {"%sfoo", MAX_ARRAY_LEN, 1, {"sfoo"}, OK, 0},
    {"f%so", MAX_ARRAY_LEN, 1, {"so"}, OK, 0},
    {"%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
    {"foo%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
    {"%sfoo", MAX_ARRAY_LEN, 1, {"sfoo"}, OK, 0},
    {"f%so", MAX_ARRAY_LEN, 1, {"so"}, OK, 0},

    /* Multiple specifiers. */
    {"%s%s", MAX_ARRAY_LEN, 2, {"s%s", "s"}, OK, 0},
    {"foo%s%s", MAX_ARRAY_LEN, 2, {"s%s", "s"}, OK, 0},
    {"%s%sfoo", MAX_ARRAY_LEN, 2, {"s%sfoo", "sfoo"}, OK, 0},
    {"f%s%so", MAX_ARRAY_LEN, 2, {"s%so", "so"}, OK, 0},
    {"%sfoo%s", MAX_ARRAY_LEN, 2, {"sfoo%s", "s"}, OK, 0},
    {"%s%s", 1, 1, {"s%s"}, ERR_LEN, 0},
    {"%s%s%s", 2, 2, {"s%s%s", "s%s"}, ERR_LEN, 0},
    {"%s%s%s%s", 3, 3, {"s%s%s%s", "s%s%s", "s%s"}, ERR_LEN, 0},

    /* One escaped specifier. */
    {"%%s", MAX_ARRAY_LEN, 0, {NULL}, OK, 0},
    {"foo%%s", MAX_ARRAY_LEN, 0, {NULL}, OK, 0},
    {"%%sfoo", MAX_ARRAY_LEN, 0, {NULL}, OK, 0},
    {"f%%so", MAX_ARRAY_LEN, 0, {NULL}, OK, 0},

    /* Specifiers after escaped '%'. */
    {"%%%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
    {"%%%s%d", MAX_ARRAY_LEN, 2, {"s%d", "d"}, OK, 0},
    {"foo%%%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
    {"%%sfoo%%%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
    {"%%sfoo%%%s%d", MAX_ARRAY_LEN, 2, {"s%d", "d"}, OK, 0},
    {"f%%so%%%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
    {"%%%%%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
    {"%%%%%s%%%a", MAX_ARRAY_LEN, 2, {"s%%%a", "a"}, OK, 0},
    {"foo%%%%%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
    {"%%sfoo%%%%%s%%%%%e", MAX_ARRAY_LEN, 2, {"s%%%%%e", "e"}, OK, 0},
    {"f%%so%%%%%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
    {"%%s%s%%%e%A", MAX_ARRAY_LEN, 3, {"s%%%e%A", "e%A", "A"}, OK, 0},
    {"foo%%s%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
    {"%%%%sfoo%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
    {"f%%%%so%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},

    /* Unicode shenanigans. */
    {"𝕗ⓞⓞ", MAX_ARRAY_LEN, 0, {NULL}, OK, 0},
    {"%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
    {"𝕗ⓞⓞ%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
    {"%s𝕗ⓞⓞ", MAX_ARRAY_LEN, 1, {"s𝕗ⓞⓞ"}, OK, 0},
    {"𝕗%sⓞ", MAX_ARRAY_LEN, 1, {"sⓞ"}, OK, 0},
    {"%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
    {"𝕗ⓞⓞ%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
    {"%s𝕗ⓞⓞ", MAX_ARRAY_LEN, 1, {"s𝕗ⓞⓞ"}, OK, 0},
    {"𝕗%sⓞ", MAX_ARRAY_LEN, 1, {"sⓞ"}, OK, 0},
    {"%s%s", MAX_ARRAY_LEN, 2, {"s%s", "s"}, OK, 0},
    {"𝕗ⓞⓞ%s%s", MAX_ARRAY_LEN, 2, {"s%s", "s"}, OK, 0},
    {"%s%s𝕗ⓞⓞ", MAX_ARRAY_LEN, 2, {"s%s𝕗ⓞⓞ", "s𝕗ⓞⓞ"}, OK, 0},
    {"𝕗%s%sⓞ", MAX_ARRAY_LEN, 2, {"s%sⓞ", "sⓞ"}, OK, 0},
    {"%s𝕗ⓞⓞ%s", MAX_ARRAY_LEN, 2, {"s𝕗ⓞⓞ%s", "s"}, OK, 0},
    {"%s%s", 1, 1, {"s%s"}, ERR_LEN, 0},
    {"%s%s%s", 2, 2, {"s%s%s", "s%s"}, ERR_LEN, 0},
    {"%s%s%s%s", 3, 3, {"s%s%s%s", "s%s%s", "s%s"}, ERR_LEN, 0},
    {"%%s", MAX_ARRAY_LEN, 0, {NULL}, OK, 0},
    {"𝕗ⓞⓞ%%s", MAX_ARRAY_LEN, 0, {NULL}, OK, 0},
    {"%%s𝕗ⓞⓞ", MAX_ARRAY_LEN, 0, {NULL}, OK, 0},
    {"𝕗%%sⓞ", MAX_ARRAY_LEN, 0, {NULL}, OK, 0},
    {"%%%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
    {"%%%s%d", MAX_ARRAY_LEN, 2, {"s%d", "d"}, OK, 0},
    {"𝕗ⓞⓞ%%%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
    {"%%s𝕗ⓞⓞ%%%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
    {"%%s𝕗ⓞⓞ%%%s%d", MAX_ARRAY_LEN, 2, {"s%d", "d"}, OK, 0},
    {"𝕗%%sⓞ%%%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
    {"%%%%%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
    {"%%%%%s%%%a", MAX_ARRAY_LEN, 2, {"s%%%a", "a"}, OK, 0},
    {"𝕗ⓞⓞ%%%%%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
    {"%%s𝕗ⓞⓞ%%%%%s%%%%%e", MAX_ARRAY_LEN, 2, {"s%%%%%e", "e"}, OK, 0},
    {"𝕗%%sⓞ%%%%%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
    {"%%s%s%%%e%A", MAX_ARRAY_LEN, 3, {"s%%%e%A", "e%A", "A"}, OK, 0},
    {"𝕗ⓞⓞ%%s%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
    {"%%%%s𝕗ⓞⓞ%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0},
    {"𝕗%%%%sⓞ%s", MAX_ARRAY_LEN, 1, {"s"}, OK, 0}
};


/*
 * Prototypes
 */

/* FIXME */
__attribute__((nonnull(1, 2, 4)))
static void
report(const char *message, const Args *args, size_t nspecs,
       const char *const *specs, Error retval, int signo);

/* FIXME */
__attribute__((nonnull(2, 3), warn_unused_result))
static bool
cmpstrs(size_t nelems, const char *const *a, const char *const *b);


/*
 * Functions
 */

static void
report(const char *const message, const Args *const args,
       const size_t nspecs, const char *const *const specs,
       const Error retval, const int signo)
{
    char specstr[MAX_STR_LEN];

    (void) joinstrs(nspecs, specs, ", ", sizeof(specstr), specstr);

/* message is not a literal. */
#if defined(__GNUC__) && __GNUC__ >= 3
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif

    warnx(message, args->str, args->maxnspecs, nspecs, specstr,
          retval, strsignal(signo));

#if defined(__GNUC__) && __GNUC__ >= 3
#pragma GCC diagnostic pop
#endif
}

static bool
cmpstrs(const size_t nelems,
        const char *const *const a, const char *const *const b)
{
    assert(a != NULL);
    assert(b != NULL);

    for (size_t i = 0; i < nelems; ++i) {
        if (a[i] == NULL) {
            if (b[i] != NULL) {
                return false;
            }
            continue;
        }

        if (strncmp(a[i], b[i], MAX_STR_LEN) != 0) {
            return false;
        }
    }

    return true;
}


/*
 * Main
 */

int
main (void) {
    volatile int result = TEST_PASSED;

    checkinit();

#if !defined(NDEBUG)
    (void) memset(hugestr, 'x', sizeof(hugestr) - 1U);
#endif
    (void) memset(longstr, 'x', sizeof(longstr) - 1U);

    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];
        const char *specs[MAX_ARRAY_LEN];
        size_t nspecs;
        int jumpval;
        volatile Error retval;

        jumpval = sigsetjmp(checkenv, true);

        if (jumpval == 0) {
            checking = 1;
            retval = getspecstrs(args.str, args.maxnspecs, &nspecs, specs);
            checking = 0;

            if (retval != args.retval) {
                result = TEST_FAILED;
                report("(%s, %zu, → %zu, → {%s}) → %u [!] ↑ %s)",
                       &args, nspecs, specs, retval, jumpval);
            }

            if (retval == OK) {
                if (nspecs != args.nspecs) {
                    result = TEST_FAILED;
                    report("(%s, %zu, → %zu [!], → {%s}) → %u ↑ %s)",
                           &args, nspecs, specs, retval, jumpval);
                }

                if (!cmpstrs(nspecs, args.specs, specs)) {
                    result = TEST_FAILED;
                    report("(%s, %zu, → %zu, → {%s} [!]) → %u ↑ %s)",
                           &args, nspecs, specs, retval, jumpval);
                }
            }
        }

        if (jumpval != args.signo) {
            result = TEST_FAILED;
            report("(%s, %zu, → %zu, → {%s}) → %u ↑ %s [!])",
                   &args, nspecs, specs, retval, jumpval);
        }
    }

    return result;
}
