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

/*
 * The patterns below have been adopted from Apache's suEXEC.
 * There should be no need to adapt them.
 */
char *const env_keep[] = {
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

/*
 * The pattern below has been adopted from Apache's suEXEC.
 * There should be no need to adapt this list.
 */
char *const env_toss[] = {
	"HTTP_PROXY",
	NULL	/* Array terminator. DO NOT REMOVE. */
};


/*
 * Functions
 */

error
env_clear(char ***vars)
{
	char **env = environ;	/* Backup of the environment. */
	size_t n = 0;

	/* FIXME: Test if this works with the glibc. */
	environ = NULL;

	environ = calloc(1, sizeof(char *));
	if (!environ) return ERR_SYS;
	*environ = NULL;
	if (!vars) return OK;

	*vars = calloc(ENV_MAX, sizeof(char *));
	if (!*vars) return ERR_SYS;
	for (n = 0; env[n]; n++) {
		/* FIXME: Neither tested in env_clear nor main.sh. */
		if (n == ENV_MAX) return ERR_ENV_MAX;
		(*vars)[n] = env[n];
	}
	return OK;
}

error
env_get_fname(const char *name, char **fname, struct stat **fstatus)
{
	char *value = NULL;

	assert(name && fname);
	// The value is checked below, extensively.
	// flawfinder: ignore
	value = getenv(name);
	if (!value) return ERR_VAR_UNDEF;
	if (*value == '\0') return ERR_VAR_EMPTY;
	reraise(path_check_len(value));
	reraise(file_safe_stat(value, fstatus));

	*fname = value;
	return OK;
}

error
env_restore(const char *const *vars,
            char *const *const keep,
	    char *const *const toss)
{
	assert(vars && keep && toss);

	for (; *vars; vars++) {
		char *name = NULL;
		char *value = NULL;

		reraise(str_vsplit(*vars, "=", 2, &name, &value));
		if (name[0] == '\0' || !value) return ERR_VAR_INVALID;

		for (char *const *kv = keep; *kv; kv++) {
			if (fnmatch(*kv, name, FNM_PERIOD) != 0)
				continue;
			for (char *const *tv = toss; *tv; tv++) {
				if (fnmatch(*tv, name, FNM_PERIOD) == 0)
					goto next;
			}

			if (setenv(name, value, true) != 0) return ERR_SYS;
			break;
		}
		next:;
	}

	return OK;
}
