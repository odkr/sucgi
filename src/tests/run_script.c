/*
 * Test run_script.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

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
		char *suffix = NULL;
		char *handler = NULL;
		error rc = ERR;
		
		rc = str_vsplit(argv[i], "=", 2, &suffix, &handler);
		if (rc != OK) {
			die("run_script: str_vsplit returned %u.", rc);
		}
		if (suffix[0] == '\0') {
			die("run_script: suffix %d is empty.", i - 1);
		}
		if (!handler) {
			die("run_script: %s: no handler given.", argv[i]);
		}

		handlers[i - 2] = (struct pair) {
			.key = suffix,
			.value = handler
		};
	}
	handlers[i - 2] = (struct pair) {NULL, NULL};

	run_script(argv[1], handlers);

	return EXIT_FAILURE;
}
