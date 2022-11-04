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

#if !defined(_FORTIFY_SOURCE)
#define _FORTIFY_SOURCE 3
#endif

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "types.h"


#if defined(LOG_PERROR) && LOG_PERROR
#define ERROR_LOG_OPTS LOG_CONS | LOG_NDELAY | LOG_PERROR
#else
#define ERROR_LOG_OPTS LOG_CONS | LOG_NDELAY
#endif


void
error(const char *const message, ...)
{
	va_list ap;

	assert(*message);

	openlog("sucgi", ERROR_LOG_OPTS, LOG_AUTH);

	va_start(ap, message);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
	vsyslog(LOG_ERR, message, ap);
#pragma GCC diagnostic pop
	va_end(ap);

	closelog();

	exit(EXIT_FAILURE);
}
