/*
 * Error handling for suCGI.
 *
 * Copyright 2022 Odin Kroeger
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

#define _ISOC99_SOURCE
#define _POSIX_C_SOURCE 200809L
#define _BSD_SOURCE
#define _DARWIN_C_SOURCE
#define _DEFAULT_SOURCE

#if !defined(_FORTIFY_SOURCE)
#define _FORTIFY_SOURCE 3
#endif /* !defined(_FORTIFY_SOURCE) */

#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <syslog.h>

#include "err.h"


/*
 * Constants
 */

#if !defined(LOG_PERROR)
#define LOG_PERROR 0
#endif


/*
 * Functions
 */

void
error(const char *const message, ...)
{
	va_list ap;

	assert(*message != '\0');

	openlog("sucgi", LOG_CONS | LOG_NDELAY | LOG_PERROR, LOG_AUTH);
	va_start(ap, message);
	vsyslog(LOG_ERR, message, ap);
	va_end(ap);
	closelog();

	exit(EXIT_FAILURE);
}