/*
 * Header for trap.c.
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

#if !defined(TESTS_LIB_TRAP_H)
#define TESTS_LIB_TRAP_H

#include <stdbool.h>

#include "../../attr.h"


/*
 * Globals
 */

/* Last signal caught. */
extern int trapped;


/*
 * Functions
 */

/*
 * Trap the first NSIGNALS signal numbers in SIGNOS in TRAPPED.
 * SIGNOS must have at least NSIGNALS elements.
 *
 * Return value:
 *      0  Success.
 *     -1  sigaction failed, errno should be set.
 */
int trapsigs(size_t nsignals, const int *signos);

/*
 * If TRAPPED is greater than 0, run CLEANUP, then restore
 * the default signal handler and re-raise the signal.
 *
 * Side-effects:
 *    Prints a message to STDERR.
 */
void traphandle(void (*cleanup)(void));


#endif /* !defined(TESTS_LIB_TRAP_H) */
