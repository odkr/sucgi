/*
 * Test path_suffix.
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
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../macros.h"
#include "../max.h"
#include "../path.h"
#include "lib.h"


/*
 * Constants
 */

/* Maximum length of dynamically created pre- and suffixes. */
#define FIX_LEN 2U

/* Maximum length of dynamically created filenames. */
#define FNAME_LEN (FIX_LEN * 2U + 1U)


/*
 * Data types
 */

/* Mapping of filenames to return values. */
typedef struct {
    const char *const fname;
    const char *const suffix;
    const Error ret;
} Args;


/*
 * Module variables
 */

/* Test cases. */
static const Args cases[] = {
    {".", NULL, ERR_NO_SUFFIX},
    {"/", NULL, ERR_NO_SUFFIX},
    {"file.suffix", ".suffix", OK},
    {"file.very-long-suffix", ".very-long-suffix", OK},
    {"file.s", ".s", OK},
    {"file.", ".", OK},
    {"file", NULL, ERR_NO_SUFFIX},
    {".file", NULL, ERR_NO_SUFFIX},
    {"ḟïľē.şüḟḟï𝓍", ".şüḟḟï𝓍", OK},
    {"ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK},
    {"ḟïľē.ş", ".ş", OK},
    {"ḟïľē.", ".", OK},
    {"ḟïľē", NULL, ERR_NO_SUFFIX},
    {".ḟïľē", NULL, ERR_NO_SUFFIX},
    {"/file.suffix", ".suffix", OK},
    {"/file.very-long-suffix", ".very-long-suffix", OK},
    {"/file.s", ".s", OK},
    {"/file.", ".", OK},
    {"/file", NULL, ERR_NO_SUFFIX},
    {"/.file", NULL, ERR_NO_SUFFIX},
    {"/ḟïľē.şüḟḟï𝓍", ".şüḟḟï𝓍", OK},
    {"/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK},
    {"/ḟïľē.ş", ".ş", OK},
    {"/ḟïľē.", ".", OK},
    {"/ḟïľē", NULL, ERR_NO_SUFFIX},
    {"/.ḟïľē", NULL, ERR_NO_SUFFIX},
    {"./file.suffix", ".suffix", OK},
    {"./file.very-long-suffix", ".very-long-suffix", OK},
    {"./file.s", ".s", OK},
    {"./file.", ".", OK},
    {"./file", NULL, ERR_NO_SUFFIX},
    {"./.file", NULL, ERR_NO_SUFFIX},
    {"./ḟïľē.şüḟḟï𝓍", ".şüḟḟï𝓍", OK},
    {"./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK},
    {"./ḟïľē.ş", ".ş", OK},
    {"./ḟïľē.", ".", OK},
    {"./ḟïľē", NULL, ERR_NO_SUFFIX},
    {"./.ḟïľē", NULL, ERR_NO_SUFFIX},
    {"../file.suffix", ".suffix", OK},
    {"../file.very-long-suffix", ".very-long-suffix", OK},
    {"../file.s", ".s", OK},
    {"../file.", ".", OK},
    {"../file", NULL, ERR_NO_SUFFIX},
    {"../.file", NULL, ERR_NO_SUFFIX},
    {"../ḟïľē.şüḟḟï𝓍", ".şüḟḟï𝓍", OK},
    {"../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK},
    {"../ḟïľē.ş", ".ş", OK},
    {"../ḟïľē.", ".", OK},
    {"../ḟïľē", NULL, ERR_NO_SUFFIX},
    {"../.ḟïľē", NULL, ERR_NO_SUFFIX},
    {"dir/file.suffix", ".suffix", OK},
    {"dir/file.very-long-suffix", ".very-long-suffix", OK},
    {"dir/file.s", ".s", OK},
    {"dir/file.", ".", OK},
    {"dir/file", NULL, ERR_NO_SUFFIX},
    {"dir/.file", NULL, ERR_NO_SUFFIX},
    {"ⓓïȓ/ḟïľē.şüḟḟï𝓍", ".şüḟḟï𝓍", OK},
    {"ⓓïȓ/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK},
    {"ⓓïȓ/ḟïľē.ş", ".ş", OK},
    {"ⓓïȓ/ḟïľē.", ".", OK},
    {"ⓓïȓ/ḟïľē", NULL, ERR_NO_SUFFIX},
    {"ⓓïȓ/.ḟïľē", NULL, ERR_NO_SUFFIX},
    {"/dir/file.suffix", ".suffix", OK},
    {"/dir/file.very-long-suffix", ".very-long-suffix", OK},
    {"/dir/file.s", ".s", OK},
    {"/dir/file.", ".", OK},
    {"/dir/file", NULL, ERR_NO_SUFFIX},
    {"/dir/.file", NULL, ERR_NO_SUFFIX},
    {"/ⓓïȓ/ḟïľē.şüḟḟï𝓍", ".şüḟḟï𝓍", OK},
    {"/ⓓïȓ/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK},
    {"/ⓓïȓ/ḟïľē.ş", ".ş", OK},
    {"/ⓓïȓ/ḟïľē.", ".", OK},
    {"/ⓓïȓ/ḟïľē", NULL, ERR_NO_SUFFIX},
    {"/ⓓïȓ/.ḟïľē", NULL, ERR_NO_SUFFIX},
    {"./dir/file.suffix", ".suffix", OK},
    {"./dir/file.very-long-suffix", ".very-long-suffix", OK},
    {"./dir/file.s", ".s", OK},
    {"./dir/file.", ".", OK},
    {"./dir/file", NULL, ERR_NO_SUFFIX},
    {"./dir/.file", NULL, ERR_NO_SUFFIX},
    {"./ⓓïȓ/ḟïľē.şüḟḟï𝓍", ".şüḟḟï𝓍", OK},
    {"./ⓓïȓ/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK},
    {"./ⓓïȓ/ḟïľē.ş", ".ş", OK},
    {"./ⓓïȓ/ḟïľē.", ".", OK},
    {"./ⓓïȓ/ḟïľē", NULL, ERR_NO_SUFFIX},
    {"./ⓓïȓ/.ḟïľē", NULL, ERR_NO_SUFFIX},
    {"../dir/file.suffix", ".suffix", OK},
    {"../dir/file.very-long-suffix", ".very-long-suffix", OK},
    {"../dir/file.s", ".s", OK},
    {"../dir/file.", ".", OK},
    {"../dir/file", NULL, ERR_NO_SUFFIX},
    {"../dir/.file", NULL, ERR_NO_SUFFIX},
    {"../ⓓïȓ/ḟïľē.şüḟḟï𝓍", ".şüḟḟï𝓍", OK},
    {"../ⓓïȓ/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK},
    {"../ⓓïȓ/ḟïľē.ş", ".ş", OK},
    {"../ⓓïȓ/ḟïľē.", ".", OK},
    {"../ⓓïȓ/ḟïľē", NULL, ERR_NO_SUFFIX},
    {"../ⓓïȓ/.ḟïľē", NULL, ERR_NO_SUFFIX},
    {"dir.suffix/file.suffix", ".suffix", OK},
    {"dir.suffix/file.very-long-suffix", ".very-long-suffix", OK},
    {"dir.suffix/file.s", ".s", OK},
    {"dir.suffix/file.", ".", OK},
    {"dir.suffix/file", NULL, ERR_NO_SUFFIX},
    {"dir.suffix/.file", NULL, ERR_NO_SUFFIX},
    {"ⓓïȓ.şüḟḟï𝓍/ḟïľē.şüḟḟï𝓍", ".şüḟḟï𝓍", OK},
    {"ⓓïȓ.şüḟḟï𝓍/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK},
    {"ⓓïȓ.şüḟḟï𝓍/ḟïľē.ş", ".ş", OK},
    {"ⓓïȓ.şüḟḟï𝓍/ḟïľē.", ".", OK},
    {"ⓓïȓ.şüḟḟï𝓍/ḟïľē", NULL, ERR_NO_SUFFIX},
    {"ⓓïȓ.şüḟḟï𝓍/.ḟïľē", NULL, ERR_NO_SUFFIX},
    {"/dir.suffix/file.suffix", ".suffix", OK},
    {"/dir.suffix/file.very-long-suffix", ".very-long-suffix", OK},
    {"/dir.suffix/file.s", ".s", OK},
    {"/dir.suffix/file.", ".", OK},
    {"/dir.suffix/file", NULL, ERR_NO_SUFFIX},
    {"/dir.suffix/.file", NULL, ERR_NO_SUFFIX},
    {"/ⓓïȓ.şüḟḟï𝓍/ḟïľē.şüḟḟï𝓍", ".şüḟḟï𝓍", OK},
    {"/ⓓïȓ.şüḟḟï𝓍/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK},
    {"/ⓓïȓ.şüḟḟï𝓍/ḟïľē.ş", ".ş", OK},
    {"/ⓓïȓ.şüḟḟï𝓍/ḟïľē.", ".", OK},
    {"/ⓓïȓ.şüḟḟï𝓍/ḟïľē", NULL, ERR_NO_SUFFIX},
    {"/ⓓïȓ.şüḟḟï𝓍/.ḟïľē", NULL, ERR_NO_SUFFIX},
    {"./dir.suffix/file.suffix", ".suffix", OK},
    {"./dir.suffix/file.very-long-suffix", ".very-long-suffix", OK},
    {"./dir.suffix/file.s", ".s", OK},
    {"./dir.suffix/file.", ".", OK},
    {"./dir.suffix/file", NULL, ERR_NO_SUFFIX},
    {"./dir.suffix/.file", NULL, ERR_NO_SUFFIX},
    {"./ⓓïȓ.şüḟḟï𝓍/ḟïľē.şüḟḟï𝓍", ".şüḟḟï𝓍", OK},
    {"./ⓓïȓ.şüḟḟï𝓍/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK},
    {"./ⓓïȓ.şüḟḟï𝓍/ḟïľē.ş", ".ş", OK},
    {"./ⓓïȓ.şüḟḟï𝓍/ḟïľē.", ".", OK},
    {"./ⓓïȓ.şüḟḟï𝓍/ḟïľē", NULL, ERR_NO_SUFFIX},
    {"./ⓓïȓ.şüḟḟï𝓍/.ḟïľē", NULL, ERR_NO_SUFFIX},
    {"../dir.suffix/file.suffix", ".suffix", OK},
    {"../dir.suffix/file.very-long-suffix", ".very-long-suffix", OK},
    {"../dir.suffix/file.s", ".s", OK},
    {"../dir.suffix/file.", ".", OK},
    {"../dir.suffix/file", NULL, ERR_NO_SUFFIX},
    {"../dir.suffix/.file", NULL, ERR_NO_SUFFIX},
    {"../ⓓïȓ.şüḟḟï𝓍/ḟïľē.şüḟḟï𝓍", ".şüḟḟï𝓍", OK},
    {"../ⓓïȓ.şüḟḟï𝓍/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK},
    {"../ⓓïȓ.şüḟḟï𝓍/ḟïľē.ş", ".ş", OK},
    {"../ⓓïȓ.şüḟḟï𝓍/ḟïľē.", ".", OK},
    {"../ⓓïȓ.şüḟḟï𝓍/ḟïľē", NULL, ERR_NO_SUFFIX},
    {"../ⓓïȓ.şüḟḟï𝓍/.ḟïľē", NULL, ERR_NO_SUFFIX},
    {"dir/./file.suffix", ".suffix", OK},
    {"dir/./file.very-long-suffix", ".very-long-suffix", OK},
    {"dir/./file.s", ".s", OK},
    {"dir/./file.", ".", OK},
    {"dir/./file", NULL, ERR_NO_SUFFIX},
    {"dir/./.file", NULL, ERR_NO_SUFFIX},
    {"ⓓïȓ/./ḟïľē.şüḟḟï𝓍", ".şüḟḟï𝓍", OK},
    {"ⓓïȓ/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK},
    {"ⓓïȓ/./ḟïľē.ş", ".ş", OK},
    {"ⓓïȓ/./ḟïľē.", ".", OK},
    {"ⓓïȓ/./ḟïľē", NULL, ERR_NO_SUFFIX},
    {"ⓓïȓ/./.ḟïľē", NULL, ERR_NO_SUFFIX},
    {"/./dir/./file.suffix", ".suffix", OK},
    {"/./dir/./file.very-long-suffix", ".very-long-suffix", OK},
    {"/./dir/./file.s", ".s", OK},
    {"/./dir/./file.", ".", OK},
    {"/./dir/./file", NULL, ERR_NO_SUFFIX},
    {"/./dir/./.file", NULL, ERR_NO_SUFFIX},
    {"/./ⓓïȓ/./ḟïľē.şüḟḟï𝓍", ".şüḟḟï𝓍", OK},
    {"/./ⓓïȓ/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK},
    {"/./ⓓïȓ/./ḟïľē.ş", ".ş", OK},
    {"/./ⓓïȓ/./ḟïľē.", ".", OK},
    {"/./ⓓïȓ/./ḟïľē", NULL, ERR_NO_SUFFIX},
    {"/./ⓓïȓ/./.ḟïľē", NULL, ERR_NO_SUFFIX},
    {"././dir/./file.suffix", ".suffix", OK},
    {"././dir/./file.very-long-suffix", ".very-long-suffix", OK},
    {"././dir/./file.s", ".s", OK},
    {"././dir/./file.", ".", OK},
    {"././dir/./file", NULL, ERR_NO_SUFFIX},
    {"././dir/./.file", NULL, ERR_NO_SUFFIX},
    {"././ⓓïȓ/./ḟïľē.şüḟḟï𝓍", ".şüḟḟï𝓍", OK},
    {"././ⓓïȓ/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK},
    {"././ⓓïȓ/./ḟïľē.ş", ".ş", OK},
    {"././ⓓïȓ/./ḟïľē.", ".", OK},
    {"././ⓓïȓ/./ḟïľē", NULL, ERR_NO_SUFFIX},
    {"././ⓓïȓ/./.ḟïľē", NULL, ERR_NO_SUFFIX},
    {".././dir/./file.suffix", ".suffix", OK},
    {".././dir/./file.very-long-suffix", ".very-long-suffix", OK},
    {".././dir/./file.s", ".s", OK},
    {".././dir/./file.", ".", OK},
    {".././dir/./file", NULL, ERR_NO_SUFFIX},
    {".././dir/./.file", NULL, ERR_NO_SUFFIX},
    {".././ⓓïȓ/./ḟïľē.şüḟḟï𝓍", ".şüḟḟï𝓍", OK},
    {".././ⓓïȓ/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK},
    {".././ⓓïȓ/./ḟïľē.ş", ".ş", OK},
    {".././ⓓïȓ/./ḟïľē.", ".", OK},
    {".././ⓓïȓ/./ḟïľē", NULL, ERR_NO_SUFFIX},
    {".././ⓓïȓ/./.ḟïľē", NULL, ERR_NO_SUFFIX},
    {"dir.suffix/./file.suffix", ".suffix", OK},
    {"dir.suffix/./file.very-long-suffix", ".very-long-suffix", OK},
    {"dir.suffix/./file.s", ".s", OK},
    {"dir.suffix/./file.", ".", OK},
    {"dir.suffix/./file", NULL, ERR_NO_SUFFIX},
    {"dir.suffix/./.file", NULL, ERR_NO_SUFFIX},
    {"ⓓïȓ.şüḟḟï𝓍/./ḟïľē.şüḟḟï𝓍", ".şüḟḟï𝓍", OK},
    {"ⓓïȓ.şüḟḟï𝓍/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK},
    {"ⓓïȓ.şüḟḟï𝓍/./ḟïľē.ş", ".ş", OK},
    {"ⓓïȓ.şüḟḟï𝓍/./ḟïľē.", ".", OK},
    {"ⓓïȓ.şüḟḟï𝓍/./ḟïľē", NULL, ERR_NO_SUFFIX},
    {"ⓓïȓ.şüḟḟï𝓍/./.ḟïľē", NULL, ERR_NO_SUFFIX},
    {"/./dir.suffix/./file.suffix", ".suffix", OK},
    {"/./dir.suffix/./file.very-long-suffix", ".very-long-suffix", OK},
    {"/./dir.suffix/./file.s", ".s", OK},
    {"/./dir.suffix/./file.", ".", OK},
    {"/./dir.suffix/./file", NULL, ERR_NO_SUFFIX},
    {"/./dir.suffix/./.file", NULL, ERR_NO_SUFFIX},
    {"/./ⓓïȓ.şüḟḟï𝓍/./ḟïľē.şüḟḟï𝓍", ".şüḟḟï𝓍", OK},
    {"/./ⓓïȓ.şüḟḟï𝓍/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK},
    {"/./ⓓïȓ.şüḟḟï𝓍/./ḟïľē.ş", ".ş", OK},
    {"/./ⓓïȓ.şüḟḟï𝓍/./ḟïľē.", ".", OK},
    {"/./ⓓïȓ.şüḟḟï𝓍/./ḟïľē", NULL, ERR_NO_SUFFIX},
    {"/./ⓓïȓ.şüḟḟï𝓍/./.ḟïľē", NULL, ERR_NO_SUFFIX},
    {"././dir.suffix/./file.suffix", ".suffix", OK},
    {"././dir.suffix/./file.very-long-suffix", ".very-long-suffix", OK},
    {"././dir.suffix/./file.s", ".s", OK},
    {"././dir.suffix/./file.", ".", OK},
    {"././dir.suffix/./file", NULL, ERR_NO_SUFFIX},
    {"././dir.suffix/./.file", NULL, ERR_NO_SUFFIX},
    {"././ⓓïȓ.şüḟḟï𝓍/./ḟïľē.şüḟḟï𝓍", ".şüḟḟï𝓍", OK},
    {"././ⓓïȓ.şüḟḟï𝓍/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK},
    {"././ⓓïȓ.şüḟḟï𝓍/./ḟïľē.ş", ".ş", OK},
    {"././ⓓïȓ.şüḟḟï𝓍/./ḟïľē.", ".", OK},
    {"././ⓓïȓ.şüḟḟï𝓍/./ḟïľē", NULL, ERR_NO_SUFFIX},
    {"././ⓓïȓ.şüḟḟï𝓍/./.ḟïľē", NULL, ERR_NO_SUFFIX},
    {".././dir.suffix/./file.suffix", ".suffix", OK},
    {".././dir.suffix/./file.very-long-suffix", ".very-long-suffix", OK},
    {".././dir.suffix/./file.s", ".s", OK},
    {".././dir.suffix/./file.", ".", OK},
    {".././dir.suffix/./file", NULL, ERR_NO_SUFFIX},
    {".././dir.suffix/./.file", NULL, ERR_NO_SUFFIX},
    {".././ⓓïȓ.şüḟḟï𝓍/./ḟïľē.şüḟḟï𝓍", ".şüḟḟï𝓍", OK},
    {".././ⓓïȓ.şüḟḟï𝓍/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK},
    {".././ⓓïȓ.şüḟḟï𝓍/./ḟïľē.ş", ".ş", OK},
    {".././ⓓïȓ.şüḟḟï𝓍/./ḟïľē.", ".", OK},
    {".././ⓓïȓ.şüḟḟï𝓍/./ḟïľē", NULL, ERR_NO_SUFFIX},
    {".././ⓓïȓ.şüḟḟï𝓍/./.ḟïľē", NULL, ERR_NO_SUFFIX},
    {"/../dir/../file.suffix", ".suffix", OK},
    {"/../dir/../file.very-long-suffix", ".very-long-suffix", OK},
    {"/../dir/../file.s", ".s", OK},
    {"/../dir/../file.", ".", OK},
    {"/../dir/../file", NULL, ERR_NO_SUFFIX},
    {"/../dir/../.file", NULL, ERR_NO_SUFFIX},
    {"/../ⓓïȓ/../ḟïľē.şüḟḟï𝓍", ".şüḟḟï𝓍", OK},
    {"/../ⓓïȓ/../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK},
    {"/../ⓓïȓ/../ḟïľē.ş", ".ş", OK},
    {"/../ⓓïȓ/../ḟïľē.", ".", OK},
    {"/../ⓓïȓ/../ḟïľē", NULL, ERR_NO_SUFFIX},
    {"/../ⓓïȓ/../.ḟïľē", NULL, ERR_NO_SUFFIX},
    {"./../dir/../file.suffix", ".suffix", OK},
    {"./../dir/../file.very-long-suffix", ".very-long-suffix", OK},
    {"./../dir/../file.s", ".s", OK},
    {"./../dir/../file.", ".", OK},
    {"./../dir/../file", NULL, ERR_NO_SUFFIX},
    {"./../dir/../.file", NULL, ERR_NO_SUFFIX},
    {"./../ⓓïȓ/../ḟïľē.şüḟḟï𝓍", ".şüḟḟï𝓍", OK},
    {"./../ⓓïȓ/../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK},
    {"./../ⓓïȓ/../ḟïľē.ş", ".ş", OK},
    {"./../ⓓïȓ/../ḟïľē.", ".", OK},
    {"./../ⓓïȓ/../ḟïľē", NULL, ERR_NO_SUFFIX},
    {"./../ⓓïȓ/../.ḟïľē", NULL, ERR_NO_SUFFIX},
    {"../../dir/../file.suffix", ".suffix", OK},
    {"../../dir/../file.very-long-suffix", ".very-long-suffix", OK},
    {"../../dir/../file.s", ".s", OK},
    {"../../dir/../file.", ".", OK},
    {"../../dir/../file", NULL, ERR_NO_SUFFIX},
    {"../../dir/../.file", NULL, ERR_NO_SUFFIX},
    {"../../ⓓïȓ/../ḟïľē.şüḟḟï𝓍", ".şüḟḟï𝓍", OK},
    {"../../ⓓïȓ/../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK},
    {"../../ⓓïȓ/../ḟïľē.ş", ".ş", OK},
    {"../../ⓓïȓ/../ḟïľē.", ".", OK},
    {"../../ⓓïȓ/../ḟïľē", NULL, ERR_NO_SUFFIX},
    {"../../ⓓïȓ/../.ḟïľē", NULL, ERR_NO_SUFFIX},
    {"dir.suffix/../file.suffix", ".suffix", OK},
    {"dir.suffix/../file.very-long-suffix", ".very-long-suffix", OK},
    {"dir.suffix/../file.s", ".s", OK},
    {"dir.suffix/../file.", ".", OK},
    {"dir.suffix/../file", NULL, ERR_NO_SUFFIX},
    {"dir.suffix/../.file", NULL, ERR_NO_SUFFIX},
    {"ⓓïȓ.şüḟḟï𝓍/../ḟïľē.şüḟḟï𝓍", ".şüḟḟï𝓍", OK},
    {"ⓓïȓ.şüḟḟï𝓍/../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK},
    {"ⓓïȓ.şüḟḟï𝓍/../ḟïľē.ş", ".ş", OK},
    {"ⓓïȓ.şüḟḟï𝓍/../ḟïľē.", ".", OK},
    {"ⓓïȓ.şüḟḟï𝓍/../ḟïľē", NULL, ERR_NO_SUFFIX},
    {"ⓓïȓ.şüḟḟï𝓍/../.ḟïľē", NULL, ERR_NO_SUFFIX},
    {"/../dir.suffix/../file.suffix", ".suffix", OK},
    {"/../dir.suffix/../file.very-long-suffix", ".very-long-suffix", OK},
    {"/../dir.suffix/../file.s", ".s", OK},
    {"/../dir.suffix/../file.", ".", OK},
    {"/../dir.suffix/../file", NULL, ERR_NO_SUFFIX},
    {"/../dir.suffix/../.file", NULL, ERR_NO_SUFFIX},
    {"/../ⓓïȓ.şüḟḟï𝓍/../ḟïľē.şüḟḟï𝓍", ".şüḟḟï𝓍", OK},
    {"/../ⓓïȓ.şüḟḟï𝓍/../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK},
    {"/../ⓓïȓ.şüḟḟï𝓍/../ḟïľē.ş", ".ş", OK},
    {"/../ⓓïȓ.şüḟḟï𝓍/../ḟïľē.", ".", OK},
    {"/../ⓓïȓ.şüḟḟï𝓍/../ḟïľē", NULL, ERR_NO_SUFFIX},
    {"/../ⓓïȓ.şüḟḟï𝓍/../.ḟïľē", NULL, ERR_NO_SUFFIX},
    {"./../dir.suffix/../file.suffix", ".suffix", OK},
    {"./../dir.suffix/../file.very-long-suffix", ".very-long-suffix", OK},
    {"./../dir.suffix/../file.s", ".s", OK},
    {"./../dir.suffix/../file.", ".", OK},
    {"./../dir.suffix/../file", NULL, ERR_NO_SUFFIX},
    {"./../dir.suffix/../.file", NULL, ERR_NO_SUFFIX},
    {"./../ⓓïȓ.şüḟḟï𝓍/../ḟïľē.şüḟḟï𝓍", ".şüḟḟï𝓍", OK},
    {"./../ⓓïȓ.şüḟḟï𝓍/../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK},
    {"./../ⓓïȓ.şüḟḟï𝓍/../ḟïľē.ş", ".ş", OK},
    {"./../ⓓïȓ.şüḟḟï𝓍/../ḟïľē.", ".", OK},
    {"./../ⓓïȓ.şüḟḟï𝓍/../ḟïľē", NULL, ERR_NO_SUFFIX},
    {"./../ⓓïȓ.şüḟḟï𝓍/../.ḟïľē", NULL, ERR_NO_SUFFIX},
    {"../../dir.suffix/../file.suffix", ".suffix", OK},
    {"../../dir.suffix/../file.very-long-suffix", ".very-long-suffix", OK},
    {"../../dir.suffix/../file.s", ".s", OK},
    {"../../dir.suffix/../file.", ".", OK},
    {"../../dir.suffix/../file", NULL, ERR_NO_SUFFIX},
    {"../../dir.suffix/../.file", NULL, ERR_NO_SUFFIX},
    {"../../ⓓïȓ.şüḟḟï𝓍/../ḟïľē.şüḟḟï𝓍", ".şüḟḟï𝓍", OK},
    {"../../ⓓïȓ.şüḟḟï𝓍/../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍", OK},
    {"../../ⓓïȓ.şüḟḟï𝓍/../ḟïľē.ş", ".ş", OK},
    {"../../ⓓïȓ.şüḟḟï𝓍/../ḟïľē.", ".", OK},
    {"../../ⓓïȓ.şüḟḟï𝓍/../ḟïľē", NULL, ERR_NO_SUFFIX},
    {"../../ⓓïȓ.şüḟḟï𝓍/../.ḟïľē", NULL, ERR_NO_SUFFIX},
    {".///", NULL, ERR_NO_SUFFIX},
    {"////", NULL, ERR_NO_SUFFIX},
    {"file.suffix///", ".suffix///", OK},
    {"file.very-long-suffix///", ".very-long-suffix///", OK},
    {"file.s///", ".s///", OK},
    {"file.///", ".///", OK},
    {"file///", NULL, ERR_NO_SUFFIX},
    {".file///", NULL, ERR_NO_SUFFIX},
    {"ḟïľē.şüḟḟï𝓍///", ".şüḟḟï𝓍///", OK},
    {"ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK},
    {"ḟïľē.ş///", ".ş///", OK},
    {"ḟïľē.///", ".///", OK},
    {"ḟïľē///", NULL, ERR_NO_SUFFIX},
    {".ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"/file.suffix///", ".suffix///", OK},
    {"/file.very-long-suffix///", ".very-long-suffix///", OK},
    {"/file.s///", ".s///", OK},
    {"/file.///", ".///", OK},
    {"/file///", NULL, ERR_NO_SUFFIX},
    {"/.file///", NULL, ERR_NO_SUFFIX},
    {"/ḟïľē.şüḟḟï𝓍///", ".şüḟḟï𝓍///", OK},
    {"/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK},
    {"/ḟïľē.ş///", ".ş///", OK},
    {"/ḟïľē.///", ".///", OK},
    {"/ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"/.ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"./file.suffix///", ".suffix///", OK},
    {"./file.very-long-suffix///", ".very-long-suffix///", OK},
    {"./file.s///", ".s///", OK},
    {"./file.///", ".///", OK},
    {"./file///", NULL, ERR_NO_SUFFIX},
    {"./.file///", NULL, ERR_NO_SUFFIX},
    {"./ḟïľē.şüḟḟï𝓍///", ".şüḟḟï𝓍///", OK},
    {"./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK},
    {"./ḟïľē.ş///", ".ş///", OK},
    {"./ḟïľē.///", ".///", OK},
    {"./ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"./.ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"../file.suffix///", ".suffix///", OK},
    {"../file.very-long-suffix///", ".very-long-suffix///", OK},
    {"../file.s///", ".s///", OK},
    {"../file.///", ".///", OK},
    {"../file///", NULL, ERR_NO_SUFFIX},
    {"../.file///", NULL, ERR_NO_SUFFIX},
    {"../ḟïľē.şüḟḟï𝓍///", ".şüḟḟï𝓍///", OK},
    {"../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK},
    {"../ḟïľē.ş///", ".ş///", OK},
    {"../ḟïľē.///", ".///", OK},
    {"../ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"../.ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"dir/file.suffix///", ".suffix///", OK},
    {"dir/file.very-long-suffix///", ".very-long-suffix///", OK},
    {"dir/file.s///", ".s///", OK},
    {"dir/file.///", ".///", OK},
    {"dir/file///", NULL, ERR_NO_SUFFIX},
    {"dir/.file///", NULL, ERR_NO_SUFFIX},
    {"ⓓïȓ/ḟïľē.şüḟḟï𝓍///", ".şüḟḟï𝓍///", OK},
    {"ⓓïȓ/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK},
    {"ⓓïȓ/ḟïľē.ş///", ".ş///", OK},
    {"ⓓïȓ/ḟïľē.///", ".///", OK},
    {"ⓓïȓ/ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"ⓓïȓ/.ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"/dir/file.suffix///", ".suffix///", OK},
    {"/dir/file.very-long-suffix///", ".very-long-suffix///", OK},
    {"/dir/file.s///", ".s///", OK},
    {"/dir/file.///", ".///", OK},
    {"/dir/file///", NULL, ERR_NO_SUFFIX},
    {"/dir/.file///", NULL, ERR_NO_SUFFIX},
    {"/ⓓïȓ/ḟïľē.şüḟḟï𝓍///", ".şüḟḟï𝓍///", OK},
    {"/ⓓïȓ/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK},
    {"/ⓓïȓ/ḟïľē.ş///", ".ş///", OK},
    {"/ⓓïȓ/ḟïľē.///", ".///", OK},
    {"/ⓓïȓ/ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"/ⓓïȓ/.ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"./dir/file.suffix///", ".suffix///", OK},
    {"./dir/file.very-long-suffix///", ".very-long-suffix///", OK},
    {"./dir/file.s///", ".s///", OK},
    {"./dir/file.///", ".///", OK},
    {"./dir/file///", NULL, ERR_NO_SUFFIX},
    {"./dir/.file///", NULL, ERR_NO_SUFFIX},
    {"./ⓓïȓ/ḟïľē.şüḟḟï𝓍///", ".şüḟḟï𝓍///", OK},
    {"./ⓓïȓ/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK},
    {"./ⓓïȓ/ḟïľē.ş///", ".ş///", OK},
    {"./ⓓïȓ/ḟïľē.///", ".///", OK},
    {"./ⓓïȓ/ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"./ⓓïȓ/.ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"../dir/file.suffix///", ".suffix///", OK},
    {"../dir/file.very-long-suffix///", ".very-long-suffix///", OK},
    {"../dir/file.s///", ".s///", OK},
    {"../dir/file.///", ".///", OK},
    {"../dir/file///", NULL, ERR_NO_SUFFIX},
    {"../dir/.file///", NULL, ERR_NO_SUFFIX},
    {"../ⓓïȓ/ḟïľē.şüḟḟï𝓍///", ".şüḟḟï𝓍///", OK},
    {"../ⓓïȓ/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK},
    {"../ⓓïȓ/ḟïľē.ş///", ".ş///", OK},
    {"../ⓓïȓ/ḟïľē.///", ".///", OK},
    {"../ⓓïȓ/ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"../ⓓïȓ/.ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"dir.suffix/file.suffix///", ".suffix///", OK},
    {"dir.suffix/file.very-long-suffix///", ".very-long-suffix///", OK},
    {"dir.suffix/file.s///", ".s///", OK},
    {"dir.suffix/file.///", ".///", OK},
    {"dir.suffix/file///", NULL, ERR_NO_SUFFIX},
    {"dir.suffix/.file///", NULL, ERR_NO_SUFFIX},
    {"ⓓïȓ.şüḟḟï𝓍/ḟïľē.şüḟḟï𝓍///", ".şüḟḟï𝓍///", OK},
    {"ⓓïȓ.şüḟḟï𝓍/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK},
    {"ⓓïȓ.şüḟḟï𝓍/ḟïľē.ş///", ".ş///", OK},
    {"ⓓïȓ.şüḟḟï𝓍/ḟïľē.///", ".///", OK},
    {"ⓓïȓ.şüḟḟï𝓍/ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"ⓓïȓ.şüḟḟï𝓍/.ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"/dir.suffix/file.suffix///", ".suffix///", OK},
    {"/dir.suffix/file.very-long-suffix///", ".very-long-suffix///", OK},
    {"/dir.suffix/file.s///", ".s///", OK},
    {"/dir.suffix/file.///", ".///", OK},
    {"/dir.suffix/file///", NULL, ERR_NO_SUFFIX},
    {"/dir.suffix/.file///", NULL, ERR_NO_SUFFIX},
    {"/ⓓïȓ.şüḟḟï𝓍/ḟïľē.şüḟḟï𝓍///", ".şüḟḟï𝓍///", OK},
    {"/ⓓïȓ.şüḟḟï𝓍/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK},
    {"/ⓓïȓ.şüḟḟï𝓍/ḟïľē.ş///", ".ş///", OK},
    {"/ⓓïȓ.şüḟḟï𝓍/ḟïľē.///", ".///", OK},
    {"/ⓓïȓ.şüḟḟï𝓍/ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"/ⓓïȓ.şüḟḟï𝓍/.ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"./dir.suffix/file.suffix///", ".suffix///", OK},
    {"./dir.suffix/file.very-long-suffix///", ".very-long-suffix///", OK},
    {"./dir.suffix/file.s///", ".s///", OK},
    {"./dir.suffix/file.///", ".///", OK},
    {"./dir.suffix/file///", NULL, ERR_NO_SUFFIX},
    {"./dir.suffix/.file///", NULL, ERR_NO_SUFFIX},
    {"./ⓓïȓ.şüḟḟï𝓍/ḟïľē.şüḟḟï𝓍///", ".şüḟḟï𝓍///", OK},
    {"./ⓓïȓ.şüḟḟï𝓍/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK},
    {"./ⓓïȓ.şüḟḟï𝓍/ḟïľē.ş///", ".ş///", OK},
    {"./ⓓïȓ.şüḟḟï𝓍/ḟïľē.///", ".///", OK},
    {"./ⓓïȓ.şüḟḟï𝓍/ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"./ⓓïȓ.şüḟḟï𝓍/.ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"../dir.suffix/file.suffix///", ".suffix///", OK},
    {"../dir.suffix/file.very-long-suffix///", ".very-long-suffix///", OK},
    {"../dir.suffix/file.s///", ".s///", OK},
    {"../dir.suffix/file.///", ".///", OK},
    {"../dir.suffix/file///", NULL, ERR_NO_SUFFIX},
    {"../dir.suffix/.file///", NULL, ERR_NO_SUFFIX},
    {"../ⓓïȓ.şüḟḟï𝓍/ḟïľē.şüḟḟï𝓍///", ".şüḟḟï𝓍///", OK},
    {"../ⓓïȓ.şüḟḟï𝓍/ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK},
    {"../ⓓïȓ.şüḟḟï𝓍/ḟïľē.ş///", ".ş///", OK},
    {"../ⓓïȓ.şüḟḟï𝓍/ḟïľē.///", ".///", OK},
    {"../ⓓïȓ.şüḟḟï𝓍/ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"../ⓓïȓ.şüḟḟï𝓍/.ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"dir/./file.suffix///", ".suffix///", OK},
    {"dir/./file.very-long-suffix///", ".very-long-suffix///", OK},
    {"dir/./file.s///", ".s///", OK},
    {"dir/./file.///", ".///", OK},
    {"dir/./file///", NULL, ERR_NO_SUFFIX},
    {"dir/./.file///", NULL, ERR_NO_SUFFIX},
    {"ⓓïȓ/./ḟïľē.şüḟḟï𝓍///", ".şüḟḟï𝓍///", OK},
    {"ⓓïȓ/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK},
    {"ⓓïȓ/./ḟïľē.ş///", ".ş///", OK},
    {"ⓓïȓ/./ḟïľē.///", ".///", OK},
    {"ⓓïȓ/./ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"ⓓïȓ/./.ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"/./dir/./file.suffix///", ".suffix///", OK},
    {"/./dir/./file.very-long-suffix///", ".very-long-suffix///", OK},
    {"/./dir/./file.s///", ".s///", OK},
    {"/./dir/./file.///", ".///", OK},
    {"/./dir/./file///", NULL, ERR_NO_SUFFIX},
    {"/./dir/./.file///", NULL, ERR_NO_SUFFIX},
    {"/./ⓓïȓ/./ḟïľē.şüḟḟï𝓍///", ".şüḟḟï𝓍///", OK},
    {"/./ⓓïȓ/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK},
    {"/./ⓓïȓ/./ḟïľē.ş///", ".ş///", OK},
    {"/./ⓓïȓ/./ḟïľē.///", ".///", OK},
    {"/./ⓓïȓ/./ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"/./ⓓïȓ/./.ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"././dir/./file.suffix///", ".suffix///", OK},
    {"././dir/./file.very-long-suffix///", ".very-long-suffix///", OK},
    {"././dir/./file.s///", ".s///", OK},
    {"././dir/./file.///", ".///", OK},
    {"././dir/./file///", NULL, ERR_NO_SUFFIX},
    {"././dir/./.file///", NULL, ERR_NO_SUFFIX},
    {"././ⓓïȓ/./ḟïľē.şüḟḟï𝓍///", ".şüḟḟï𝓍///", OK},
    {"././ⓓïȓ/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK},
    {"././ⓓïȓ/./ḟïľē.ş///", ".ş///", OK},
    {"././ⓓïȓ/./ḟïľē.///", ".///", OK},
    {"././ⓓïȓ/./ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"././ⓓïȓ/./.ḟïľē///", NULL, ERR_NO_SUFFIX},
    {".././dir/./file.suffix///", ".suffix///", OK},
    {".././dir/./file.very-long-suffix///", ".very-long-suffix///", OK},
    {".././dir/./file.s///", ".s///", OK},
    {".././dir/./file.///", ".///", OK},
    {".././dir/./file///", NULL, ERR_NO_SUFFIX},
    {".././dir/./.file///", NULL, ERR_NO_SUFFIX},
    {".././ⓓïȓ/./ḟïľē.şüḟḟï𝓍///", ".şüḟḟï𝓍///", OK},
    {".././ⓓïȓ/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK},
    {".././ⓓïȓ/./ḟïľē.ş///", ".ş///", OK},
    {".././ⓓïȓ/./ḟïľē.///", ".///", OK},
    {".././ⓓïȓ/./ḟïľē///", NULL, ERR_NO_SUFFIX},
    {".././ⓓïȓ/./.ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"dir.suffix/./file.suffix///", ".suffix///", OK},
    {"dir.suffix/./file.very-long-suffix///", ".very-long-suffix///", OK},
    {"dir.suffix/./file.s///", ".s///", OK},
    {"dir.suffix/./file.///", ".///", OK},
    {"dir.suffix/./file///", NULL, ERR_NO_SUFFIX},
    {"dir.suffix/./.file///", NULL, ERR_NO_SUFFIX},
    {"ⓓïȓ.şüḟḟï𝓍/./ḟïľē.şüḟḟï𝓍///", ".şüḟḟï𝓍///", OK},
    {"ⓓïȓ.şüḟḟï𝓍/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK},
    {"ⓓïȓ.şüḟḟï𝓍/./ḟïľē.ş///", ".ş///", OK},
    {"ⓓïȓ.şüḟḟï𝓍/./ḟïľē.///", ".///", OK},
    {"ⓓïȓ.şüḟḟï𝓍/./ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"ⓓïȓ.şüḟḟï𝓍/./.ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"/./dir.suffix/./file.suffix///", ".suffix///", OK},
    {"/./dir.suffix/./file.very-long-suffix///", ".very-long-suffix///", OK},
    {"/./dir.suffix/./file.s///", ".s///", OK},
    {"/./dir.suffix/./file.///", ".///", OK},
    {"/./dir.suffix/./file///", NULL, ERR_NO_SUFFIX},
    {"/./dir.suffix/./.file///", NULL, ERR_NO_SUFFIX},
    {"/./ⓓïȓ.şüḟḟï𝓍/./ḟïľē.şüḟḟï𝓍///", ".şüḟḟï𝓍///", OK},
    {"/./ⓓïȓ.şüḟḟï𝓍/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK},
    {"/./ⓓïȓ.şüḟḟï𝓍/./ḟïľē.ş///", ".ş///", OK},
    {"/./ⓓïȓ.şüḟḟï𝓍/./ḟïľē.///", ".///", OK},
    {"/./ⓓïȓ.şüḟḟï𝓍/./ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"/./ⓓïȓ.şüḟḟï𝓍/./.ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"././dir.suffix/./file.suffix///", ".suffix///", OK},
    {"././dir.suffix/./file.very-long-suffix///", ".very-long-suffix///", OK},
    {"././dir.suffix/./file.s///", ".s///", OK},
    {"././dir.suffix/./file.///", ".///", OK},
    {"././dir.suffix/./file///", NULL, ERR_NO_SUFFIX},
    {"././dir.suffix/./.file///", NULL, ERR_NO_SUFFIX},
    {"././ⓓïȓ.şüḟḟï𝓍/./ḟïľē.şüḟḟï𝓍///", ".şüḟḟï𝓍///", OK},
    {"././ⓓïȓ.şüḟḟï𝓍/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK},
    {"././ⓓïȓ.şüḟḟï𝓍/./ḟïľē.ş///", ".ş///", OK},
    {"././ⓓïȓ.şüḟḟï𝓍/./ḟïľē.///", ".///", OK},
    {"././ⓓïȓ.şüḟḟï𝓍/./ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"././ⓓïȓ.şüḟḟï𝓍/./.ḟïľē///", NULL, ERR_NO_SUFFIX},
    {".././dir.suffix/./file.suffix///", ".suffix///", OK},
    {".././dir.suffix/./file.very-long-suffix///", ".very-long-suffix///", OK},
    {".././dir.suffix/./file.s///", ".s///", OK},
    {".././dir.suffix/./file.///", ".///", OK},
    {".././dir.suffix/./file///", NULL, ERR_NO_SUFFIX},
    {".././dir.suffix/./.file///", NULL, ERR_NO_SUFFIX},
    {".././ⓓïȓ.şüḟḟï𝓍/./ḟïľē.şüḟḟï𝓍///", ".şüḟḟï𝓍///", OK},
    {".././ⓓïȓ.şüḟḟï𝓍/./ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK},
    {".././ⓓïȓ.şüḟḟï𝓍/./ḟïľē.ş///", ".ş///", OK},
    {".././ⓓïȓ.şüḟḟï𝓍/./ḟïľē.///", ".///", OK},
    {".././ⓓïȓ.şüḟḟï𝓍/./ḟïľē///", NULL, ERR_NO_SUFFIX},
    {".././ⓓïȓ.şüḟḟï𝓍/./.ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"/../dir/../file.suffix///", ".suffix///", OK},
    {"/../dir/../file.very-long-suffix///", ".very-long-suffix///", OK},
    {"/../dir/../file.s///", ".s///", OK},
    {"/../dir/../file.///", ".///", OK},
    {"/../dir/../file///", NULL, ERR_NO_SUFFIX},
    {"/../dir/../.file///", NULL, ERR_NO_SUFFIX},
    {"/../ⓓïȓ/../ḟïľē.şüḟḟï𝓍///", ".şüḟḟï𝓍///", OK},
    {"/../ⓓïȓ/../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK},
    {"/../ⓓïȓ/../ḟïľē.ş///", ".ş///", OK},
    {"/../ⓓïȓ/../ḟïľē.///", ".///", OK},
    {"/../ⓓïȓ/../ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"/../ⓓïȓ/../.ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"./../dir/../file.suffix///", ".suffix///", OK},
    {"./../dir/../file.very-long-suffix///", ".very-long-suffix///", OK},
    {"./../dir/../file.s///", ".s///", OK},
    {"./../dir/../file.///", ".///", OK},
    {"./../dir/../file///", NULL, ERR_NO_SUFFIX},
    {"./../dir/../.file///", NULL, ERR_NO_SUFFIX},
    {"./../ⓓïȓ/../ḟïľē.şüḟḟï𝓍///", ".şüḟḟï𝓍///", OK},
    {"./../ⓓïȓ/../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK},
    {"./../ⓓïȓ/../ḟïľē.ş///", ".ş///", OK},
    {"./../ⓓïȓ/../ḟïľē.///", ".///", OK},
    {"./../ⓓïȓ/../ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"./../ⓓïȓ/../.ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"../../dir/../file.suffix///", ".suffix///", OK},
    {"../../dir/../file.very-long-suffix///", ".very-long-suffix///", OK},
    {"../../dir/../file.s///", ".s///", OK},
    {"../../dir/../file.///", ".///", OK},
    {"../../dir/../file///", NULL, ERR_NO_SUFFIX},
    {"../../dir/../.file///", NULL, ERR_NO_SUFFIX},
    {"../../ⓓïȓ/../ḟïľē.şüḟḟï𝓍///", ".şüḟḟï𝓍///", OK},
    {"../../ⓓïȓ/../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK},
    {"../../ⓓïȓ/../ḟïľē.ş///", ".ş///", OK},
    {"../../ⓓïȓ/../ḟïľē.///", ".///", OK},
    {"../../ⓓïȓ/../ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"../../ⓓïȓ/../.ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"dir.suffix/../file.suffix///", ".suffix///", OK},
    {"dir.suffix/../file.very-long-suffix///", ".very-long-suffix///", OK},
    {"dir.suffix/../file.s///", ".s///", OK},
    {"dir.suffix/../file.///", ".///", OK},
    {"dir.suffix/../file///", NULL, ERR_NO_SUFFIX},
    {"dir.suffix/../.file///", NULL, ERR_NO_SUFFIX},
    {"ⓓïȓ.şüḟḟï𝓍/../ḟïľē.şüḟḟï𝓍///", ".şüḟḟï𝓍///", OK},
    {"ⓓïȓ.şüḟḟï𝓍/../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK},
    {"ⓓïȓ.şüḟḟï𝓍/../ḟïľē.ş///", ".ş///", OK},
    {"ⓓïȓ.şüḟḟï𝓍/../ḟïľē.///", ".///", OK},
    {"ⓓïȓ.şüḟḟï𝓍/../ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"ⓓïȓ.şüḟḟï𝓍/../.ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"/../dir.suffix/../file.suffix///", ".suffix///", OK},
    {"/../dir.suffix/../file.very-long-suffix///", ".very-long-suffix///", OK},
    {"/../dir.suffix/../file.s///", ".s///", OK},
    {"/../dir.suffix/../file.///", ".///", OK},
    {"/../dir.suffix/../file///", NULL, ERR_NO_SUFFIX},
    {"/../dir.suffix/../.file///", NULL, ERR_NO_SUFFIX},
    {"/../ⓓïȓ.şüḟḟï𝓍/../ḟïľē.şüḟḟï𝓍///", ".şüḟḟï𝓍///", OK},
    {"/../ⓓïȓ.şüḟḟï𝓍/../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK},
    {"/../ⓓïȓ.şüḟḟï𝓍/../ḟïľē.ş///", ".ş///", OK},
    {"/../ⓓïȓ.şüḟḟï𝓍/../ḟïľē.///", ".///", OK},
    {"/../ⓓïȓ.şüḟḟï𝓍/../ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"/../ⓓïȓ.şüḟḟï𝓍/../.ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"./../dir.suffix/../file.suffix///", ".suffix///", OK},
    {"./../dir.suffix/../file.very-long-suffix///", ".very-long-suffix///", OK},
    {"./../dir.suffix/../file.s///", ".s///", OK},
    {"./../dir.suffix/../file.///", ".///", OK},
    {"./../dir.suffix/../file///", NULL, ERR_NO_SUFFIX},
    {"./../dir.suffix/../.file///", NULL, ERR_NO_SUFFIX},
    {"./../ⓓïȓ.şüḟḟï𝓍/../ḟïľē.şüḟḟï𝓍///", ".şüḟḟï𝓍///", OK},
    {"./../ⓓïȓ.şüḟḟï𝓍/../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK},
    {"./../ⓓïȓ.şüḟḟï𝓍/../ḟïľē.ş///", ".ş///", OK},
    {"./../ⓓïȓ.şüḟḟï𝓍/../ḟïľē.///", ".///", OK},
    {"./../ⓓïȓ.şüḟḟï𝓍/../ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"./../ⓓïȓ.şüḟḟï𝓍/../.ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"../../dir.suffix/../file.suffix///", ".suffix///", OK},
    {"../../dir.suffix/../file.very-long-suffix///", ".very-long-suffix///", OK},
    {"../../dir.suffix/../file.s///", ".s///", OK},
    {"../../dir.suffix/../file.///", ".///", OK},
    {"../../dir.suffix/../file///", NULL, ERR_NO_SUFFIX},
    {"../../dir.suffix/../.file///", NULL, ERR_NO_SUFFIX},
    {"../../ⓓïȓ.şüḟḟï𝓍/../ḟïľē.şüḟḟï𝓍///", ".şüḟḟï𝓍///", OK},
    {"../../ⓓïȓ.şüḟḟï𝓍/../ḟïľē.𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", ".𝚟ēȓẙ-ľǭñģ-şüḟḟï𝓍///", OK},
    {"../../ⓓïȓ.şüḟḟï𝓍/../ḟïľē.ş///", ".ş///", OK},
    {"../../ⓓïȓ.şüḟḟï𝓍/../ḟïľē.///", ".///", OK},
    {"../../ⓓïȓ.şüḟḟï𝓍/../ḟïľē///", NULL, ERR_NO_SUFFIX},
    {"../../ⓓïȓ.şüḟḟï𝓍/../.ḟïľē///", NULL, ERR_NO_SUFFIX}
};


/*
 * Main
 */

int
main (void)
{
    char ascii[127];            /* Array of ASCII characters w/o NUL. */
    char **prefixes;            /* Dynamically created prefixes. */
    char **suffices;            /* Dynmiacally created suffixes. */
    unsigned int maxnfixes;     /* Maximum number of pre- or suffices. */
    unsigned int nprefixes;     /* Number of prefixes. */
    unsigned int nsuffices;     /* Number of suffices. */

    maxnfixes = (unsigned int) pow(sizeof(ascii), FIX_LEN);
    nprefixes = 0;
    nsuffices = 0;

    for (unsigned int i = 0; i < sizeof(ascii); ++i) {
        ascii[i] = (char) (i + 1);
    }

    prefixes = calloc(maxnfixes, sizeof(*prefixes));
    if(prefixes == NULL) {
        err(TEST_ERROR, "calloc");
    }

    suffices = calloc(maxnfixes, sizeof(*suffices));
    if(suffices == NULL) {
        err(TEST_ERROR, "calloc");
    }

    for (size_t i = 0; i < NELEMS(cases); ++i) {
        const Args args = cases[i];
        const char *suffix;
        Error ret;

        warnx("checking (%s, -> %s) -> %u ...",
              args.fname, args.suffix, args.ret);

        ret = path_suffix(args.fname, &suffix);

        if (ret != args.ret) {
            errx(TEST_FAILED, "returned code %u", ret);
        }
        if (ret == OK && strncmp(args.suffix, suffix, MAX_SUFFIX_LEN) != 0) {
            errx(TEST_FAILED, "returned suffix %s", suffix);
        }
    }

    warnx("generating up to %u filenames and suffices ...", maxnfixes);
    for (unsigned int i = 0; i < maxnfixes; ++i) {
        char *s;
        int rc;

        s = calloc(FIX_LEN + 1, sizeof(*s));
        if(s == NULL) {
            err(TEST_ERROR, "calloc");
        }

        rc = to_str(i, sizeof(ascii), ascii, FIX_LEN + 1U, s);
        assert(rc == 0);

        if (strchr(s, '/') == NULL) {
            prefixes[nprefixes++] = s;

            if (strchr(s, '.') == NULL) {
                suffices[nsuffices++] = s;
            }
        }
    }

    warnx("checking dynamically created filenames ...");
    for (unsigned int i = 0; i < nprefixes; ++i) {
        for (unsigned int j = 0; j < nsuffices; ++j) {
            char fname[FNAME_LEN + 1U];
            const char *suffix;
            int n;
            Error ret;

            n = snprintf(fname, sizeof(fname),
                         "%s.%s", prefixes[i], suffices[j]);
            if (n < 0) {
                err(TEST_ERROR, "snprintf");
            }
            assert((size_t) n <= FNAME_LEN);

            ret = path_suffix(fname, &suffix);

            if (ret != OK) {
                errx(TEST_FAILED, "(%s, -> %s) -> %u!", fname, suffix, ret);
            }

            if (suffix[0] != '.' ||
                strncmp(suffices[j], &suffix[1], MAX_STR_LEN) != 0)
            {
                errx(TEST_FAILED, "(%s, -> %s!) -> %u", fname, suffix, ret);
            }
        }
    }

    warnx("all tests passed");
    return EXIT_SUCCESS;
}
