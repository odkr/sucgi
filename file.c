/*
 * File handling for suCGI.
 *
 * Copyright 2022 Odin Kroeger
 *
 * This file is part of suCGI.
 *
 * suCGI is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * suCGI is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with suCGI. If not, see <https://www.gnu.org/licenses>.
 */

#if !defined(_FORTIFY_SOURCE)
#define _FORTIFY_SOURCE 3
#endif

#if defined(__linux__) && __linux__
#include <linux/version.h>
#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
#include <linux/openat2.h>
#include <sys/syscall.h>
#include <sys/types.h>
#endif
#endif
#endif /* defined(__linux__) && __linux__ */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "file.h"
#include "max.h"
#include "str.h"
#include "types.h"


/*
 * Constants
 */

/* Flags to pass to every call to an open(2) function. */
#define BASE_OFLAGS ( O_NOFOLLOW | O_CLOEXEC )

/* Flags to pass to every pass to open(2) when opening a directory. */
#define DIR_OFLAGS ( O_DIRECTORY | O_RDONLY )


/*
 * System-dependent functions
 */

/* Linux >= v5.6 implementation of file_sec_open */
#if HAVE_OPENAT2

enum retval
file_sec_open__linux__(const char *const fname, const int flags,
                       int *const fd)
{
	struct open_how how;	/* Flags to openat2(2). */
	long rc;		/* Return code. */

	assert(*fname != '\0');

	(void) memset(&how, 0, sizeof(how));
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
	how.flags = flags | O_CLOEXEC;
#pragma GCC diagnostic pop
	how.resolve = RESOLVE_NO_SYMLINKS | RESOLVE_NO_MAGICLINKS;

	errno = 0;
	rc = syscall(__NR_openat2, AT_FDCWD, fname, &how, sizeof(how));
	if (rc < 0)
		return ERR_OPEN;
	if (rc > INT_MAX)
		return ERR_CNV;

	*fd = (int) rc;
	return OK;
}

/* XNU >= v7195.50.7.100.1 implementation of file_sec_open */
#elif HAVE_NOFOLLOW_ANY /* ... && !HAVE_OPENAT2 */

enum retval
file_sec_open__macos__(const char *const fname, const int flags,
                       int *const fd)
{
	assert(*fname != '\0');

	errno = 0;
	/* RATS: ignore; safe-ish and used safely. */
	*fd = open(fname, flags | O_CLOEXEC | O_NOFOLLOW_ANY);
	if (*fd < 0)
		return ERR_OPEN;

	return OK;
}

#endif

/* POSIX.1-2008 implementation of file_sec_open */
enum retval
file_sec_open__posix__(const char *const fname, const int flags,
                       int *const fd)
{
	/* RATS: ignore; writes to tokens respect MAX_FNAME. */
	char tokens[MAX_FNAME];		/* Copy of fname for strtok. */
	char *cur;			/* Current path segment. */
	int file;			/* Current file. */

	assert(*fname != '\0');

	if (str_cp(MAX_FNAME - 1U, fname, tokens) != OK)
		return ERR_LEN;

	file = AT_FDCWD;
	if (fname[0] == '/') {
		int flgs;
		
		flgs = BASE_OFLAGS;
		if (fname[1] == '\0')
			flgs |= O_DIRECTORY | flags;
		else
			flgs |= DIR_OFLAGS;
		
		/* RATS: ignore; filename is a string literal. */
		file = open("/", flgs);
		if (file < 0)
			return ERR_OPEN;
	}

	cur = strtok(tokens, "/");
	while (cur != NULL) {
		char *next;	/* Next path segment. */
		int flgs;	/* Flags for openat. */
		int dir;	/* Current directory. */

		next = strtok(NULL, "/");
		dir = file;

		flgs = BASE_OFLAGS;
		if (next == NULL)
			flgs |= flags;
		else
			flgs |= DIR_OFLAGS;

		file = openat(dir, cur, flgs);

		if (file < 0) {
			file_vclose(dir);
			return ERR_OPEN;
		}

		if (dir != AT_FDCWD)
			if (close(dir) != 0) {
				file_vclose(file);
				return ERR_CLOSE;
			}

		cur = next;
	}

	*fd = file;
	return OK;
}


/*
 * Portable functions
 */

bool
file_is_exe(const struct stat fstatus)
{
	mode_t perm = fstatus.st_mode;		/* Permissions. */

	if (fstatus.st_uid == geteuid())
		return (perm & S_IXUSR) != 0;
	if (fstatus.st_gid == getegid())
		return (perm & S_IXGRP) != 0;
	return (perm & S_IXOTH) != 0;
}

bool
file_is_wexcl(const uid_t uid, const struct stat fstatus)
{
	mode_t perm = fstatus.st_mode;		/* Permissions. */

	return fstatus.st_uid == uid &&
	       (perm & S_IWGRP)	== 0 &&
	       (perm & S_IWOTH) == 0;
}

void file_vclose(int fd)
{
	int err;				/* Backup of errno. */
	
	err = errno;
	(void) close(fd);
	errno = err;
}
