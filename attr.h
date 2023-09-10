/*
 * C attributes.
 *
 * Copyright 2023 Odin Kroeger
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

#if !defined(ATTR_H)
#define ATTR_H

#if !defined(__GNUC__) || __GNUC__ < 3
/* cppcheck-suppress misra-c2012-21.1; idiomatic. */
#define __attribute__(attr)
#endif

/*
 * NOLINTBEGIN(bugprone-reserved-identifier,cert-dcl37-c)
 * See <https://github.com/llvm/llvm-project/issues/64130>.
 */

/*
 * NATTR must not excise __attribute__, because attributes are required to
 * compile against some standard libraries, e.g., that of macOS.
 */
#if !defined(NATTR) || !NATTR
#define _attr(...)          __attribute__((__VA_ARGS__))
#else
#define _attr(...)
#endif

#define _format(...)        _attr(format(__VA_ARGS__))
#define _noreturn           _attr(noreturn)
#define _unused             _attr(unused)

#if defined(__GNUC__) && \
    (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__  >= 3))
#define _nonnull(...)       _attr(nonnull(__VA_ARGS__))
#else
#define _nonnull(...)
#endif

#if defined(__GNUC__) && \
    (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__  >= 4))
#define _nodiscard          _attr(warn_unused_result)
#else
#define _nodiscard
#endif

#if defined(__GNUC__) && __GNUC__ >= 10
/* RATS: ignore; not a call to access. */
#define _access(...)        _attr(access(__VA_ARGS__))
#else
#define _access(...)
#endif

#define _read_only(...)     _access(read_only, __VA_ARGS__)
#define _read_write(...)    _access(read_write, __VA_ARGS__)
#define _write_only(...)    _access(write_only, __VA_ARGS__)

/* NOLINTEND(bugprone-reserved-identifier,cert-dcl37-c) */

#endif /* !defined(ATTR_H) */
