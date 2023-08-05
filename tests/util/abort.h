/*
 * Header for abort.c.
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

#if !defined(TESTS_UTIL_ABORT_H)
#define TESTS_UTIL_ABORT_H

#include <sys/types.h>
#include <setjmp.h>
#include <signal.h>

#include "types.h"


/*
 * Global variables
 */

/* The environment to jump to if the programme was abnormally terminated. */
extern sigjmp_buf abort_env;

/* The last signal indicating abnormal termination that was caught. */
extern volatile sig_atomic_t abort_signal;


/*
 * Functions
 */

/*
 * Start to catch signals that would normally trigger abnormal termination.
 * If such a signal is caught, it is stored in the global "abort_signal" and
 * then siglongjmp to the global "abort_env". "abort_signal" is reset to 0
 * whenever the function "abort_catch" is called.
 *
 * abort_catch takes a pointer to an error handling function. If an error
 * occurs, this function is called after clean-up; if that pointer is NULL,
 * error handling is left to the caller.
 *
 * Return value:
 *     Zero      Success.
 *     Non-zero  Failure.
 *
 * Errors:
 *     See "sigs_handle".
 */
int abort_catch(ErrorFn errh);

/*
 * Restore the signal handlers that were in place for signals that normally
 * trigger abnormal termination before the most recent call to abort_catch.
 *
 * abort_reset takes a pointer to an error handling function.
 * See abort_catch for details.
 *
 * Return value:
 *     Zero      Success.
 *     Non-zero  Failure.
 *
 * Errors:
 *     See "sigs_handle".
 */
int abort_reset(ErrorFn errh);

#endif /* !defined(TESTS_UTIL_ABORT_H) */
