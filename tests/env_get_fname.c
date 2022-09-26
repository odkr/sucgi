/*
 * Test env_get_fname.
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
	struct stat fstatus;
	char *var;
	const char *fname;
	char *ftype;
	mode_t fmode;
	enum error rc;

	if (argc != 3) die("usage: env_get_fname VAR FTYPE");

	var = argv[1];
	ftype = argv[2];

	switch (ftype[0]) {
		case 'f':
			fmode = S_IFREG;
			break;
		case 'd':
			fmode = S_IFDIR;
			break;
		default:
			die("env_get_fname: filetype must be 'f' or 'd'.");
	}

	rc = env_get_fname(var, fmode, &fname, &fstatus);
	switch (rc) {
		case SC_OK:
			break;
		case SC_ERR_SYS:
			die("env_get_fname: $%s: %s.", var, strerror(errno));
		case SC_ERR_FTYPE:
			die("env_get_fname: $%s: not of type %s.", var, ftype);
		case SC_ERR_ENV_LEN:
			die("env_get_fname: $%s: path too long.", var);
		case SC_ERR_ENV_NIL:
			die("env_get_fname: $%s: unset or empty.", var);
		default:
			die("env_get_fname: unexpected return code %u.", rc);
	}

	/* RATS: ignore. */
	(void) printf(
		"uid=%llu gid=%llu mode=%o size=%llu\n",
		(uint64_t) fstatus.st_uid,  (uint64_t) fstatus.st_gid,
		           fstatus.st_mode, (uint64_t) fstatus.st_size
	);

	return EXIT_SUCCESS;
}
