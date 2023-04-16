/*
 * Useful regular expressions.
 *
 * Copyright 2022 and 2023 Odin Kroeger.
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

#if !defined(PREGS_H)
#define PREGS_H


/* Pattern that matches X.509 distinguished name components. */
#define PREG_X509 "(C|ST|L|O|OU|CN|T|I|G|S|D|UID|Email)"

/* Pattern that matches decimal numbers equal to or greater than zero. */
#define PREG_N "(0|[1-9][0-9]*)"


#endif /* !defined(PREGS_H) */
