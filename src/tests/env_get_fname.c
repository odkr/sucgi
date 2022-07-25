/*
 * Test env_get_fname.
 */

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

#include "../env.h"
#include "../err.h"
#include "utils.h"


int
main (int argc, char **argv)
{
	const char *name = NULL;
	char *val = NULL;

	switch (argc) {
		case 2:
			name = argv[1];
			break;
		default:
	 		die("usage: env_get_fname VAR");
	}

	return (int) env_get_fname(name, &val, NULL);
}
