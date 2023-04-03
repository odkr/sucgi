dnl System-dependent values.
dnl
dnl Copyright 2023 Odin Kroeger
dnl
dnl This file is part of suCGI.
dnl
dnl suCGI is free software: you can redistribute it and/or modify it under
dnl the terms of the GNU General Public License as published by the Free
dnl Software Foundation, either version 3 of the License, or (at your option)
dnl any later version.
dnl
dnl suCGI is distributed in the hope that it will be useful, but WITHOUT
dnl ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
dnl FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
dnl for more details.
dnl
dnl You should have received a copy of the GNU General Public License along
dnl with suCGI. If not, see <https://www.gnu.org/licenses>.
dnl
include(`macros.m4')dnl
/*
 * System-dependent values.
 *
 * DO NOT EDIT THIS FILE. Your changes will get overwritten.
 * Edit compat.h.m4 instead and generate compat.h using ./configure.
 *
 * Copyright 2023 Odin Kroeger
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

#if !defined(COMPAT_H)
#define COMPAT_H

/* Type that getgrouplist takes and returns group IDs as. */
ifnempty(`__GRP_T', `dnl
#define GRP_T __GRP_T
', `dnl
#if defined(__APPLE__) && __APPLE__
#define GRP_T int
#else
#define GRP_T gid_t
#endif
')dnl

/* Type that setgroups takes the number of groups as. */
ifnempty(`__NGRPS_T', `dnl
#define NGRPS_T __NGRPS_T
', `dnl
#if defined(__linux__) && __linux__
#define NGRPS_T size_t
#else
#define NGRPS_T int
#endif
')dnl


#endif /* !defined(COMPAT_H) */
