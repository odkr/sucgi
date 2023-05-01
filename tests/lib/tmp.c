/*
 * Temporary directory handling.
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

#define _XOPEN_SOURCE 700

#include <sys/stat.h>
#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "ftree.h"
#include "tmp.h"


/*
 * Module variables
 */

/* Name of the temporary directory. */
static char tmpdir[L_tmpnam] = {0};

/* mkdir(tmpdir, ...) return value. */
static volatile sig_atomic_t created = -1;


/*
 * Functions
 */

char *
tmpdirmake(void)
{
    char *fname;

    if (created == 0) {
        return tmpdir;
    }

/*
 * The temporary name is used to create a directory, which
 * is an atomic operation, so using tmpnam is fine.
 */
#if defined(__GNUC__) && \
    (__GNUC__ > 3 || (__GNUC__ >= 3 && __GNUC_MINOR__ > 1))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

    errno = 0;
    if (tmpnam(tmpdir) == NULL) {
        return NULL;
    }

#if defined(__GNUC__) && \
    (__GNUC__ > 3 || (__GNUC__ >= 3 && __GNUC_MINOR__ > 1))
#pragma GCC diagnostic pop
#endif

    created = mkdir(tmpdir, S_IRWXU);
    if (created != 0) {
        return NULL;
    }

    fname = realpath(tmpdir, NULL);
    if (fname == NULL) {
        return NULL;
    }

    return fname;
}

int
tmpdirremove(void) {
    int retval;

    retval = 0;

    if (created != 0) {
        return retval;
    }

    created = -1;

    if (*tmpdir == '\0') {
        return retval;
    }

    retval = ftreerm(tmpdir);

    *tmpdir = '\0';

    return retval;
}
