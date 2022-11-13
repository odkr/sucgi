/*
 * Headers for file.c.
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

#if !defined(FILE_H)
#define FILE_H

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

#include <sys/stat.h>
#include <stdbool.h>

#include "attr.h"
#include "error.h"
#include "types.h"


/*
 * Costants
 */

#if defined(__NR_openat2)
#define HAVE_OPENAT2 1
#define HAVE_NOFOLLOW_ANY 0
#define file_sec_open file_sec_open__linux__
#elif defined(O_NOFOLLOW_ANY)
#define HAVE_OPENAT2 0
#define HAVE_NOFOLLOW_ANY 1
#define file_sec_open file_sec_open__macos__
#else
#define HAVE_OPENAT2 0
#define HAVE_NOFOLLOW_ANY 0
#define file_sec_open file_sec_open__posix__
#endif


/*
 * Functions
 */

/*
 * Open FNAME with FLAGS and store its file descriptor in FD.
 * FNAME must not contain symbolic links.
 *
 * FD is closed on exit.
 *
 * Return value:
 *      OK        Success.
 *      ERR_CNV*  File descriptor is too large (Linux only).
 *      ERR_OPEN  open(2)/openat2(2) failed.
 *
 *      Errors marked with an asterisk should be impossible.
 */
__attribute__((nonnull(1, 3), warn_unused_result))
enum retval file_sec_open__linux__(const char *const fname, const int flags,
                                   int *const fd);

__attribute__((nonnull(1, 3), warn_unused_result))
enum retval file_sec_open__macos__(const char *const fname, const int flags,
                                   int *const fd);

__attribute__((nonnull(1, 3), warn_unused_result))
enum retval file_sec_open__posix__(const char *const fname, const int flags,
                                   int *const fd);


/*
 * Check if FSTATUS indicates that the current user has execute permissions.
 */
__attribute__((pure, warn_unused_result))
bool file_is_exe(const struct stat fstatus);

/*
 * Check if FSTATUS indicates that only UID has write permissions.
 */
__attribute__((pure, warn_unused_result))
bool file_is_wexcl(const uid_t uid, const struct stat fstatus);

/*
 * Close FD, but do not set errno on failures.
 */
void file_vclose(int fd);



#endif /* !defined(FILE_H) */
