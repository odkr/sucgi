/*
 * Run jobs in parallel.
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

#define _BSD_SOURCE
#define _DARWIN_C_SOURCE
#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/wait.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <spawn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <wordexp.h>


/*
 * Constants
 */

/* Limit for the maximum number of jobs to run in parallel. */
#define MAX_NRUN 64

/* Programme name. */
#define PROGNAME "runpara"

/* Control sequence for clearing the current line. */
#define CLEARLN "\033" "[0K"

/* Control sequence for moving to the beginning of the previous line. */
#define REWINDLN "\r" "\033" "[1A"


/*
 * Macros
 */

/* Calculate the number of elements in ARRAY. */
#define NELEMS(array) (sizeof(array) / sizeof(*(array)))


/*
 * Data types
 */

/* A record for recently started jobs. */
typedef struct {
    pid_t pid;                      /* Process ID. */
    char *comm;                     /* Command line. */
    int status;                     /* Exit status. */
    volatile sig_atomic_t exited;   /* Whether the job has exited. */
} Job;

/* A trap. */
typedef struct {
    int signo;                      /* Signal to catch. */
    int flags;                      /* sigaction flags. */
    void (* handler)(int);          /* Signal handler. */
} Trap;


/*
 * Prototypes
 */

/*
 * Reset the alarm, send a SIGTERM to all running jobs, wait GRACE seconds
 * for them to terminate, and then kill any jobs that are still running.
 *
 * Side-effects:
 *     May print warnings to stderr.
 *
 * Caveats:
 *     Not async-safe.
 */
static void cleanup(void);

/*
 * Collect all finished jobs and store that they have exited and the
 * status that they have exited with in JOBS; if waitpid raises an error
 * other than ECHILD, store that error in WAITPIDERR, if a process ID
 * cannot be found, store that process ID in UNKNOWNPID.
 */
static void collect(int);

/*
 * Store SIGNO in CAUGHT.
 */
static void catch(int signo);

/*
 * Send SIGNO to all running jobs and return the number of jobs signalled.
 *
 * Side-effects:
 *     May print warnings to stderr.
 *
 * Caveats:
 *     Not async-safe.
 */
static int signaljobs(int signo);

/*
 * Clear the current line in TTY.
 *
 * Caveats:
 *     Not async-safe.
 */
static void clearln(FILE *tty);

/*
 * Rewind to the beginning of the previous line in TTY.
 *
 * Caveats:
 *     Not async-safe.
 */
static void rewindln(FILE *tty);

/*
 * Convert the current option argument into a number. If the argument is not
 * a number, smaller than MIN, or greater than MAX, raise an error.
 *
 * Caveats:
 *     Not async-safe.
 */
static long opttonum(long min, long max);


/*
 * Global variables
 */

extern char **environ;


/*
 * Module variables
 */

/* Most recently started jobs. */
static Job jobs[MAX_NRUN] = {0};

/* Most recently caught signal. */
static volatile sig_atomic_t caught = 0;

/* Number of jobs currently running. */
static volatile sig_atomic_t nrun = 0;

/* Most recent unexpected waitpid error. */
static volatile sig_atomic_t waitpiderr = 0;

/* Most recent unknown process ID. */
static volatile sig_atomic_t unknownpid = 0;

/* Maximum number of jobs to run in parallel. */
static long maxnrun = 4;

/* Signal set that contains only SIGCHLD. */
static sigset_t chldsig;

/* Number of seconds to wait for jobs to terminate on abnormal exit. */
static long grace = 5;


/*
 * Functions
 */

static void
cleanup(void)
{
    (void) alarm(0);

    if (nrun > 0) {
        sigset_t oldmask;
        time_t now;

        errno = 0;
        if (sigprocmask(SIG_UNBLOCK, &chldsig, &oldmask) != 0) {
            warn("sigprocmask");
        }

        (void) signaljobs(SIGTERM);

        now = time(NULL);
        for (long rem = grace; nrun > 0 && rem > 0;
             rem = grace - (long) difftime(time(NULL), now))
        {
            clearln(stderr);
            warnx("waiting %lds for %ld job(s) to terminate ...",
                  rem, (long) nrun);
            rewindln(stderr);
            (void) sleep(1);
        }

        clearln(stderr);
        if (nrun > 0) {
            warnx("killing %ld remaining job(s) ...", (long) nrun);
        } else {
            warnx("all jobs terminated");
        }

        errno = 0;
        if (sigprocmask(SIG_BLOCK, &oldmask, NULL) != 0) {
            warn("sigprocmask");
        }
    }
}

static void
collect(const int signo __attribute__((unused)))
{
    int olderr;

    olderr = errno;

    do {
        pid_t pid;
        int status;
        int i;

        do {
            errno = 0;
            pid = waitpid(-1, &status, WNOHANG);
        } while (pid < 0 && errno == EINTR);

        if (pid <= 0) {
            if (pid < 0 && errno != ECHILD) {
                waitpiderr = errno;
            }
            break;
        }

        for (i = 0; i < maxnrun; ++i) {
            Job *job = &jobs[i];

            if (job->exited == 0 && pid == job->pid) {
                job->status = status;
                job->exited = 1;
                --nrun;
                break;
            }
        }

        if (i == maxnrun) {
            /* Should be unreachable. */
            unknownpid = pid;
            break;
        }
    } while (true);

    errno = olderr;
}

static void
catch(const int signo) {
    caught = signo;
}

static int
signaljobs(const int signo)
{
    int njobs;

    njobs = 0;
    for (int i = 0; i < maxnrun; ++i) {
        Job *job = &jobs[i];

        if (job->pid > 0 && job->exited == 0) {
            errno = 0;

            if (kill(job->pid, signo) == 0) {
                ++njobs;
            } else {
                warn("kill %lld", (long long) job->pid);
            }
        }
    }

    return njobs;
}

static void
clearln(FILE *const tty)
{
    if (isatty(fileno(tty))) {
        (void) fputs(CLEARLN, tty);
    }
}

static void
rewindln(FILE *const tty)
{
    if (isatty(fileno(tty))) {
        (void) fputs(REWINDLN, tty);
    }
}

static long
opttonum(const long min, const long max)
{
    long num;

    errno = 0;
    num = strtol(optarg, NULL, 10);

    if (num == 0 && errno != 0) {
        err(EXIT_FAILURE, "-%c", optopt);
    }
    if (num < min) {
        errx(EXIT_FAILURE, "-%c: %ld is less than %ld", optopt, num, min);
    }
    if (num > max) {
        errx(EXIT_FAILURE, "-%c: %ld is greater than %ld", optopt, num, max);
    }

    return num;
}


/*
 * Main
 */

int
main (int argc, char **argv)
{
    /* How to handle which signals. */
    const Trap traps[] = {
        {.signo = SIGCHLD, .handler = collect, .flags = SA_RESTART},
        {.signo = SIGHUP, .handler = catch},
        {.signo = SIGINT, .handler = catch},
        {.signo = SIGALRM, .handler = catch},
        {.signo = SIGTERM, .handler = catch}
    };

    /* Default signal handler. */
    const struct sigaction defhdl = {
        .sa_handler = SIG_DFL
    };

    posix_spawnattr_t procattrs;            /* Process attributes. */
    posix_spawn_file_actions_t fileacts;    /* File descriptor actions. */
    wordexp_t comms[MAX_NRUN];              /* Parsed commands. */
    bool statusmask[128];                   /* Ignored exit statuses. */
    sigset_t nosigs;                        /* Empty signal set. */
    sigset_t oldmask;                       /* Old signal mask. */
    sigset_t trapped;                       /* Set of trapped signals. */
    long timeout;                           /* Number of seconds to wait. */
    int nfork;                              /* Number of jobs forked. */
    int nrep;                               /* Number of jobs reported on. */
    int exstatus;                           /* runpara exit status. */
    int ch;                                 /* Option character. */
    bool cont;                              /* Continue despite errors? */
    bool quiet;                             /* Be quiet? */
    bool suppress;                          /* Suppress job output? */

    errno = posix_spawnattr_init(&procattrs);
    if (errno != 0) {
        err(EXIT_FAILURE, "posix_spawnattr_init");
    }

    errno = posix_spawn_file_actions_init(&fileacts);
    if (errno != 0) {
        err(EXIT_FAILURE, "posix_spawn_file_actions_init");
    }

    (void) memset(jobs, 0, sizeof(jobs));
    (void) memset(statusmask, 0, sizeof(statusmask));
    (void) memset(comms, 0, sizeof(comms));
    (void) sigemptyset(&nosigs);
    (void) sigemptyset(&chldsig);
    (void) sigemptyset(&trapped);

/* GCC mistakes chldsig for an int, and hence warns about a sign change. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
    (void) sigaddset(&chldsig, SIGCHLD);
#pragma GCC diagnostic pop

    timeout = 3600;
    nfork = 0;
    nrep = 0;

    exstatus = EXIT_SUCCESS;
    cont = false;
    quiet = false;
    suppress = true;

    while ((ch = getopt(argc, argv, "cg:j:i:qt:Sh")) != -1) {
        switch (ch) {
        case 'h':
            (void) printf(
"runpara - run jobs in parallel\n\n"
"Usage:      runpara [-c] [-gN] [-iRET] [-jN] [-tN] [-q] [-S]\n"
"                    [VAR ...] COMM [...]\n"
"            runpara -h\n\n"
"Operands:\n"
"    VAR     Variable to add to the environment.\n"
"    COMM    Command to run. Subject to shell word expansion.\n\n"
"Options:\n"
"    -c      Continue even if a job reports an error.\n"
"    -g N    Give jobs N secs to clean up on abnormal exit (default: %ld).\n"
"    -i RET  Ignore non-zero exit status RET. Can be given multiple times.\n"
"    -j N    Run N jobs in parallel (default: %ld).\n"
"    -t N    Abort after N secs (default: %ld).\n"
"    -q      Be quiet. Implied if only one job is given.\n"
"    -S      Do not suppress job output. Implied if only one job is given.\n"
"    -h      Print this help screen.\n\n"
"Copyright 2023 Odin Kroeger.\n"
"Released under the GNU General Public License.\n"
"This programme comes with ABSOLUTELY NO WARRANTY.\n",
                grace, maxnrun, timeout);
            return EXIT_SUCCESS;
        case 'c':
            cont = true;
            break;
        case 'g':
            grace = opttonum(0, SHRT_MAX);
            break;
        case 'j':
            maxnrun = opttonum(1, NELEMS(jobs));
            break;
        case 'i':
            statusmask[opttonum(0, NELEMS(statusmask) - 1U)] = true;
            break;
        case 'q':
            quiet = true;
            break;
        case 't':
            timeout = opttonum(0, 100000000);
            break;
        case 'S':
            suppress = false;
            break;
        default:
            return EXIT_FAILURE;
        }
    }

    argc -= optind;
    argv += optind;

    while (*argv != NULL && strchr(*argv, '=') != NULL) {
        putenv(*argv);
        argc--;
        argv++;
    }

    if (argc < 1) {
        (void) fputs("usage: runpara <opts> [VAR ...] COMM [...]\n", stderr);
        return EXIT_FAILURE;
    }

    if (argc == 1) {
        quiet = true;
        suppress = false;
    }

    /*
     * This must be done before jobs are spawned, because wordexp may
     * shell out and ergo trigger a SIGCHLD, breaking job collection.
     */
    for (int i = 0; i < argc; ++i) {
        int retval;

        retval = wordexp(argv[i], &comms[i], WRDE_SHOWERR);
        switch (retval) {
            case 0:
                break;
            case WRDE_NOSPACE:
                errx(EXIT_FAILURE, "memory allocation error");
            case WRDE_SYNTAX:
                errx(EXIT_FAILURE, "%s: syntax error", argv[i]);
            case WRDE_BADCHAR:
                errx(EXIT_FAILURE, "%s: forbidden character", argv[i]);
            default:
                errx(EXIT_FAILURE, "wordexp returned %d", retval);
        }
    }

    errno = 0;
    if (sigprocmask(SIG_BLOCK, &chldsig, &oldmask) != 0) {
        err(EXIT_FAILURE, "sigprocmask");
    }

    for (size_t i = 0; i < NELEMS(traps); ++i) {
        const Trap trap = traps[i];
        const struct sigaction action = {
            .sa_handler = trap.handler,
            .sa_flags = trap.flags,
        };

        errno = 0;
        if (sigaction(trap.signo, &action, NULL) != 0) {
            err(EXIT_FAILURE, "sigaction");
        }

/* GCC mistakes trapped for an int, and hence warns about a sign change. */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
        sigaddset(&trapped, trap.signo);
#pragma GCC diagnostic pop
    }

    errno = posix_spawnattr_setflags(
        &procattrs, POSIX_SPAWN_SETSIGMASK | POSIX_SPAWN_SETSIGDEF
    );
    if (errno != 0) {
        err(EXIT_FAILURE, "posix_spawnattr_setflags");
    }

    errno = posix_spawnattr_setsigmask(&procattrs, &oldmask);
    if (errno != 0) {
        err(EXIT_FAILURE, "posix_spawnattr_setsigmask");
    }

    errno = posix_spawnattr_setsigdefault(&procattrs, &trapped);
    if (errno != 0) {
        err(EXIT_FAILURE, "posix_spawnattr_setsigdefault");
    }

    if (suppress) {
        int nullfd;

        errno = 0;
        nullfd = open("/dev/null", O_RDWR | O_NONBLOCK);
        if (nullfd < 0) {
            err(EXIT_FAILURE, "open /dev/null");
        }

        for (int fd = 0; fd < 3; ++fd) {
            errno = posix_spawn_file_actions_adddup2(&fileacts, nullfd, fd);
            if (errno != 0) {
                err(EXIT_FAILURE, "posix_spawn_file_actions_adddup2");
            }
        }
    }

    errno = 0;
    if (atexit(cleanup) != 0) {
        err(EXIT_FAILURE, "atexit");
    }

    (void) alarm((unsigned int) timeout);

    do {
        for (long i = 0; i < maxnrun; ++i) {
            Job *job = &jobs[i];

            /* Report exit status of finished jobs. */
            if (job->exited == 1) {
                if (WIFSIGNALED(job->status)) {
                    int signo;

                    signo = WTERMSIG(job->status);
                    warnx("%s[%d]: %s", job->comm, job->pid, strsignal(signo));
                    return EXIT_FAILURE;
                } else if (WIFEXITED(job->status)) {
                    int status;

                    status = WEXITSTATUS(job->status);
                    if (status > 0) {
                        warnx("%s[%d] exited with status %d",
                              job->comm, job->pid, status);

                        if ((size_t) status > NELEMS(statusmask) ||
                            !statusmask[status])
                        {
                            if (!cont) {
                                return status;
                            }

                            exstatus = status;
                        }
                    }
                } else {
                    /* Should be unreachable. */
                    warnx("%s[%d] exited abnormally", job->comm, job->pid);
                    return EXIT_FAILURE;
                }

                job->pid = 0;
                job->exited = 0;
                ++nrep;
            }

            /* Start a job. */
            if (job->pid == 0 && nfork < argc) {
                char **comm = comms[nfork].we_wordv;

                job->comm = argv[nfork];

                errno = posix_spawnp(&job->pid, *comm, &fileacts,
                                     &procattrs, comm, environ);
                if (errno == 0) {
                    ++nfork;
                    ++nrun;

                    if (!quiet) {
                        warnx("[%d] %s", job->pid, job->comm);
                    }
                } else {
                    err(EXIT_FAILURE, "posix_spawnp %s", job->comm);
                }
            }
        }

        if (nrun > 0) {
            if (nfork == argc && !quiet) {
                clearln(stderr);
                warnx("waiting for %ld job(s) to complete ...", (long) nrun);
                rewindln(stderr);
            }

            (void) sigsuspend(&nosigs);
        }
    } while (argc > nrep && caught == 0 && waitpiderr == 0 && unknownpid == 0);

    (void) alarm(0);

    clearln(stderr);

    if (waitpiderr > 0) {
        /* Should be unreachable. */
        errx(EXIT_FAILURE, "waitpid: %s", strerror((int) waitpiderr));
    }

    if (unknownpid > 0) {
        /* Should be unreachable. */
        errx(EXIT_FAILURE, "forgot about process %ld", (long) unknownpid);
    }

    if (caught > 0) {
        warnx("%s", strsignal((int) caught));

        cleanup();

        (void) sigaction((int) caught, &defhdl, NULL);
        (void) raise((int) caught);

        /* Should be unreachable. */
        return (int) caught + 128;
    }

    if (!quiet) {
        warnx("all jobs completed");
    }

    return exstatus;
}
