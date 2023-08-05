/*
 * Catch abnormal programme termination.
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

#define _XOPEN_SOURCE 700

#include <setjmp.h>

#include "abort.h"
#include "sigs.h"
#include "types.h"


/*
 * Global variables
 */

/* Point to jump to if a signal is caught by abort_catch. */
sigjmp_buf abort_env;

/* Last signal caught by abort_catch. */
volatile sig_atomic_t abort_signal = 0;


/*
 * Module variables
 */

/* Previous signal handlers. */
static Trap *old = NULL;


/*
 * Prototypes
 */

/*
 * Set the global "abort_signal" to the given signal and then siglongjmp to
 * the global "abort_env", using the signal as return value.
 */
static void catch(int signal);


/*
 * Functions
 */

static void
catch(const int signal)
{
    abort_signal = signal;
    siglongjmp(abort_env, signal);
}

int
abort_catch(const ErrorFn errh)
{
    abort_signal = 0;
    return sigs_handle(catch, SIGS_NABORT, sigs_abort, old, errh);
}

int
abort_reset(const ErrorFn errh)
{
    int retval = 0;

    if (old != NULL) {
        retval = sigs_trap(SIGS_NABORT, old, NULL, errh);
    }

    return retval;
}
