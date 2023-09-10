/*
 * Header for mock.c.
 *
 * Copyright 2023 Odin Kroeger.
 *
 * This file is part of suCGI.
 *
 * suCGI is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * suCGI is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General
 * Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with suCGI. If not, see <https://www.gnu.org/licenses/>.
 */

#if !defined(TESTS_MOCK_UNISTD_H)
#define TESTS_MOCK_UNISTD_H


#include <sys/types.h>

#include "../../attr.h"
#include "../../params.h"


/*
 * Interposition
 */

#if !defined(__MACH__)
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

/* NOLINTBEGIN(bugprone-reserved-identifier,cert-dcl37-c); see attr.h. */
#if     !defined(__INTEL_COMPILER)                          \
    &&  (   (defined(__GNUC__) && __GNUC__ >= 7)            \
        ||  (defined(__clang__) && __clang_major__ >= 4)    )
#define _no_sanitize_all __attribute__((no_sanitize("all")))
#else
#define _no_sanitize_all
#endif
/* NOLINTEND(bugprone-reserved-identifier,cert-dcl37-c) */


/*
 * Functions
 */

_no_sanitize_all
uid_t mock_getuid(void);

_no_sanitize_all
uid_t mock_geteuid(void);

_no_sanitize_all
int mock_setuid(uid_t uid);

_no_sanitize_all
int mock_seteuid(uid_t euid);

_no_sanitize_all
int mock_setreuid(uid_t uid, uid_t euid);

_no_sanitize_all
gid_t mock_getgid(void);

_no_sanitize_all
gid_t mock_getegid(void);

_no_sanitize_all
int mock_setgid(gid_t gid);

_no_sanitize_all
int mock_setegid(gid_t egid);

_no_sanitize_all
int mock_setregid(gid_t gid, gid_t egid);

_no_sanitize_all
int mock_getgroups(int gidsetlen, gid_t *gidset);

_no_sanitize_all
int mock_setgroups(NGRPS_T ngroups, const gid_t *gidset);

#endif /* !defined(TESTS_MOCK_UNISTD_H) */
