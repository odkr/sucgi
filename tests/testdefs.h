/*
 * Definitions for tests.
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

#if !defined(TESTS_TESTDEFS_H)
#define TESTS_TESTDEFS_H

/* Exit statuses for tests. */
enum status {
	T_PASS = 0,	/* All tests passed. */
	T_SKIP = 75,	/* Some tests were skipped. */
	T_FAIL = 70,	/* At least one test failed. */
	T_ERR = 1	/* Some error occurred. */
};


#endif /* defined(TESTS_TESTDEFS_H) */
