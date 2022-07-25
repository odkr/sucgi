/*
 * Test run_script.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "../str.h"
#include "../utils.h"
#include "utils.h"

int
main (int argc, char **argv)
{
	const char *script = NULL;
	const char *pairs = NULL;
	char **pairv = NULL;
	enum code rc = ERR;

	switch (argc) {
		case 2:
			script = argv[1];
			pairs = ".sh=sh";
			break;
		case 3:
			script = argv[1];
			pairs = argv[2];
			break;
		default:
			die("usage: run_script SCRIPT [HANDLER]");
	}

	rc = str_words(pairs, &pairv);
	if (rc != OK) die("run_script: str_words returned %u.", rc);

	run_script(script, (const char **) pairv);

	return EXIT_FAILURE;
}
