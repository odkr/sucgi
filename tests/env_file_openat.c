/*
 * Test env_file_openat.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "../env.h"
#include "../err.h"
#include "../tools/lib.h"


int
main (int argc, char **argv)
{
	char *jail;		/* Jail directory. */
	char *var;		/* Variable name. */
	struct stat fstatus;	/* File's filesystem status. */
	const char *fname;	/* Filename. */
	int fd;			/* File descriptor. */
	enum error rc;		/* Return code. */

	if (argc != 3) die("usage: env_file_openat JAIL VAR");

	jail = argv[1];
	var = argv[2];

	rc = env_file_openat(jail, var, O_RDONLY | O_CLOEXEC, &fname, &fd);
	switch (rc) {
		case SC_OK:
			break;
		case SC_ERR_SYS:
			die("env_file_openat: $%s: %s.", var, strerror(errno));
		case SC_ERR_ENV_LEN:
			die("env_file_openat: $%s: path too long.", var);
		case SC_ERR_ENV_MAL:
			die("env_file_openat: $%s: not in %s.", var, jail);
		case SC_ERR_ENV_NIL:
			die("env_file_openat: $%s: unset or empty.", var);
		default:
			die("env_file_openat: unexpected return code %u.", rc);
	}

	if (fstat(fd, &fstatus) != 0) {
		die("env_file_openat: stat %s: %s.", fname, strerror(errno));
	}

	/* RATS: ignore. */
	(void) printf(
		"uid=%llu gid=%llu mode=%o size=%llu\n",
		(uint64_t) fstatus.st_uid,  (uint64_t) fstatus.st_gid,
		           fstatus.st_mode, (uint64_t) fstatus.st_size
	);

	return EXIT_SUCCESS;
}
