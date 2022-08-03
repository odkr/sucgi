/*
 * Test path_check_len.
 */

#include "../path.h"
#include "utils.h"


int
main (int argc, char **argv)
{
	const char *path = NULL;

	if (argc != 2) die("usage: path_check_len PATH");
	path = argv[1];

	return (int) path_check_len(path);
}
