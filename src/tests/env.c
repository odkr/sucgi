/* Environment functions for the test suite.
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

#include <assert.h>
#include <stdarg.h>

#include "../err.h"
#include "env.h"


error
env_init(const size_t n, ...)
{
	va_list ap;	/* Variadic argument. */
	size_t i = 0;	/* Iterator. */

	assert(n > 0U);
	environ = calloc(n, sizeof(char *));
	if (!environ) return ERR_SYS;

	va_start(ap, n);
	for (; i < (n - 1u); i++) {
		char *var = va_arg(ap, char *);
		assert(var);
		environ[i] = var;
	}

	va_end(ap);
	environ[i] = NULL;

	return OK;
}
