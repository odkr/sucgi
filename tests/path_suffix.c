/*
 * Test path_suffix.
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

#define _BSD_SOURCE
#define _DARWIN_C_SOURCE
#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <err.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../macros.h"
#include "../params.h"
#include "../path.h"
#include "libutil/abort.h"
#include "libutil/str.h"
#include "libutil/types.h"


/*
 * Data types
 */

/* Mapping of filenames to return values. */
typedef struct {
    const char *const fname;
    const char *const suffix;
    const Error retval;
    int signal;
} PathSuffixArgs;


/*
 * Module variables
 */


/*
 * Main
 */

int
main (void)
{
#if !defined(NDEBUG)

    /* RATS: ignore; used safely. */
    char hugefname[MAX_FNAME_LEN + 1U];
    str_fill(sizeof(hugefname), hugefname, 'x');
#endif

    /* RATS: ignore; used safely. */
    char longfname[MAX_FNAME_LEN];
    str_fill(sizeof(longfname), longfname, 'x');

    const PathSuffixArgs cases[] = {
        /* Illegal arguments. */
#if !defined(NDEBUG)
        {"", "<n/a>", OK, SIGABRT},
        {hugefname, "<n/a>", OK, SIGABRT},
#endif

        /* Long filename. */
        {longfname, "<n/a>", ERR_SUFFIX, 0},

        /* Various checks. */
        {".", "<n/a>", ERR_SUFFIX, 0},
        {"/", "<n/a>", ERR_SUFFIX, 0},
        {"file.sfx", ".sfx", OK, 0},
        {"file.very-long-suffix", ".very-long-suffix", OK, 0},
        {"file.s", ".s", OK, 0},
        {"file.", ".", OK, 0},
        {"file", "<n/a>", ERR_SUFFIX, 0},
        {".file", "<n/a>", ERR_SUFFIX, 0},
        {"ḟïľē.şḟ𝓍", ".şḟ𝓍", OK, 0},
        {"ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK, 0},
        {"ḟïľē.ş", ".ş", OK, 0},
        {"ḟïľē.", ".", OK, 0},
        {"ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {".ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"/file.sfx", ".sfx", OK, 0},
        {"/file.very-long-suffix", ".very-long-suffix", OK, 0},
        {"/file.s", ".s", OK, 0},
        {"/file.", ".", OK, 0},
        {"/file", "<n/a>", ERR_SUFFIX, 0},
        {"/.file", "<n/a>", ERR_SUFFIX, 0},
        {"/ḟïľē.şḟ𝓍", ".şḟ𝓍", OK, 0},
        {"/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK, 0},
        {"/ḟïľē.ş", ".ş", OK, 0},
        {"/ḟïľē.", ".", OK, 0},
        {"/ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"/.ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"./file.sfx", ".sfx", OK, 0},
        {"./file.very-long-suffix", ".very-long-suffix", OK, 0},
        {"./file.s", ".s", OK, 0},
        {"./file.", ".", OK, 0},
        {"./file", "<n/a>", ERR_SUFFIX, 0},
        {"./.file", "<n/a>", ERR_SUFFIX, 0},
        {"./ḟïľē.şḟ𝓍", ".şḟ𝓍", OK, 0},
        {"./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK, 0},
        {"./ḟïľē.ş", ".ş", OK, 0},
        {"./ḟïľē.", ".", OK, 0},
        {"./ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"./.ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"../file.sfx", ".sfx", OK, 0},
        {"../file.very-long-suffix", ".very-long-suffix", OK, 0},
        {"../file.s", ".s", OK, 0},
        {"../file.", ".", OK, 0},
        {"../file", "<n/a>", ERR_SUFFIX, 0},
        {"../.file", "<n/a>", ERR_SUFFIX, 0},
        {"../ḟïľē.şḟ𝓍", ".şḟ𝓍", OK, 0},
        {"../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK, 0},
        {"../ḟïľē.ş", ".ş", OK, 0},
        {"../ḟïľē.", ".", OK, 0},
        {"../ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"../.ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"dir/file.sfx", ".sfx", OK, 0},
        {"dir/file.very-long-suffix", ".very-long-suffix", OK, 0},
        {"dir/file.s", ".s", OK, 0},
        {"dir/file.", ".", OK, 0},
        {"dir/file", "<n/a>", ERR_SUFFIX, 0},
        {"dir/.file", "<n/a>", ERR_SUFFIX, 0},
        {"ⓓïȓ/ḟïľē.şḟ𝓍", ".şḟ𝓍", OK, 0},
        {"ⓓïȓ/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK, 0},
        {"ⓓïȓ/ḟïľē.ş", ".ş", OK, 0},
        {"ⓓïȓ/ḟïľē.", ".", OK, 0},
        {"ⓓïȓ/ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"ⓓïȓ/.ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"/dir/file.sfx", ".sfx", OK, 0},
        {"/dir/file.very-long-suffix", ".very-long-suffix", OK, 0},
        {"/dir/file.s", ".s", OK, 0},
        {"/dir/file.", ".", OK, 0},
        {"/dir/file", "<n/a>", ERR_SUFFIX, 0},
        {"/dir/.file", "<n/a>", ERR_SUFFIX, 0},
        {"/ⓓïȓ/ḟïľē.şḟ𝓍", ".şḟ𝓍", OK, 0},
        {"/ⓓïȓ/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK, 0},
        {"/ⓓïȓ/ḟïľē.ş", ".ş", OK, 0},
        {"/ⓓïȓ/ḟïľē.", ".", OK, 0},
        {"/ⓓïȓ/ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"/ⓓïȓ/.ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"./dir/file.sfx", ".sfx", OK, 0},
        {"./dir/file.very-long-suffix", ".very-long-suffix", OK, 0},
        {"./dir/file.s", ".s", OK, 0},
        {"./dir/file.", ".", OK, 0},
        {"./dir/file", "<n/a>", ERR_SUFFIX, 0},
        {"./dir/.file", "<n/a>", ERR_SUFFIX, 0},
        {"./ⓓïȓ/ḟïľē.şḟ𝓍", ".şḟ𝓍", OK, 0},
        {"./ⓓïȓ/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK, 0},
        {"./ⓓïȓ/ḟïľē.ş", ".ş", OK, 0},
        {"./ⓓïȓ/ḟïľē.", ".", OK, 0},
        {"./ⓓïȓ/ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"./ⓓïȓ/.ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"../dir/file.sfx", ".sfx", OK, 0},
        {"../dir/file.very-long-suffix", ".very-long-suffix", OK, 0},
        {"../dir/file.s", ".s", OK, 0},
        {"../dir/file.", ".", OK, 0},
        {"../dir/file", "<n/a>", ERR_SUFFIX, 0},
        {"../dir/.file", "<n/a>", ERR_SUFFIX, 0},
        {"../ⓓïȓ/ḟïľē.şḟ𝓍", ".şḟ𝓍", OK, 0},
        {"../ⓓïȓ/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK, 0},
        {"../ⓓïȓ/ḟïľē.ş", ".ş", OK, 0},
        {"../ⓓïȓ/ḟïľē.", ".", OK, 0},
        {"../ⓓïȓ/ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"../ⓓïȓ/.ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"dir.sfx/file.sfx", ".sfx", OK, 0},
        {"dir.sfx/file.very-long-suffix", ".very-long-suffix", OK, 0},
        {"dir.sfx/file.s", ".s", OK, 0},
        {"dir.sfx/file.", ".", OK, 0},
        {"dir.sfx/file", "<n/a>", ERR_SUFFIX, 0},
        {"dir.sfx/.file", "<n/a>", ERR_SUFFIX, 0},
        {"ⓓïȓ.şḟ𝓍/ḟïľē.şḟ𝓍", ".şḟ𝓍", OK, 0},
        {"ⓓïȓ.şḟ𝓍/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK, 0},
        {"ⓓïȓ.şḟ𝓍/ḟïľē.ş", ".ş", OK, 0},
        {"ⓓïȓ.şḟ𝓍/ḟïľē.", ".", OK, 0},
        {"ⓓïȓ.şḟ𝓍/ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"ⓓïȓ.şḟ𝓍/.ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"/dir.sfx/file.sfx", ".sfx", OK, 0},
        {"/dir.sfx/file.very-long-suffix", ".very-long-suffix", OK, 0},
        {"/dir.sfx/file.s", ".s", OK, 0},
        {"/dir.sfx/file.", ".", OK, 0},
        {"/dir.sfx/file", "<n/a>", ERR_SUFFIX, 0},
        {"/dir.sfx/.file", "<n/a>", ERR_SUFFIX, 0},
        {"/ⓓïȓ.şḟ𝓍/ḟïľē.şḟ𝓍", ".şḟ𝓍", OK, 0},
        {"/ⓓïȓ.şḟ𝓍/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK, 0},
        {"/ⓓïȓ.şḟ𝓍/ḟïľē.ş", ".ş", OK, 0},
        {"/ⓓïȓ.şḟ𝓍/ḟïľē.", ".", OK, 0},
        {"/ⓓïȓ.şḟ𝓍/ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"/ⓓïȓ.şḟ𝓍/.ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"./dir.sfx/file.sfx", ".sfx", OK, 0},
        {"./dir.sfx/file.very-long-suffix", ".very-long-suffix", OK, 0},
        {"./dir.sfx/file.s", ".s", OK, 0},
        {"./dir.sfx/file.", ".", OK, 0},
        {"./dir.sfx/file", "<n/a>", ERR_SUFFIX, 0},
        {"./dir.sfx/.file", "<n/a>", ERR_SUFFIX, 0},
        {"./ⓓïȓ.şḟ𝓍/ḟïľē.şḟ𝓍", ".şḟ𝓍", OK, 0},
        {"./ⓓïȓ.şḟ𝓍/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK, 0},
        {"./ⓓïȓ.şḟ𝓍/ḟïľē.ş", ".ş", OK, 0},
        {"./ⓓïȓ.şḟ𝓍/ḟïľē.", ".", OK, 0},
        {"./ⓓïȓ.şḟ𝓍/ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"./ⓓïȓ.şḟ𝓍/.ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"../dir.sfx/file.sfx", ".sfx", OK, 0},
        {"../dir.sfx/file.very-long-suffix", ".very-long-suffix", OK, 0},
        {"../dir.sfx/file.s", ".s", OK, 0},
        {"../dir.sfx/file.", ".", OK, 0},
        {"../dir.sfx/file", "<n/a>", ERR_SUFFIX, 0},
        {"../dir.sfx/.file", "<n/a>", ERR_SUFFIX, 0},
        {"../ⓓïȓ.şḟ𝓍/ḟïľē.şḟ𝓍", ".şḟ𝓍", OK, 0},
        {"../ⓓïȓ.şḟ𝓍/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK, 0},
        {"../ⓓïȓ.şḟ𝓍/ḟïľē.ş", ".ş", OK, 0},
        {"../ⓓïȓ.şḟ𝓍/ḟïľē.", ".", OK, 0},
        {"../ⓓïȓ.şḟ𝓍/ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"../ⓓïȓ.şḟ𝓍/.ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"dir/./file.sfx", ".sfx", OK, 0},
        {"dir/./file.very-long-suffix", ".very-long-suffix", OK, 0},
        {"dir/./file.s", ".s", OK, 0},
        {"dir/./file.", ".", OK, 0},
        {"dir/./file", "<n/a>", ERR_SUFFIX, 0},
        {"dir/./.file", "<n/a>", ERR_SUFFIX, 0},
        {"ⓓïȓ/./ḟïľē.şḟ𝓍", ".şḟ𝓍", OK, 0},
        {"ⓓïȓ/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK, 0},
        {"ⓓïȓ/./ḟïľē.ş", ".ş", OK, 0},
        {"ⓓïȓ/./ḟïľē.", ".", OK, 0},
        {"ⓓïȓ/./ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"ⓓïȓ/./.ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"/./dir/./file.sfx", ".sfx", OK, 0},
        {"/./dir/./file.very-long-suffix", ".very-long-suffix", OK, 0},
        {"/./dir/./file.s", ".s", OK, 0},
        {"/./dir/./file.", ".", OK, 0},
        {"/./dir/./file", "<n/a>", ERR_SUFFIX, 0},
        {"/./dir/./.file", "<n/a>", ERR_SUFFIX, 0},
        {"/./ⓓïȓ/./ḟïľē.şḟ𝓍", ".şḟ𝓍", OK, 0},
        {"/./ⓓïȓ/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK, 0},
        {"/./ⓓïȓ/./ḟïľē.ş", ".ş", OK, 0},
        {"/./ⓓïȓ/./ḟïľē.", ".", OK, 0},
        {"/./ⓓïȓ/./ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"/./ⓓïȓ/./.ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"././dir/./file.sfx", ".sfx", OK, 0},
        {"././dir/./file.very-long-suffix", ".very-long-suffix", OK, 0},
        {"././dir/./file.s", ".s", OK, 0},
        {"././dir/./file.", ".", OK, 0},
        {"././dir/./file", "<n/a>", ERR_SUFFIX, 0},
        {"././dir/./.file", "<n/a>", ERR_SUFFIX, 0},
        {"././ⓓïȓ/./ḟïľē.şḟ𝓍", ".şḟ𝓍", OK, 0},
        {"././ⓓïȓ/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK, 0},
        {"././ⓓïȓ/./ḟïľē.ş", ".ş", OK, 0},
        {"././ⓓïȓ/./ḟïľē.", ".", OK, 0},
        {"././ⓓïȓ/./ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"././ⓓïȓ/./.ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {".././dir/./file.sfx", ".sfx", OK, 0},
        {".././dir/./file.very-long-suffix", ".very-long-suffix", OK, 0},
        {".././dir/./file.s", ".s", OK, 0},
        {".././dir/./file.", ".", OK, 0},
        {".././dir/./file", "<n/a>", ERR_SUFFIX, 0},
        {".././dir/./.file", "<n/a>", ERR_SUFFIX, 0},
        {".././ⓓïȓ/./ḟïľē.şḟ𝓍", ".şḟ𝓍", OK, 0},
        {".././ⓓïȓ/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK, 0},
        {".././ⓓïȓ/./ḟïľē.ş", ".ş", OK, 0},
        {".././ⓓïȓ/./ḟïľē.", ".", OK, 0},
        {".././ⓓïȓ/./ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {".././ⓓïȓ/./.ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"dir.sfx/./file.sfx", ".sfx", OK, 0},
        {"dir.sfx/./file.very-long-suffix", ".very-long-suffix", OK, 0},
        {"dir.sfx/./file.s", ".s", OK, 0},
        {"dir.sfx/./file.", ".", OK, 0},
        {"dir.sfx/./file", "<n/a>", ERR_SUFFIX, 0},
        {"dir.sfx/./.file", "<n/a>", ERR_SUFFIX, 0},
        {"ⓓïȓ.şḟ𝓍/./ḟïľē.şḟ𝓍", ".şḟ𝓍", OK, 0},
        {"ⓓïȓ.şḟ𝓍/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK, 0},
        {"ⓓïȓ.şḟ𝓍/./ḟïľē.ş", ".ş", OK, 0},
        {"ⓓïȓ.şḟ𝓍/./ḟïľē.", ".", OK, 0},
        {"ⓓïȓ.şḟ𝓍/./ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"ⓓïȓ.şḟ𝓍/./.ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"/./dir.sfx/./file.sfx", ".sfx", OK, 0},
        {"/./dir.sfx/./file.very-long-suffix", ".very-long-suffix", OK, 0},
        {"/./dir.sfx/./file.s", ".s", OK, 0},
        {"/./dir.sfx/./file.", ".", OK, 0},
        {"/./dir.sfx/./file", "<n/a>", ERR_SUFFIX, 0},
        {"/./dir.sfx/./.file", "<n/a>", ERR_SUFFIX, 0},
        {"/./ⓓïȓ.şḟ𝓍/./ḟïľē.şḟ𝓍", ".şḟ𝓍", OK, 0},
        {"/./ⓓïȓ.şḟ𝓍/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK, 0},
        {"/./ⓓïȓ.şḟ𝓍/./ḟïľē.ş", ".ş", OK, 0},
        {"/./ⓓïȓ.şḟ𝓍/./ḟïľē.", ".", OK, 0},
        {"/./ⓓïȓ.şḟ𝓍/./ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"/./ⓓïȓ.şḟ𝓍/./.ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"././dir.sfx/./file.sfx", ".sfx", OK, 0},
        {"././dir.sfx/./file.very-long-suffix", ".very-long-suffix", OK, 0},
        {"././dir.sfx/./file.s", ".s", OK, 0},
        {"././dir.sfx/./file.", ".", OK, 0},
        {"././dir.sfx/./file", "<n/a>", ERR_SUFFIX, 0},
        {"././dir.sfx/./.file", "<n/a>", ERR_SUFFIX, 0},
        {"././ⓓïȓ.şḟ𝓍/./ḟïľē.şḟ𝓍", ".şḟ𝓍", OK, 0},
        {"././ⓓïȓ.şḟ𝓍/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK, 0},
        {"././ⓓïȓ.şḟ𝓍/./ḟïľē.ş", ".ş", OK, 0},
        {"././ⓓïȓ.şḟ𝓍/./ḟïľē.", ".", OK, 0},
        {"././ⓓïȓ.şḟ𝓍/./ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"././ⓓïȓ.şḟ𝓍/./.ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {".././dir.sfx/./file.sfx", ".sfx", OK, 0},
        {".././dir.sfx/./file.very-long-suffix", ".very-long-suffix", OK, 0},
        {".././dir.sfx/./file.s", ".s", OK, 0},
        {".././dir.sfx/./file.", ".", OK, 0},
        {".././dir.sfx/./file", "<n/a>", ERR_SUFFIX, 0},
        {".././dir.sfx/./.file", "<n/a>", ERR_SUFFIX, 0},
        {".././ⓓïȓ.şḟ𝓍/./ḟïľē.şḟ𝓍", ".şḟ𝓍", OK, 0},
        {".././ⓓïȓ.şḟ𝓍/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK, 0},
        {".././ⓓïȓ.şḟ𝓍/./ḟïľē.ş", ".ş", OK, 0},
        {".././ⓓïȓ.şḟ𝓍/./ḟïľē.", ".", OK, 0},
        {".././ⓓïȓ.şḟ𝓍/./ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {".././ⓓïȓ.şḟ𝓍/./.ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"/../dir/../file.sfx", ".sfx", OK, 0},
        {"/../dir/../file.very-long-suffix", ".very-long-suffix", OK, 0},
        {"/../dir/../file.s", ".s", OK, 0},
        {"/../dir/../file.", ".", OK, 0},
        {"/../dir/../file", "<n/a>", ERR_SUFFIX, 0},
        {"/../dir/../.file", "<n/a>", ERR_SUFFIX, 0},
        {"/../ⓓïȓ/../ḟïľē.şḟ𝓍", ".şḟ𝓍", OK, 0},
        {"/../ⓓïȓ/../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK, 0},
        {"/../ⓓïȓ/../ḟïľē.ş", ".ş", OK, 0},
        {"/../ⓓïȓ/../ḟïľē.", ".", OK, 0},
        {"/../ⓓïȓ/../ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"/../ⓓïȓ/../.ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"./../dir/../file.sfx", ".sfx", OK, 0},
        {"./../dir/../file.very-long-suffix", ".very-long-suffix", OK, 0},
        {"./../dir/../file.s", ".s", OK, 0},
        {"./../dir/../file.", ".", OK, 0},
        {"./../dir/../file", "<n/a>", ERR_SUFFIX, 0},
        {"./../dir/../.file", "<n/a>", ERR_SUFFIX, 0},
        {"./../ⓓïȓ/../ḟïľē.şḟ𝓍", ".şḟ𝓍", OK, 0},
        {"./../ⓓïȓ/../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK, 0},
        {"./../ⓓïȓ/../ḟïľē.ş", ".ş", OK, 0},
        {"./../ⓓïȓ/../ḟïľē.", ".", OK, 0},
        {"./../ⓓïȓ/../ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"./../ⓓïȓ/../.ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"../../dir/../file.sfx", ".sfx", OK, 0},
        {"../../dir/../file.very-long-suffix", ".very-long-suffix", OK, 0},
        {"../../dir/../file.s", ".s", OK, 0},
        {"../../dir/../file.", ".", OK, 0},
        {"../../dir/../file", "<n/a>", ERR_SUFFIX, 0},
        {"../../dir/../.file", "<n/a>", ERR_SUFFIX, 0},
        {"../../ⓓïȓ/../ḟïľē.şḟ𝓍", ".şḟ𝓍", OK, 0},
        {"../../ⓓïȓ/../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK, 0},
        {"../../ⓓïȓ/../ḟïľē.ş", ".ş", OK, 0},
        {"../../ⓓïȓ/../ḟïľē.", ".", OK, 0},
        {"../../ⓓïȓ/../ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"../../ⓓïȓ/../.ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"dir.sfx/../file.sfx", ".sfx", OK, 0},
        {"dir.sfx/../file.very-long-suffix", ".very-long-suffix", OK, 0},
        {"dir.sfx/../file.s", ".s", OK, 0},
        {"dir.sfx/../file.", ".", OK, 0},
        {"dir.sfx/../file", "<n/a>", ERR_SUFFIX, 0},
        {"dir.sfx/../.file", "<n/a>", ERR_SUFFIX, 0},
        {"ⓓïȓ.şḟ𝓍/../ḟïľē.şḟ𝓍", ".şḟ𝓍", OK, 0},
        {"ⓓïȓ.şḟ𝓍/../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK, 0},
        {"ⓓïȓ.şḟ𝓍/../ḟïľē.ş", ".ş", OK, 0},
        {"ⓓïȓ.şḟ𝓍/../ḟïľē.", ".", OK, 0},
        {"ⓓïȓ.şḟ𝓍/../ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"ⓓïȓ.şḟ𝓍/../.ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"/../dir.sfx/../file.sfx", ".sfx", OK, 0},
        {"/../dir.sfx/../file.very-long-suffix", ".very-long-suffix", OK, 0},
        {"/../dir.sfx/../file.s", ".s", OK, 0},
        {"/../dir.sfx/../file.", ".", OK, 0},
        {"/../dir.sfx/../file", "<n/a>", ERR_SUFFIX, 0},
        {"/../dir.sfx/../.file", "<n/a>", ERR_SUFFIX, 0},
        {"/../ⓓïȓ.şḟ𝓍/../ḟïľē.şḟ𝓍", ".şḟ𝓍", OK, 0},
        {"/../ⓓïȓ.şḟ𝓍/../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK, 0},
        {"/../ⓓïȓ.şḟ𝓍/../ḟïľē.ş", ".ş", OK, 0},
        {"/../ⓓïȓ.şḟ𝓍/../ḟïľē.", ".", OK, 0},
        {"/../ⓓïȓ.şḟ𝓍/../ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"/../ⓓïȓ.şḟ𝓍/../.ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"./../dir.sfx/../file.sfx", ".sfx", OK, 0},
        {"./../dir.sfx/../file.very-long-suffix", ".very-long-suffix", OK, 0},
        {"./../dir.sfx/../file.s", ".s", OK, 0},
        {"./../dir.sfx/../file.", ".", OK, 0},
        {"./../dir.sfx/../file", "<n/a>", ERR_SUFFIX, 0},
        {"./../dir.sfx/../.file", "<n/a>", ERR_SUFFIX, 0},
        {"./../ⓓïȓ.şḟ𝓍/../ḟïľē.şḟ𝓍", ".şḟ𝓍", OK, 0},
        {"./../ⓓïȓ.şḟ𝓍/../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK, 0},
        {"./../ⓓïȓ.şḟ𝓍/../ḟïľē.ş", ".ş", OK, 0},
        {"./../ⓓïȓ.şḟ𝓍/../ḟïľē.", ".", OK, 0},
        {"./../ⓓïȓ.şḟ𝓍/../ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"./../ⓓïȓ.şḟ𝓍/../.ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"../../dir.sfx/../file.sfx", ".sfx", OK, 0},
        {"../../dir.sfx/../file.very-long-suffix", ".very-long-suffix", OK, 0},
        {"../../dir.sfx/../file.s", ".s", OK, 0},
        {"../../dir.sfx/../file.", ".", OK, 0},
        {"../../dir.sfx/../file", "<n/a>", ERR_SUFFIX, 0},
        {"../../dir.sfx/../.file", "<n/a>", ERR_SUFFIX, 0},
        {"../../ⓓïȓ.şḟ𝓍/../ḟïľē.şḟ𝓍", ".şḟ𝓍", OK, 0},
        {"../../ⓓïȓ.şḟ𝓍/../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK, 0},
        {"../../ⓓïȓ.şḟ𝓍/../ḟïľē.ş", ".ş", OK, 0},
        {"../../ⓓïȓ.şḟ𝓍/../ḟïľē.", ".", OK, 0},
        {"../../ⓓïȓ.şḟ𝓍/../ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {"../../ⓓïȓ.şḟ𝓍/../.ḟïľē", "<n/a>", ERR_SUFFIX, 0},
        {".///", "<n/a>", ERR_SUFFIX, 0},
        {"////", "<n/a>", ERR_SUFFIX, 0},
        {"file.sfx///", ".sfx///", OK, 0},
        {"file.very-long-suffix///", ".very-long-suffix///", OK, 0},
        {"file.s///", ".s///", OK, 0},
        {"file.///", ".///", OK, 0},
        {"file///", "<n/a>", ERR_SUFFIX, 0},
        {".file///", "<n/a>", ERR_SUFFIX, 0},
        {"ḟïľē.şḟ𝓍///", ".şḟ𝓍///", OK, 0},
        {"ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK, 0},
        {"ḟïľē.ş///", ".ş///", OK, 0},
        {"ḟïľē.///", ".///", OK, 0},
        {"ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {".ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"/file.sfx///", ".sfx///", OK, 0},
        {"/file.very-long-suffix///", ".very-long-suffix///", OK, 0},
        {"/file.s///", ".s///", OK, 0},
        {"/file.///", ".///", OK, 0},
        {"/file///", "<n/a>", ERR_SUFFIX, 0},
        {"/.file///", "<n/a>", ERR_SUFFIX, 0},
        {"/ḟïľē.şḟ𝓍///", ".şḟ𝓍///", OK, 0},
        {"/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK, 0},
        {"/ḟïľē.ş///", ".ş///", OK, 0},
        {"/ḟïľē.///", ".///", OK, 0},
        {"/ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"/.ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"./file.sfx///", ".sfx///", OK, 0},
        {"./file.very-long-suffix///", ".very-long-suffix///", OK, 0},
        {"./file.s///", ".s///", OK, 0},
        {"./file.///", ".///", OK, 0},
        {"./file///", "<n/a>", ERR_SUFFIX, 0},
        {"./.file///", "<n/a>", ERR_SUFFIX, 0},
        {"./ḟïľē.şḟ𝓍///", ".şḟ𝓍///", OK, 0},
        {"./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK, 0},
        {"./ḟïľē.ş///", ".ş///", OK, 0},
        {"./ḟïľē.///", ".///", OK, 0},
        {"./ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"./.ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"../file.sfx///", ".sfx///", OK, 0},
        {"../file.very-long-suffix///", ".very-long-suffix///", OK, 0},
        {"../file.s///", ".s///", OK, 0},
        {"../file.///", ".///", OK, 0},
        {"../file///", "<n/a>", ERR_SUFFIX, 0},
        {"../.file///", "<n/a>", ERR_SUFFIX, 0},
        {"../ḟïľē.şḟ𝓍///", ".şḟ𝓍///", OK, 0},
        {"../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK, 0},
        {"../ḟïľē.ş///", ".ş///", OK, 0},
        {"../ḟïľē.///", ".///", OK, 0},
        {"../ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"../.ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"dir/file.sfx///", ".sfx///", OK, 0},
        {"dir/file.very-long-suffix///", ".very-long-suffix///", OK, 0},
        {"dir/file.s///", ".s///", OK, 0},
        {"dir/file.///", ".///", OK, 0},
        {"dir/file///", "<n/a>", ERR_SUFFIX, 0},
        {"dir/.file///", "<n/a>", ERR_SUFFIX, 0},
        {"ⓓïȓ/ḟïľē.şḟ𝓍///", ".şḟ𝓍///", OK, 0},
        {"ⓓïȓ/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK, 0},
        {"ⓓïȓ/ḟïľē.ş///", ".ş///", OK, 0},
        {"ⓓïȓ/ḟïľē.///", ".///", OK, 0},
        {"ⓓïȓ/ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"ⓓïȓ/.ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"/dir/file.sfx///", ".sfx///", OK, 0},
        {"/dir/file.very-long-suffix///", ".very-long-suffix///", OK, 0},
        {"/dir/file.s///", ".s///", OK, 0},
        {"/dir/file.///", ".///", OK, 0},
        {"/dir/file///", "<n/a>", ERR_SUFFIX, 0},
        {"/dir/.file///", "<n/a>", ERR_SUFFIX, 0},
        {"/ⓓïȓ/ḟïľē.şḟ𝓍///", ".şḟ𝓍///", OK, 0},
        {"/ⓓïȓ/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK, 0},
        {"/ⓓïȓ/ḟïľē.ş///", ".ş///", OK, 0},
        {"/ⓓïȓ/ḟïľē.///", ".///", OK, 0},
        {"/ⓓïȓ/ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"/ⓓïȓ/.ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"./dir/file.sfx///", ".sfx///", OK, 0},
        {"./dir/file.very-long-suffix///", ".very-long-suffix///", OK, 0},
        {"./dir/file.s///", ".s///", OK, 0},
        {"./dir/file.///", ".///", OK, 0},
        {"./dir/file///", "<n/a>", ERR_SUFFIX, 0},
        {"./dir/.file///", "<n/a>", ERR_SUFFIX, 0},
        {"./ⓓïȓ/ḟïľē.şḟ𝓍///", ".şḟ𝓍///", OK, 0},
        {"./ⓓïȓ/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK, 0},
        {"./ⓓïȓ/ḟïľē.ş///", ".ş///", OK, 0},
        {"./ⓓïȓ/ḟïľē.///", ".///", OK, 0},
        {"./ⓓïȓ/ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"./ⓓïȓ/.ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"../dir/file.sfx///", ".sfx///", OK, 0},
        {"../dir/file.very-long-suffix///", ".very-long-suffix///", OK, 0},
        {"../dir/file.s///", ".s///", OK, 0},
        {"../dir/file.///", ".///", OK, 0},
        {"../dir/file///", "<n/a>", ERR_SUFFIX, 0},
        {"../dir/.file///", "<n/a>", ERR_SUFFIX, 0},
        {"../ⓓïȓ/ḟïľē.şḟ𝓍///", ".şḟ𝓍///", OK, 0},
        {"../ⓓïȓ/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK, 0},
        {"../ⓓïȓ/ḟïľē.ş///", ".ş///", OK, 0},
        {"../ⓓïȓ/ḟïľē.///", ".///", OK, 0},
        {"../ⓓïȓ/ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"../ⓓïȓ/.ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"dir.sfx/file.sfx///", ".sfx///", OK, 0},
        {"dir.sfx/file.very-long-suffix///", ".very-long-suffix///", OK, 0},
        {"dir.sfx/file.s///", ".s///", OK, 0},
        {"dir.sfx/file.///", ".///", OK, 0},
        {"dir.sfx/file///", "<n/a>", ERR_SUFFIX, 0},
        {"dir.sfx/.file///", "<n/a>", ERR_SUFFIX, 0},
        {"ⓓïȓ.şḟ𝓍/ḟïľē.şḟ𝓍///", ".şḟ𝓍///", OK, 0},
        {"ⓓïȓ.şḟ𝓍/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK, 0},
        {"ⓓïȓ.şḟ𝓍/ḟïľē.ş///", ".ş///", OK, 0},
        {"ⓓïȓ.şḟ𝓍/ḟïľē.///", ".///", OK, 0},
        {"ⓓïȓ.şḟ𝓍/ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"ⓓïȓ.şḟ𝓍/.ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"/dir.sfx/file.sfx///", ".sfx///", OK, 0},
        {"/dir.sfx/file.very-long-suffix///", ".very-long-suffix///", OK, 0},
        {"/dir.sfx/file.s///", ".s///", OK, 0},
        {"/dir.sfx/file.///", ".///", OK, 0},
        {"/dir.sfx/file///", "<n/a>", ERR_SUFFIX, 0},
        {"/dir.sfx/.file///", "<n/a>", ERR_SUFFIX, 0},
        {"/ⓓïȓ.şḟ𝓍/ḟïľē.şḟ𝓍///", ".şḟ𝓍///", OK, 0},
        {"/ⓓïȓ.şḟ𝓍/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK, 0},
        {"/ⓓïȓ.şḟ𝓍/ḟïľē.ş///", ".ş///", OK, 0},
        {"/ⓓïȓ.şḟ𝓍/ḟïľē.///", ".///", OK, 0},
        {"/ⓓïȓ.şḟ𝓍/ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"/ⓓïȓ.şḟ𝓍/.ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"./dir.sfx/file.sfx///", ".sfx///", OK, 0},
        {"./dir.sfx/file.very-long-suffix///", ".very-long-suffix///", OK, 0},
        {"./dir.sfx/file.s///", ".s///", OK, 0},
        {"./dir.sfx/file.///", ".///", OK, 0},
        {"./dir.sfx/file///", "<n/a>", ERR_SUFFIX, 0},
        {"./dir.sfx/.file///", "<n/a>", ERR_SUFFIX, 0},
        {"./ⓓïȓ.şḟ𝓍/ḟïľē.şḟ𝓍///", ".şḟ𝓍///", OK, 0},
        {"./ⓓïȓ.şḟ𝓍/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK, 0},
        {"./ⓓïȓ.şḟ𝓍/ḟïľē.ş///", ".ş///", OK, 0},
        {"./ⓓïȓ.şḟ𝓍/ḟïľē.///", ".///", OK, 0},
        {"./ⓓïȓ.şḟ𝓍/ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"./ⓓïȓ.şḟ𝓍/.ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"../dir.sfx/file.sfx///", ".sfx///", OK, 0},
        {"../dir.sfx/file.very-long-suffix///", ".very-long-suffix///", OK, 0},
        {"../dir.sfx/file.s///", ".s///", OK, 0},
        {"../dir.sfx/file.///", ".///", OK, 0},
        {"../dir.sfx/file///", "<n/a>", ERR_SUFFIX, 0},
        {"../dir.sfx/.file///", "<n/a>", ERR_SUFFIX, 0},
        {"../ⓓïȓ.şḟ𝓍/ḟïľē.şḟ𝓍///", ".şḟ𝓍///", OK, 0},
        {"../ⓓïȓ.şḟ𝓍/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK, 0},
        {"../ⓓïȓ.şḟ𝓍/ḟïľē.ş///", ".ş///", OK, 0},
        {"../ⓓïȓ.şḟ𝓍/ḟïľē.///", ".///", OK, 0},
        {"../ⓓïȓ.şḟ𝓍/ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"../ⓓïȓ.şḟ𝓍/.ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"dir/./file.sfx///", ".sfx///", OK, 0},
        {"dir/./file.very-long-suffix///", ".very-long-suffix///", OK, 0},
        {"dir/./file.s///", ".s///", OK, 0},
        {"dir/./file.///", ".///", OK, 0},
        {"dir/./file///", "<n/a>", ERR_SUFFIX, 0},
        {"dir/./.file///", "<n/a>", ERR_SUFFIX, 0},
        {"ⓓïȓ/./ḟïľē.şḟ𝓍///", ".şḟ𝓍///", OK, 0},
        {"ⓓïȓ/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK, 0},
        {"ⓓïȓ/./ḟïľē.ş///", ".ş///", OK, 0},
        {"ⓓïȓ/./ḟïľē.///", ".///", OK, 0},
        {"ⓓïȓ/./ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"ⓓïȓ/./.ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"/./dir/./file.sfx///", ".sfx///", OK, 0},
        {"/./dir/./file.very-long-suffix///", ".very-long-suffix///", OK, 0},
        {"/./dir/./file.s///", ".s///", OK, 0},
        {"/./dir/./file.///", ".///", OK, 0},
        {"/./dir/./file///", "<n/a>", ERR_SUFFIX, 0},
        {"/./dir/./.file///", "<n/a>", ERR_SUFFIX, 0},
        {"/./ⓓïȓ/./ḟïľē.şḟ𝓍///", ".şḟ𝓍///", OK, 0},
        {"/./ⓓïȓ/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK, 0},
        {"/./ⓓïȓ/./ḟïľē.ş///", ".ş///", OK, 0},
        {"/./ⓓïȓ/./ḟïľē.///", ".///", OK, 0},
        {"/./ⓓïȓ/./ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"/./ⓓïȓ/./.ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"././dir/./file.sfx///", ".sfx///", OK, 0},
        {"././dir/./file.very-long-suffix///", ".very-long-suffix///", OK, 0},
        {"././dir/./file.s///", ".s///", OK, 0},
        {"././dir/./file.///", ".///", OK, 0},
        {"././dir/./file///", "<n/a>", ERR_SUFFIX, 0},
        {"././dir/./.file///", "<n/a>", ERR_SUFFIX, 0},
        {"././ⓓïȓ/./ḟïľē.şḟ𝓍///", ".şḟ𝓍///", OK, 0},
        {"././ⓓïȓ/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK, 0},
        {"././ⓓïȓ/./ḟïľē.ş///", ".ş///", OK, 0},
        {"././ⓓïȓ/./ḟïľē.///", ".///", OK, 0},
        {"././ⓓïȓ/./ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"././ⓓïȓ/./.ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {".././dir/./file.sfx///", ".sfx///", OK, 0},
        {".././dir/./file.very-long-suffix///", ".very-long-suffix///", OK, 0},
        {".././dir/./file.s///", ".s///", OK, 0},
        {".././dir/./file.///", ".///", OK, 0},
        {".././dir/./file///", "<n/a>", ERR_SUFFIX, 0},
        {".././dir/./.file///", "<n/a>", ERR_SUFFIX, 0},
        {".././ⓓïȓ/./ḟïľē.şḟ𝓍///", ".şḟ𝓍///", OK, 0},
        {".././ⓓïȓ/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK, 0},
        {".././ⓓïȓ/./ḟïľē.ş///", ".ş///", OK, 0},
        {".././ⓓïȓ/./ḟïľē.///", ".///", OK, 0},
        {".././ⓓïȓ/./ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {".././ⓓïȓ/./.ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"dir.sfx/./file.sfx///", ".sfx///", OK, 0},
        {"dir.sfx/./file.very-long-suffix///", ".very-long-suffix///", OK, 0},
        {"dir.sfx/./file.s///", ".s///", OK, 0},
        {"dir.sfx/./file.///", ".///", OK, 0},
        {"dir.sfx/./file///", "<n/a>", ERR_SUFFIX, 0},
        {"dir.sfx/./.file///", "<n/a>", ERR_SUFFIX, 0},
        {"ⓓïȓ.şḟ𝓍/./ḟïľē.şḟ𝓍///", ".şḟ𝓍///", OK, 0},
        {"ⓓïȓ.şḟ𝓍/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK, 0},
        {"ⓓïȓ.şḟ𝓍/./ḟïľē.ş///", ".ş///", OK, 0},
        {"ⓓïȓ.şḟ𝓍/./ḟïľē.///", ".///", OK, 0},
        {"ⓓïȓ.şḟ𝓍/./ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"ⓓïȓ.şḟ𝓍/./.ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"/./dir.sfx/./file.sfx///", ".sfx///", OK, 0},
        {"/./dir.sfx/./file.very-long-suffix///", ".very-long-suffix///", OK, 0},
        {"/./dir.sfx/./file.s///", ".s///", OK, 0},
        {"/./dir.sfx/./file.///", ".///", OK, 0},
        {"/./dir.sfx/./file///", "<n/a>", ERR_SUFFIX, 0},
        {"/./dir.sfx/./.file///", "<n/a>", ERR_SUFFIX, 0},
        {"/./ⓓïȓ.şḟ𝓍/./ḟïľē.şḟ𝓍///", ".şḟ𝓍///", OK, 0},
        {"/./ⓓïȓ.şḟ𝓍/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK, 0},
        {"/./ⓓïȓ.şḟ𝓍/./ḟïľē.ş///", ".ş///", OK, 0},
        {"/./ⓓïȓ.şḟ𝓍/./ḟïľē.///", ".///", OK, 0},
        {"/./ⓓïȓ.şḟ𝓍/./ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"/./ⓓïȓ.şḟ𝓍/./.ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"././dir.sfx/./file.sfx///", ".sfx///", OK, 0},
        {"././dir.sfx/./file.very-long-suffix///", ".very-long-suffix///", OK, 0},
        {"././dir.sfx/./file.s///", ".s///", OK, 0},
        {"././dir.sfx/./file.///", ".///", OK, 0},
        {"././dir.sfx/./file///", "<n/a>", ERR_SUFFIX, 0},
        {"././dir.sfx/./.file///", "<n/a>", ERR_SUFFIX, 0},
        {"././ⓓïȓ.şḟ𝓍/./ḟïľē.şḟ𝓍///", ".şḟ𝓍///", OK, 0},
        {"././ⓓïȓ.şḟ𝓍/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK, 0},
        {"././ⓓïȓ.şḟ𝓍/./ḟïľē.ş///", ".ş///", OK, 0},
        {"././ⓓïȓ.şḟ𝓍/./ḟïľē.///", ".///", OK, 0},
        {"././ⓓïȓ.şḟ𝓍/./ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"././ⓓïȓ.şḟ𝓍/./.ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {".././dir.sfx/./file.sfx///", ".sfx///", OK, 0},
        {".././dir.sfx/./file.very-long-suffix///", ".very-long-suffix///", OK, 0},
        {".././dir.sfx/./file.s///", ".s///", OK, 0},
        {".././dir.sfx/./file.///", ".///", OK, 0},
        {".././dir.sfx/./file///", "<n/a>", ERR_SUFFIX, 0},
        {".././dir.sfx/./.file///", "<n/a>", ERR_SUFFIX, 0},
        {".././ⓓïȓ.şḟ𝓍/./ḟïľē.şḟ𝓍///", ".şḟ𝓍///", OK, 0},
        {".././ⓓïȓ.şḟ𝓍/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK, 0},
        {".././ⓓïȓ.şḟ𝓍/./ḟïľē.ş///", ".ş///", OK, 0},
        {".././ⓓïȓ.şḟ𝓍/./ḟïľē.///", ".///", OK, 0},
        {".././ⓓïȓ.şḟ𝓍/./ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {".././ⓓïȓ.şḟ𝓍/./.ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"/../dir/../file.sfx///", ".sfx///", OK, 0},
        {"/../dir/../file.very-long-suffix///", ".very-long-suffix///", OK, 0},
        {"/../dir/../file.s///", ".s///", OK, 0},
        {"/../dir/../file.///", ".///", OK, 0},
        {"/../dir/../file///", "<n/a>", ERR_SUFFIX, 0},
        {"/../dir/../.file///", "<n/a>", ERR_SUFFIX, 0},
        {"/../ⓓïȓ/../ḟïľē.şḟ𝓍///", ".şḟ𝓍///", OK, 0},
        {"/../ⓓïȓ/../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK, 0},
        {"/../ⓓïȓ/../ḟïľē.ş///", ".ş///", OK, 0},
        {"/../ⓓïȓ/../ḟïľē.///", ".///", OK, 0},
        {"/../ⓓïȓ/../ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"/../ⓓïȓ/../.ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"./../dir/../file.sfx///", ".sfx///", OK, 0},
        {"./../dir/../file.very-long-suffix///", ".very-long-suffix///", OK, 0},
        {"./../dir/../file.s///", ".s///", OK, 0},
        {"./../dir/../file.///", ".///", OK, 0},
        {"./../dir/../file///", "<n/a>", ERR_SUFFIX, 0},
        {"./../dir/../.file///", "<n/a>", ERR_SUFFIX, 0},
        {"./../ⓓïȓ/../ḟïľē.şḟ𝓍///", ".şḟ𝓍///", OK, 0},
        {"./../ⓓïȓ/../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK, 0},
        {"./../ⓓïȓ/../ḟïľē.ş///", ".ş///", OK, 0},
        {"./../ⓓïȓ/../ḟïľē.///", ".///", OK, 0},
        {"./../ⓓïȓ/../ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"./../ⓓïȓ/../.ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"../../dir/../file.sfx///", ".sfx///", OK, 0},
        {"../../dir/../file.very-long-suffix///", ".very-long-suffix///", OK, 0},
        {"../../dir/../file.s///", ".s///", OK, 0},
        {"../../dir/../file.///", ".///", OK, 0},
        {"../../dir/../file///", "<n/a>", ERR_SUFFIX, 0},
        {"../../dir/../.file///", "<n/a>", ERR_SUFFIX, 0},
        {"../../ⓓïȓ/../ḟïľē.şḟ𝓍///", ".şḟ𝓍///", OK, 0},
        {"../../ⓓïȓ/../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK, 0},
        {"../../ⓓïȓ/../ḟïľē.ş///", ".ş///", OK, 0},
        {"../../ⓓïȓ/../ḟïľē.///", ".///", OK, 0},
        {"../../ⓓïȓ/../ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"../../ⓓïȓ/../.ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"dir.sfx/../file.sfx///", ".sfx///", OK, 0},
        {"dir.sfx/../file.very-long-suffix///", ".very-long-suffix///", OK, 0},
        {"dir.sfx/../file.s///", ".s///", OK, 0},
        {"dir.sfx/../file.///", ".///", OK, 0},
        {"dir.sfx/../file///", "<n/a>", ERR_SUFFIX, 0},
        {"dir.sfx/../.file///", "<n/a>", ERR_SUFFIX, 0},
        {"ⓓïȓ.şḟ𝓍/../ḟïľē.şḟ𝓍///", ".şḟ𝓍///", OK, 0},
        {"ⓓïȓ.şḟ𝓍/../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK, 0},
        {"ⓓïȓ.şḟ𝓍/../ḟïľē.ş///", ".ş///", OK, 0},
        {"ⓓïȓ.şḟ𝓍/../ḟïľē.///", ".///", OK, 0},
        {"ⓓïȓ.şḟ𝓍/../ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"ⓓïȓ.şḟ𝓍/../.ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"/../dir.sfx/../file.sfx///", ".sfx///", OK, 0},
        {"/../dir.sfx/../file.very-long-suffix///", ".very-long-suffix///", OK, 0},
        {"/../dir.sfx/../file.s///", ".s///", OK, 0},
        {"/../dir.sfx/../file.///", ".///", OK, 0},
        {"/../dir.sfx/../file///", "<n/a>", ERR_SUFFIX, 0},
        {"/../dir.sfx/../.file///", "<n/a>", ERR_SUFFIX, 0},
        {"/../ⓓïȓ.şḟ𝓍/../ḟïľē.şḟ𝓍///", ".şḟ𝓍///", OK, 0},
        {"/../ⓓïȓ.şḟ𝓍/../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK, 0},
        {"/../ⓓïȓ.şḟ𝓍/../ḟïľē.ş///", ".ş///", OK, 0},
        {"/../ⓓïȓ.şḟ𝓍/../ḟïľē.///", ".///", OK, 0},
        {"/../ⓓïȓ.şḟ𝓍/../ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"/../ⓓïȓ.şḟ𝓍/../.ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"./../dir.sfx/../file.sfx///", ".sfx///", OK, 0},
        {"./../dir.sfx/../file.very-long-suffix///", ".very-long-suffix///", OK, 0},
        {"./../dir.sfx/../file.s///", ".s///", OK, 0},
        {"./../dir.sfx/../file.///", ".///", OK, 0},
        {"./../dir.sfx/../file///", "<n/a>", ERR_SUFFIX, 0},
        {"./../dir.sfx/../.file///", "<n/a>", ERR_SUFFIX, 0},
        {"./../ⓓïȓ.şḟ𝓍/../ḟïľē.şḟ𝓍///", ".şḟ𝓍///", OK, 0},
        {"./../ⓓïȓ.şḟ𝓍/../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK, 0},
        {"./../ⓓïȓ.şḟ𝓍/../ḟïľē.ş///", ".ş///", OK, 0},
        {"./../ⓓïȓ.şḟ𝓍/../ḟïľē.///", ".///", OK, 0},
        {"./../ⓓïȓ.şḟ𝓍/../ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"./../ⓓïȓ.şḟ𝓍/../.ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"../../dir.sfx/../file.sfx///", ".sfx///", OK, 0},
        {"../../dir.sfx/../file.very-long-suffix///", ".very-long-suffix///", OK, 0},
        {"../../dir.sfx/../file.s///", ".s///", OK, 0},
        {"../../dir.sfx/../file.///", ".///", OK, 0},
        {"../../dir.sfx/../file///", "<n/a>", ERR_SUFFIX, 0},
        {"../../dir.sfx/../.file///", "<n/a>", ERR_SUFFIX, 0},
        {"../../ⓓïȓ.şḟ𝓍/../ḟïľē.şḟ𝓍///", ".şḟ𝓍///", OK, 0},
        {"../../ⓓïȓ.şḟ𝓍/../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK, 0},
        {"../../ⓓïȓ.şḟ𝓍/../ḟïľē.ş///", ".ş///", OK, 0},
        {"../../ⓓïȓ.şḟ𝓍/../ḟïľē.///", ".///", OK, 0},
        {"../../ⓓïȓ.şḟ𝓍/../ḟïľē///", "<n/a>", ERR_SUFFIX, 0},
        {"../../ⓓïȓ.şḟ𝓍/../.ḟïľē///", "<n/a>", ERR_SUFFIX, 0}
    };

    volatile int result = PASS;

    for (volatile size_t i = 0; i < NELEMS(cases); ++i) {
        const PathSuffixArgs args = cases[i];

        if (sigsetjmp(abort_env, 1) == 0) {
            if (args.signal != 0) {
                warnx("the next test should fail an assertion.");
            }

            const char *suffix = NULL;

            (void) abort_catch(err);
            const Error retval = path_suffix(args.fname, &suffix);
            (void) abort_reset(err);

            if (retval != args.retval) {
                result = FAIL;
                warnx("(%s, → %s) → %u [!]",
                      args.fname, suffix, retval);
            }

            if (retval == OK &&
                strncmp(args.suffix, suffix, MAX_SUFFIX_LEN) != 0)
            {
                result = FAIL;
                warnx("(%s, → %s [!]) → %u",
                      args.fname, suffix, retval);
            }
        }

        if (abort_signal != args.signal) {
            result = FAIL;
            warnx("(%s, → <suffix>) ↑ %s [!]",
                  args.fname, strsignal(abort_signal));
        }
    }

    return result;
}
