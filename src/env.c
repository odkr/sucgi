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

/* Default environment variables to keep. */
/* Flawfinder: ignore; array is constant. */
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
/* Flawfinder: ignore; array is constant. */
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
	char **env = environ;		/* Copy of the environ pointer. */
	
	var = NULL;
	environ = &var;

	if (!vars) return OK;
	for (size_t n = 0; n < VAR_MAX; n++) {
		vars[n] = env[n];
		if (!env[n]) return OK;
	}

	return ERR_VAR_MAX;
}

error
env_get_fname(const char *name, const mode_t ftype,
              /* Flawfinder: ignore; realpath should respect PATH_MAX. */
              char (*fname)[STR_MAX], struct stat *fstatus)
{
	struct stat buf;		/* Filesystem status of the file. */
	const char *value = NULL;	/* Value of the given variable. */

	assert(name && fname);
	/* Flawfinder: ignore; value is checked below, extensively. */
	value = getenv(name);
	if (!value) return ERR_VAR_UNDEF;
	if (str_eq(value, "")) return ERR_VAR_EMPTY;
	check(path_check_len(value));
	/* Flawfinder: ignore; fname is guaranteed to be >= PATH_MAX. */
	if (!realpath(value, *fname)) return ERR_SYS;
	check(file_safe_stat(*fname, &buf));
	if ((buf.st_mode & S_IFMT) != ftype) return ERR_FILE_TYPE;

	/* Flawfinder: ignore; fstatus is guaranteed to be large enough. */
	if (fstatus != NULL) (void) memcpy(fstatus, &buf, sizeof(struct stat));
	return OK;
}

error
env_sanitise (const char *const keep[], const char *const toss[])
{
	/* Flawfinder: ignore; env_clear respects VAR_MAX. */
	char *vars[VAR_MAX] = {NULL};	/* Backup of the environment. */

	/*
	 * Words of wisdom from the authors of suexec.c:
	 *
	 * > While cleaning the environment, the environment should be clean.
	 * > (E.g. malloc() may get the name of a file for writing debugging
	 * > info. Bad news if MALLOC_DEBUG_FILE is set to /etc/passwd.)
	 */

	check(env_clear(vars));

	/* Repopulate the environment. */
	for (size_t i = 0; (i < VAR_MAX) && (vars[i] != NULL); i++) {
		/* Flawfinder: ignore; str_split respects STR_MAX. */
		char name[STR_MAX] = {0};	/* Variable name. */
		char *value = NULL;		/* Variable value. */

		/*
		 * FIXME: value should be sanitised
		 *        before it is passed to setenv.
		 *
		 * This violates SEI CERT C recommendations STR02 and ENV03.
		 */
		check(str_split(vars[i], "=", &name, &value));
		if (str_eq(name, "") || !value) return ERR_VAR_INVALID;

		if (str_matchv(name, keep, 0) && !str_matchv(name, toss, 0)) {
			if (setenv(name, value, true) != 0) return ERR_SYS;
		}
	}

	return OK;
}
