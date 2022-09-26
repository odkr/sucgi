/*
 * Run a programme with a given environment, any environment.
 *
 * env(1) implementations tend to accept only valid environment variables
 * (even though what they accept varies). evilenv accepts anything.
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "lib.h"

int
main (int argc, char **argv)
{
	char *prog = argv[1];

	if (argc < 3) die("usage: evilenv PROG VAR [VAR [...]]");

	/* RATS: ignore */
	execve(prog, (char *[]) {prog, NULL}, &argv[2]);

	die("evilenv: exec %s: %s.", prog, strerror(errno));
}
