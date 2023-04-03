dnl Build information.
dnl
dnl Copyright 2022 and 2023 Odin Kroeger
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
 * Build information.
 *
 * DO NOT EDIT THIS FILE. Your changes will get overwritten.
 * Edit build.h.m4 instead and generate build.h using ./configure.
 *
 * Copyright 2022 and 2023 Odin Kroeger
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

#if !defined(BUILD_H)
#define BUILD_H

ifnempty(`__have_features_h', `dnl
#include <features.h>

')dnl

/* Libc. */
#if defined(__GLIBC__)
#define LIBC "glibc"
#elif defined(__GNU_LIBRARY__)
#define LIBC "glibc"
#elif defined(__KLIBC__)
#define LIBC "klibc"
#elif defined(__UCLIBC__)
#define LIBC "uClibc"
#elif defined(__APPLE__)
#define LIBC "Apple"
#endif

/* Toolchain. */
ifnempty(`__CC', `dnl
#define CC "__CC"'
)dnl
ifnempty(`__CFLAGS', `dnl
#define CFLAGS "__CFLAGS"
')dnl
ifnempty(`__AR', `dnl
#define AR "__AR"'
)dnl
ifnempty(`__ARFLAGS', `dnl
#define ARFLAGS "__ARFLAGS"'
)dnl
ifnempty(`__LDFLAGS', `dnl
#define LDFLAGS "__LDFLAGS"'
)dnl
ifnempty(`__LDLIBS', `dnl
#define LDFLAGS "__LDLIBS"'
)dnl


#endif /* !defined(BUILD_H) */
