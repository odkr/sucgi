/*
 * Signal handling for tests.
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

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>

#include "types.h"
#include "sigs.h"


/*
 * Macros
 */

/* Number of signals on the system. */
#if !defined(NSIG)
#if defined(_NSIG)
#define NSIG _NSIG
#else
#define NSIG 128
#endif
#endif


/*
 * Global variables
 */

/* Signals that trigger abnormal termination by default. */
const int sigs_abort[SIGS_NABORT] = {
    SIGABRT, SIGBUS, SIGFPE, SIGILL, SIGQUIT,
    SIGSEGV, SIGSYS, SIGTRAP, SIGXCPU, SIGXFSZ
};

/* Signals that trigger normal termination by default. */
const int sigs_term[SIGS_NTERM] = {
    SIGALRM, SIGHUP, SIGINT, SIGPIPE, SIGTERM,
    SIGUSR1, SIGUSR2
};


/*
 * Functions
 */

int
sigs_trap(const size_t ntraps, const Trap *const traps, Trap *const old,
          const ErrorFn errh)
{
    assert(traps != NULL);

    int retval = 0;
    int fatalerr = 0;

    sigset_t allsigs;
    errno = 0;
    retval = sigfillset(&allsigs);
    if (retval != 0) {
        /* NOTREACHED */
        if (errh != NULL) {
            errh(EXIT_FAILURE, "%s:%d: sigfillset", __FILE__, __LINE__);
        }
        return retval;
    }

    sigset_t oldmask;
    errno = 0;
    retval = sigprocmask(SIG_SETMASK, &allsigs, &oldmask);
    if (retval != 0) {
        /* NOTREACHED */
        if (errh != NULL) {
            errh(EXIT_FAILURE, "%s:%d: sigprocmask", __FILE__, __LINE__);
        }
        return retval;
    }

    for (size_t i = 0; i < ntraps; ++i) {
        const struct sigaction *const act = &traps[i].action;
        struct sigaction *ptr = NULL;
        const int sig = traps[i].signal;

        if (old != NULL) {
            ptr = &old[i].action;
            old[i].signal = sig;
        }

        errno = 0;
        retval = sigaction(sig, act, ptr);
        if (retval != 0) {
            fatalerr = errno;
            break;
        }
    }

    errno = 0;
    if (sigprocmask(SIG_SETMASK, &oldmask, NULL) != 0) {
        /* NOTREACHED */
        if (retval == 0) {
            if (errh != NULL) {
                errh(EXIT_FAILURE, "%s:%d: sigprocmask", __FILE__, __LINE__);
            }
            return -1;
        }
    }

    errno = fatalerr;
    if (retval != 0 && errh != NULL) {
        /* NOTREACHED */
        errh(EXIT_FAILURE, "%s: %d: sigaction", __FILE__, __LINE__);
    }

    return retval;
}

int
/* cppcheck-suppress misra-c2012-8.7; sigs_action could be used externally. */
sigs_action(const struct sigaction action,
            const size_t nsignals, const int *const signals,
            Trap *const old, const ErrorFn errh)
{
    int retval = -1;

    Trap traps[NSIG];
    for (size_t i = 0; i < nsignals; ++i) {
        traps[i] = (Trap) {.signal = signals[i], .action = action};
    }

    retval = sigs_trap(nsignals, traps, old, errh);

    return retval;
}

int
sigs_handle(void (*handler)(int),
            const size_t nsignals, const int *const signals,
            Trap *const old, const ErrorFn errh)
{
    assert(handler != NULL);
    const struct sigaction action = {.sa_handler = handler};
    return sigs_action(action, nsignals, signals, old, errh);
}

int
sigs_raise_default(const int signal, const ErrorFn errh)
{
    static const struct sigaction dflaction = {.sa_handler = SIG_DFL};
    errno = 0;
    if (sigaction(signal, &dflaction, NULL) != 0) {
        if (errh != NULL) {
            errh(EXIT_FAILURE, "%s:%d: sigaction", __FILE__, __LINE__);
        }
        return -1;
    }

    errno = 0;
    if (raise(signal) != 0) {
        if (errh != NULL) {
            errh(EXIT_FAILURE, "%s:%d: raise", __FILE__, __LINE__);
        }
        return -1;
    }

    return 0;
}

int
sigs_retry_int(int (*const func)(int), const int val)
{
    int retval = 0;

    do {
        errno = 0;
        retval = func(val);
    } while (retval != 0 &&
             /* cppcheck-suppress misra-c2012-22.10 */
             errno == EINTR);

    return retval;
}

int
sigs_retry_ptr(int (*const func)(void *), void *const ptr)
{
    int retval = 0;

    do {
        errno = 0;
        retval = func(ptr);
    } while (retval != 0 &&
             /* cppcheck-suppress misra-c2012-22.10 */
             errno == EINTR);

    return retval;
}
