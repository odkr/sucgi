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
		/* cppcheck-suppress cert-STR05-C; not a constant. */
		/* Flawfinder: ignore */
		char suffix[STR_MAX] = "";
		char *handler = NULL;
		error rc = ERR;

		rc = str_split(argv[i], "=", &suffix, &handler);
		if (rc != OK) die("run_script: str_split returned %u.", rc);

		char *key = strndup(suffix, STR_MAX - 1);
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
