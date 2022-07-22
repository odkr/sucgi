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

#include <fcntl.h>
#include <libgen.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>

#include "err.h"
#include "str.h"

bool
file_is_exec(const struct stat *const fstatus)
{
	if (fstatus->st_uid != geteuid() && fstatus->st_gid != getegid()) {
		return fstatus->st_mode & S_IXOTH;
	}
	return (fstatus->st_mode & S_IXUSR) || (fstatus->st_mode & S_IXGRP);
}

bool
file_is_wexcl(const uid_t uid, const gid_t gid,
              const struct stat *const fstatus)
{
	return     fstatus->st_uid == uid
		&& (fstatus->st_gid == gid || !(fstatus->st_mode & S_IWGRP))
		&& !(fstatus->st_mode & S_IWOTH);
}

#if defined(HAVE_OPENAT2) && HAVE_OPENAT2 && \
    defined(HAVE_SYSCALL) && HAVE_SYSCALL
#include <linux/openat2.h>
#include <sys/syscall.h>

#if defined(__NR_openat2) && __NR_openat2
enum code
file_safe_open(const char *fname, const int flags, int *fd)
{
	struct open_how how = {
		.flags = (long long unsigned int) flags,
        	.resolve = RESOLVE_NO_SYMLINKS
	};

	/* RESOLVE_NO_SYMLINKS ensures that fname contains no symlinks. */
	*fd = (int) syscall(__NR_openat2, AT_FDCWD, fname,
	                    &how, sizeof(struct open_how));
	if (*fd < 0) return ERR_SYS;
	return OK;
}
#else
#error openat2 is not available.
#endif /* __NR_openat2. */

#elif O_NOFOLLOW_ANY
enum code
file_safe_open (const char *fname, const int flags, int *fd)
{
	// - O_NOFOLLOW_ANY ensures that fname contains no symlinks.
	// - Whether files are regular is checked in main.
	// - By using file descriptors,
	//   race conditions should be avoided.
	// - Changes to the path or the file's content are either permitted
	//   (the user may run whatever code they wish) or outside the scope
	//   of suCGI's threat model (suCGI is insecure if an attacker can
	//   change user's files. FIXME: Discuss this in the threat model.)
	//
	// flawfinder: ignore
	*fd = open(fname, flags | O_NOFOLLOW_ANY);
	if (*fd < 0) return ERR_SYS;
	return OK;
}
#else
#error Neither openat2 nor O_NOFOLLOW_ANY are available.
#endif /* HAVE_OPENAT2 or O_NOFOLLOW_ANY. */

enum code
file_safe_stat(const char *fname, struct stat **fstatus)
{
	struct stat *buf = NULL;
	int fd = -1;
	int rc = -1;

	buf = malloc(sizeof(struct stat));
	if (!buf) return ERR_SYS;
	reraise(file_safe_open(fname, O_RDONLY | O_CLOEXEC, &fd));
	rc = fstat(fd, buf);
	if (close(fd) != 0 || rc != 0) return ERR_SYS;

	if (fstatus) *fstatus = buf;
	return OK;
}
