/*
 * Functions for fail handling.
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

#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <syslog.h>

#include "err.h"


#if !LOG_PERROR
#define LOG_PERROR 0
#endif


void
fail(const char *const message, ...)
{
	va_list ap;
	
	openlog("sucgi", LOG_CONS | LOG_NDELAY | LOG_PERROR, LOG_AUTH);
	va_start(ap, message);
	vsyslog(LOG_ERR, message, ap);
	va_end(ap);
	closelog();

	exit(EXIT_FAILURE);
}
