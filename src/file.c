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

#include <assert.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#if defined(__linux__)
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
#include <linux/openat2.h>
#include <sys/syscall.h>
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0) */
#elif defined(__APPLE__) && defined(__MACH__)
#include <TargetConditionals.h>
#endif /* defined(__linux__) ... defined(__APPLE__) && defined(__MACH__) */

#include "err.h"
#include "str.h"


bool
file_is_exec(const struct stat *const fstatus)
{
	assert(fstatus);

	if (fstatus->st_uid != geteuid() && fstatus->st_gid != getegid()) {
		return fstatus->st_mode & S_IXOTH;
	}
	return (fstatus->st_mode & S_IXUSR) || (fstatus->st_mode & S_IXGRP);
}

bool
file_is_wexcl(const uid_t uid, const struct stat *const fstatus)
{
	assert(fstatus);

	return uid == fstatus->st_uid        &&
	       !(fstatus->st_mode & S_IWGRP) &&
	       !(fstatus->st_mode & S_IWOTH);
}

#if defined(__NR_openat2)
error
file_safe_open(const char *fname, const int flags, int *fd)
{
	
	struct open_how how = {
		.flags = (long long unsigned int) flags,
        	.resolve = RESOLVE_NO_SYMLINKS
	};
	long rc = -1;

	assert(fname);
	/*
	 * RESOLVE_NO_SYMLINKS ensures that fname contains no symlinks.
	 * See below for more details.
	 */
	rc = syscall(__NR_openat2, AT_FDCWD, fname,
	             &how, sizeof(struct open_how));
	if (rc < 0) return ERR_SYS;
	if (rc > INT_MAX) return ERR_CONV;

	*fd = (int) rc;
	return OK;
}

#elif defined(TARGET_OS_MAC) && TARGET_OS_MAC && defined(O_NOFOLLOW_ANY)
error
file_safe_open(const char *fname, const int flags, int *fd)
{
	assert(fname);
	/*
	 * - O_NOFOLLOW_ANY ensures that fname contains no symlinks.
	 * - Whether files are regular is checked in main.
	 * - By using file descriptors, race conditions should be avoided.
	 * - Changes to the path or the file's content are either permitted
	 *   (the user may run whatever code they wish) or outside the scope
	 *   of suCGI's threat model (suCGI is insecure if an attacker can
	 *   change user's files).
	 *
	 * TODO: Discuss that suCGI is insecure if an attacker can change user
	 * files in the threat model.
	 */
	/* Flawfinder: ignore; symlinks are not followed. */
	*fd = open(fname, flags | O_NOFOLLOW_ANY);
	if (*fd < 0) return ERR_SYS;
	return OK;
}

#else
#error suCGI requires Linux v5.6 or macOS v11.
#endif /* defined(__NR_openat2) ...
          defined(TARGET_OS_MAC) && TARGET_OS_MAC && defined(O_NOFOLLOW_ANY) */


error
file_safe_stat(const char *fname, struct stat *fstatus)
{
	struct stat buf;	/* Filesystem status of fname. */
	int fd = -1;		/* File descriptor. */
	int rc = -1;		/* Return code. */

	assert(fname);
	check(file_safe_open(fname, O_RDONLY | O_CLOEXEC, &fd));
	rc = fstat(fd, &buf);
	if (close(fd) != 0 || rc != 0) return ERR_SYS;
	/* Flawfinder: ignore; fstatus is guaranteed to be large enough. */
	if (fstatus) (void) memcpy(fstatus, &buf, sizeof(struct stat));
	return OK;
}
