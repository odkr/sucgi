/*
 * Header for sigs.c.
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

#if !defined(TESTS_UTIL_SIGS_H)
#define TESTS_UTIL_SIGS_H

#include <stddef.h>

#include "../../attr.h"
#include "types.h"


/*
 * Macros
 */

/* Number of signals that, by default, cause abnormal termination. */
#define SIGS_NABORT 10

/* Number of signals that, by default, cause normal termination. */
#define SIGS_NTERM 7


/*
 * Global variables
 */

/* Signals that, by default, trigger abnormal termination. */
extern const int sigs_abort[SIGS_NABORT];

/* Signals that, by default, trigger normal termination. */
extern const int sigs_term[SIGS_NTERM];


/*
 * Functions
 */

/*
 * Register the given functions for the given signals and store the
 * functions that have previously been registered for those signals in
 * the memory area pointed to by "old". At least "ntraps" signal-function
 * pairs must be given, and the memory area pointed to by "old" must be
 * large enough to hold "ntraps" pairs. Supernumery pairs are ignored.
 *
 * Signal delivery is suspended while signal handlers are registered.
 *
 * sigs_trap also takes a pointer to an error handling function. If
 * an error occurs, this function is called after clean-up; if that
 * pointer is NULL, error handling is left to the caller.
 *
 * Return value:
 *     Zero      Success.
 *     Non-zero  Something went wrong; errno should be set.
 *
 * Errors:
 *     See sigfillset, sigprocmask, and sigaction.
 */
_read_only(2, 1) _write_only(3) _nonnull(2)
int sigs_trap(size_t ntraps, const Trap *traps, Trap *old, ErrorFn errh);

/*
 * Register the given action to handle the given signals.
 *
 * Otherwise the same as sigs_trap.
 *
 * Errors:
 *     See calloc and sigs_trap.
 */
_read_only(3, 2) _write_only(4) _nonnull(3)
int sigs_action(struct sigaction action, size_t nsignals, const int *signals,
                Trap *old, ErrorFn errh);

/*
 * Register the given handler to handle the given signals.
 *
 * Otherwise the same as sigs_action.
 */
_read_only(3, 2) _write_only(4) _nonnull(1, 3)
int sigs_handle(void (*handler)(int), size_t nsignals, const int *signals,
                Trap *old, ErrorFn errh);

/*
 * Reset the handler for the given signal to its default and then raise it.
 *
 * sigs_raise_default also takes a pointer to an error handling function.
 * See sigs_trap for details.
 *
 * Return value:
 *     Zero      Success.
 *     Non-zero  Something went wrong; errno should be set.
 *
 * Errors:
 *     See sigaction and raise.
 */
int sigs_raise_default(int signal, ErrorFn errh);

/*
 * Call the given function with the given value. If the function is
 * interrupted by a signal, call it again, and repeat calling it until
 * it is *not* interrupted by a signal. Return the return value of
 * the last call.
 */
_nodiscard
int sigs_retry_int(int (*func)(int), int val);

/*
 * The same sigs_retry_int but passes a pointer, not an integer.
 */
_nodiscard
int sigs_retry_ptr(int (*func)(void *), void *ptr);

#endif /* !defined(TESTS_UTIL_SIGS_H) */
