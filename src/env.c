/*
 * Environment handling for suCGI.
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
#include <errno.h>
#include <fnmatch.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "env.h"
#include "err.h"
#include "file.h"
#include "path.h"
#include "str.h"


/*
 * Globals
 */

/* The environment. */
extern char **environ;

/* Default environment variables to keep. */
/* flawfinder: ignore (array is constant). */
const char *const env_keep[49] = {
	"AUTH_TYPE",
	"CONTENT_LENGTH",
	"CONTENT_TYPE",
	"CONTEXT_DOCUMENT_ROOT",
	"CONTEXT_PREFIX",
	"DATE_GMT",
	"DATE_LOCAL",
	"DOCUMENT_NAME",
	"DOCUMENT_PATH_INFO",
	"DOCUMENT_ROOT",
	"DOCUMENT_URI",
	"GATEWAY_INTERFACE",
	"HTTP_*",
	"HTTPS",
	"LAST_MODIFIED",
	"PATH_INFO",
	"PATH_TRANSLATED",
	"QUERY_STRING",
	"QUERY_STRING_UNESCAPED",
	"REMOTE_ADDR",
	"REMOTE_HOST",
	"REMOTE_IDENT",
	"REMOTE_PORT",
	"REMOTE_USER",
	"REDIRECT_ERROR_NOTES",
	"REDIRECT_HANDLER",
	"REDIRECT_QUERY_STRING",
	"REDIRECT_REMOTE_USER",
	"REDIRECT_SCRIPT_FILENAME",
	"REDIRECT_STATUS REDIRECT_URL",
	"REQUEST_METHOD",
	"REQUEST_URI",
	"REQUEST_SCHEME",
	"SCRIPT_FILENAME",
	"SCRIPT_NAME",
	"SCRIPT_URI",
	"SCRIPT_URL",
	"SERVER_ADMIN",
	"SERVER_NAME",
	"SERVER_ADDR",
	"SERVER_PORT",
	"SERVER_PROTOCOL",
	"SERVER_SIGNATURE",
	"SERVER_SOFTWARE",
	"SSL_*",
	"UNIQUE_ID",
	"USER_NAME",
	"TZ",
	NULL	/* Array terminator. DO NOT REMOVE. */
};

/* Default environment variables to toss. */
/* flawfinder: ignore (array is constant). */
const char *const env_toss[2] = {
	"HTTP_PROXY",
	NULL	/* Array terminator. DO NOT REMOVE. */
};


/*
 * Functions
 */

error
env_clear(char *vars[])
{
	static char *var = NULL;	/* First environment variable. */
	char **env = environ;		/* Copy of the pointer. */
	
	var = NULL;
	environ = &var;

	if (!vars) return OK;
	for (size_t n = 0; n < ENV_MAX; n++) {
		vars[n] = env[n];
		if (!env[n]) return OK;
	}

	return ERR_ENV_MAX;
}

error
env_get_fname(const char *name, char **fname, struct stat *fstatus)
{
	char *value = NULL;

	assert(name && fname);
	/* flawfinder: ignore (value is sanitised below). */
	value = getenv(name);
	if (!value) return ERR_VAR_UNDEF;
	if (*value == '\0') return ERR_VAR_EMPTY;
	reraise(path_check_len(value));
	reraise(file_safe_stat(value, fstatus));

	*fname = value;
	return OK;
}

error
env_restore(char *vars[], const char *const keep[], const char *const toss[])
{
	assert(vars && keep && toss);

	for (size_t i = 0; i < ENV_MAX && vars[i]; i++) {
		char *name = vars[i];
		char *value = NULL;
		char *sep = NULL;

		sep = strpbrk(vars[i], "=");
		if (!sep || sep == *(vars + i)) return ERR_VAR_INVALID;
		value = sep + 1;
		*sep = '\0';

		for (size_t j = 0; keep[j]; j++) {
			if (fnmatch(keep[j], name, FNM_PERIOD) != 0)
				continue;
			for (size_t k = 0; toss[k]; k++) {
				if (fnmatch(toss[k], name, FNM_PERIOD) == 0)
					goto next;
			}

			if (setenv(name, value, true) != 0) return ERR_SYS;
			break;
		}
		next:;
	}

	return OK;
}
