/*
 * Test env_file_open.
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
	struct stat fstatus;	/* Filesystem metadata. */
	const char *fname;	/* Filename. */
	int fd;			/* File descriptor. */
	enum error rc;		/* Return code. */

	if (argc != 3) die("usage: env_file_open JAIL VAR");

	jail = argv[1];
	var = argv[2];

	rc = env_file_open(jail, var, O_RDONLY | O_CLOEXEC, &fname, &fd);
	switch (rc) {
		case OK:
			break;
		case ERR_SYS:
			die("env_file_open: $%s: %s.", var, strerror(errno));
		case ERR_ENV_LEN:
			die("env_file_open: $%s: path too long.", var);
		case ERR_ENV_MAL:
			die("env_file_open: $%s: not in %s.", var, jail);
		case ERR_ENV_NIL:
			die("env_file_open: $%s: unset or empty.", var);
		default:
			die("env_file_open: unexpected return code %u.", rc);
	}

	if (fstat(fd, &fstatus) != 0) {
		die("env_file_open: stat %s: %s.", fname, strerror(errno));
	}

	/* RATS: ignore. */
	(void) printf("uid=%llu gid=%llu mode=%o size=%llu\n",
	              (unsigned long long) fstatus.st_uid,
	              (unsigned long long) fstatus.st_gid,
	                                   fstatus.st_mode,
	              (unsigned long long) fstatus.st_size);

	return EXIT_SUCCESS;
}
