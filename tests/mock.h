/*
 * Header for mock.c.
 *
 * Copyright 2023 Odin Kroeger.
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

#if !defined(TESTS_LIB_MOCK_H)
#define TESTS_LIB_MOCK_H


#include <sys/types.h>

#include "../cattr.h"
#include "../compat.h"


/*
 * Interposition
 */

#if defined(__linux__) && __linux__
#define mockgetuid getuid
#define mockgeteuid geteuid
#define mocksetuid setuid
#define mockseteuid seteuid
#define mocksetreuid setreuid
#define mockgetgid getgid
#define mockgetegid getegid
#define mocksetgid setgid
#define mocksetegid setegid
#define mockseteuid seteuid
#define mocksetregid setregid
#define mockgetgroups getgroups
#define mocksetgroups setgroups
#endif


/*
 * Attributes
 */

#if     !defined(__INTEL_COMPILER)                          \
    &&  (   (defined(__GNUC__) && __GNUC__ >= 5)            \
        ||  (defined(__clang__) && __clang_major__ >= 4))
#define NO_SANITIZE __attribute__((no_sanitize("all")))
#else
#define NO_SANITIZE
#endif



/*
 * Functions
 */

NO_SANITIZE
uid_t mockgetuid(void);

NO_SANITIZE
uid_t mockgeteuid(void);

NO_SANITIZE
int mocksetuid(uid_t uid);

NO_SANITIZE
int mockseteuid(uid_t euid);

NO_SANITIZE
int mocksetreuid(uid_t uid, uid_t euid);

NO_SANITIZE
gid_t mockgetgid(void);

NO_SANITIZE
gid_t mockgetegid(void);

NO_SANITIZE
int mocksetgid(gid_t gid);

NO_SANITIZE
int mocksetegid(gid_t egid);

NO_SANITIZE
int mocksetregid(gid_t gid, gid_t egid);

NO_SANITIZE
int mockgetgroups(int gidsetlen, gid_t *gidset);

NO_SANITIZE
int mocksetgroups(const NGRPS_T ngroups, const gid_t *const gidset);


#endif /* !defined(TESTS_LIB_MOCK_H) */
