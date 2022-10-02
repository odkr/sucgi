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

#define _ISOC99_SOURCE
#define _POSIX_C_SOURCE 200809L
#define _BSD_SOURCE
#define _DARWIN_C_SOURCE
#define _DEFAULT_SOURCE

#if !defined(_FORTIFY_SOURCE)
#define _FORTIFY_SOURCE 3
#endif /* !defined(_FORTIFY_SOURCE) */

#if defined(__linux__)
#include <linux/version.h>
#if defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
#include <linux/openat2.h>
#include <sys/syscall.h>
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0) */
#endif /* defined(LINUX_VERSION_CODE) && defined(KERNEL_VERSION) */
#endif /* defined(__linux__) */

#include <assert.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "file.h"
#include "err.h"
#include "str.h"


bool
file_is_exec(const struct stat *const fstatus)
{
	uid_t euid = geteuid();			/* Effective UID. */
	gid_t egid = getegid();			/* Effective GID. */
	uid_t fuid = fstatus->st_uid;		/* Owner UID. */
	gid_t fgid = fstatus->st_gid;		/* Owner GID. */
	mode_t fmode = fstatus->st_mode;	/* Permissions. */

	if (fuid != euid && fgid != egid) return (fmode & S_IXOTH);

	return (fuid == euid && (fmode & S_IXUSR)) ||
	       (fgid == egid && (fmode & S_IXGRP));
}


bool
file_is_wexcl(const uid_t uid, const struct stat *const fstatus)
{
	mode_t fmode = fstatus->st_mode;        /* Permissions. */

	return  (fstatus->st_uid == uid) &&
	       !(fmode & S_IWGRP)        &&
	       !(fmode & S_IWOTH);
}

/*
 * - The RESOLVE_NO_SYMLINKS flag to openat2(2) (Linux)/the O_NOFOLLOW_ANY
 *   flag to open(2) (macOS) ensures that paths opened by file_safe_*
 *   functions contain no symlinks.
 * - file_safe_* functions operate on file descriptors, so there should be no
 *   pernicious TOCTOU gaps (the remaining TOCTOU gaps should be harmless).
 * - file_safe_* functions are only called via env_get_fname and
 *   path_check_wexcl. env_get_fname makes sure that:
 *   - files are of a given type (directory, regular, etc.).
 *   - paths have been canonicalised.
 *   path_check_wexcl should only be passed canonical paths.
 */

#if defined(__NR_openat2)
enum error
file_safe_open(const char *const fname, const int flags, int *const fd)
{
	struct open_how how;	/* Flags to openat2(2). */
	long rc;		/* Return code. */

	assert(*fname != '\0');

	(void) memset(&how, 0, sizeof(how));
	how.flags = (uint64_t) flags;
	how.resolve = RESOLVE_NO_SYMLINKS | RESOLVE_NO_MAGICLINKS;

	rc = syscall(__NR_openat2, AT_FDCWD, fname, &how, sizeof(how));
	if (rc < 0) return ERR_SYS;
	if (rc > INT_MAX) return ERR_CNV;

	*fd = (int) rc;
	return OK;
}

#elif defined(O_NOFOLLOW_ANY)
enum error
file_safe_open(const char *const fname, const int flags, int *const fd)
{
	assert(*fname != '\0');

	/* RATS: ignore; see above. */
	*fd = open(fname, flags | O_NOFOLLOW_ANY);
	if (*fd < 0) return ERR_SYS;
	return OK;
}

#else
#error "suCGI requires openat2 or O_NOFOLLOW_ANY."
#endif /* defined(__NR_openat2) ...
          defined(O_NOFOLLOW_ANY) */


enum error
file_safe_stat(const char *const fname, struct stat *const fstatus)
{
	int fd;		/* File descriptor. */
	int rc;		/* Return code. */

	assert(*fname != '\0');

	try(file_safe_open(fname, O_RDONLY | O_CLOEXEC, &fd));
	rc = fstat(fd, fstatus);		
	if ((close(fd) != 0) || (rc != 0)) return ERR_SYS;

	return OK;
}
