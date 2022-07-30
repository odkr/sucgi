/*
 * Test run_script.
 */

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../err.h"
#include "../str.h"
#include "../utils.h"
#include "utils.h"

#include <stdio.h>

int
main (int argc, char **argv)
{
	struct pair handlers[argc];
	int i = 0;

	if (argc < 2) {
		die("usage: run_script SCRIPT "
		    "[SUFFIX=HANDLER [SUFFIX=HANDLER [...]]]");
	}

	for (i = 2; i < argc; i++) {
		str4096 suffix = "";
		char *handler = NULL;
		error rc = ERR;

		rc = str_split(argv[i], "=", &suffix, &handler);
		if (rc != OK) {
			die("run_script: str_split returned %u.", rc);
		} else if ('\0' == suffix[0]) {
			die("run_script: suffix %d is empty.", i - 1);
		}

		char *key = strndup(suffix, STR_MAX_LEN);
		if (!key) die("strndup: %s.", strerror(errno));

		handlers[i - 2] = (struct pair) {
			.key = key,
			.value = handler
		};
	}
	handlers[i - 2] = (struct pair) {NULL, NULL};

	run_script(argv[1], handlers);

	return EXIT_FAILURE;
}
