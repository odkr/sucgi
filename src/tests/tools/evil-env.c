/*
 * Run a programme with a given environment, any environment.
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "../utils.h"

int
main (int argc, char **argv)
{
	const int envc = argc - 2;
	char *prog = argv[argc - 1];
	char **envv = calloc((size_t) envc, sizeof(char *)); 

	if (envc < 0) die("usage: evil-env VAR [VAR [...] PROG");
	if (!envv) die("evil-env: %s.", strerror(errno));

	/* cppcheck-suppress nullPointerRedundantCheck */
	/* Flawfinder: ignore */
	(void) memcpy(envv, &argv[1], (size_t) envc * sizeof(char *));

	// Flawfinder: ignore
	execve(prog, (char *[]) {prog, NULL}, envv);

	die("evil-env: exec %s: %s.", prog, strerror(errno));
}
