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


#define MAX_HANDLERS 255U

int
main (int argc, char **argv)
{
	struct pair handlers[MAX_HANDLERS];
	int i = 0;

	if (argc < 2) {
		die("usage: run_script SCRIPT "
		    "[SUFFIX=HANDLER [SUFFIX=HANDLER [...]]]");
	}
	if ((unsigned long) argc > MAX_HANDLERS) {
		die("run_script: too many operands.");
	}

	for (i = 2; i < argc; i++) {
		/* Flawfinder: ignore */
		char suffix[STR_MAX] = {0};
		char *handler = NULL;
		error rc = ERR;

		rc = str_split(argv[i], "=", &suffix, &handler);
		if (rc != OK) die("run_script: str_split returned %u.", rc);

		char *key = strndup(suffix, STR_MAX - 1U);
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
