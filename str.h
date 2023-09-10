/*
 * Header file for str.c.
 *
 * Copyright 2022 and 2023 Odin Kroeger.
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

#if !defined(STR_H)
#define STR_H

#include <stdlib.h>

#include "attr.h"
#include "types.h"


/*
 * Copy each byte of the given string to the memory area pointed to by the
 * parameter "dest", but stop when a null byte is encountered or the given
 * number of bytes has been copied, then terminate the copied value with a
 * null byte and return the length of the destination in the variable
 * pointed to by "destlen".
 *
 * The given memory area must be large enough to hold the result; that is,
 * it must be at least one byte larger than the given number of bytes.
 *
 * Return value:
 *      OK       Success.
 *      ERR_LEN  Source string is too long.
 */
_read_only(2, 1) _write_only(3) _write_only(4) _nonnull(2, 4)
Error str_copy(size_t nbytes, const char *src, size_t *destlen, char *dest);

/*
 * Search the given string for printf format specifiers and return the number
 * of specifiers found in the variable that the parameter "nspecs" points to
 * and the format characters that follow the '%' signs in the memory area
 * "fmtchars" points to; that memory area must be large enough to hold the
 * given maximum number of format specifiers.
 *
 * Caveats:
 *      Does not support $n% format modifiers.
 *
 * Return value:
 *      OK       Success.
 *      ERR_LEN  Too many format specifiers.
 */
_read_only(1) _write_only(3) _write_only(4, 2) _nonnull(1, 3, 4) _nodiscard
Error str_fmtspecs(const char *str, size_t maxnspecs,
                   size_t *nspecs, const char **fmtchars);

/*
 * Split the given string at the first occurence of any of the given bytes,
 * copy the substring up to, but not including, that byte to the memory area
 * pointed to by the parameter "head" and return a pointer to the substring
 * that starts after the separator in the memory area pointed to by the
 * parameter "tail".
 *
 * The memory area pointed to by "head" must be of the given size;
 * the substring before the separator must be shorter than that size.
 *
 * Neither substring is meaningful if an error occurs.
 *
 * Return value:
 *      OK       Success.
 *      ERR_LEN  The substring before the separator is too long.
 */
_read_only(1) _read_only(2) _write_only(4, 3) _nonnull(1, 2, 4, 5) _nodiscard
Error str_split(const char *str, const char *sep,
                size_t size, char *head, const char **tail);

#endif /* !defined(STR_H) */
