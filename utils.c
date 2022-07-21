/*
 * Run scripts for suCGI.
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
#include <string.h>
#include <unistd.h>
#include <syslog.h>

#include "err.h"
#include "str.h"
#include "utils.h"


void
run_script (const char *const script, char **pairs)
{
	char *suffix = NULL;	/* File suffix. */
	char **pair = NULL;	/* Current pair. */
	int i = 0;		/* No. of current pair. */

	suffix = strrchr(script, '.');
	if (!suffix) error("%s: No filename suffix.", script);

	for (pair = pairs; *pair; pair++) {
		enum code rc = ERR;	/* Return code. */
		char *inter = NULL;	/* Interpreter .*/
		char *ftype = NULL;	/* Suffix associated with inter. */

		i++;
		rc = str_vsplit(*pair, "=", 2, &ftype, &inter);
		switch (rc) {
			case OK:
				break;
			case ERR_SYS:
				error("Interpreter %d: %s.",
				      i, strerror(errno));
				break;
			default:
				error("%s: line %d: str_words returned %d.",
				      __FILE__, __LINE__ - 10, rc);
		}

		if (ftype[0] == '\0') {
			error("Script type %d: No filename suffix.", i);
		}
		if (ftype[0] != '.' || str_eq(ftype, ".")) {
			error("Script type %d: Weird filename suffix.", i);
		} 

		if (!inter || inter[0] == '\0') {
			error("Script type %d: No interpreter given.", i);
		}

		if (str_eq(suffix, ftype)) {
			// suCGI's whole point is to do this safely.
			// flawfinder: ignore.
			execlp(inter, inter, script, NULL);
		};
	}

	error("Filename suffix %s: No interpreter registered.", suffix);
}
