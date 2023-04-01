/*
 * System-dependent values.
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

#if !defined(COMPAT_H)
#define COMPAT_H

/* Type that getgrouplist takes and returns group IDs as. */
#if !defined(GRP_T)
#if defined(__APPLE__) && __APPLE__
#define GRP_T int
#else
#define GRP_T gid_t
#endif
#endif /* !defined(GRP_T) */

/* Type that setgroups takes the number of groups as. */
#if !defined(NGRPS_T)
#if defined(__linux__) && __linux__
#define NGRPS_T size_t
#else
#define NGRPS_T int
#endif
#endif /* !defined(NGRPS_T) */

#endif /* !defined(COMPAT_H) */
