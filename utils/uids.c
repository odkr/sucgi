/*
 * Print user IDs.
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

#define _BSD_SOURCE
#define _DARWIN_C_SOURCE
#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <sys/types.h>
#include <assert.h>
#include <err.h>
#include <errno.h>
#include <pwd.h>
#include <inttypes.h>
#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


/*
 * Constants
 */

/* Initial maximum number of seen IDs. */
#define NIDS 2048


/*
 * Data types
 */

/* lfind comparison function. */
typedef int (*Compar)(const void *, const void *);


/*
 * Prototypes
 */

/* Compare two user IDs. */
static int cmpuids(const uid_t *uid1, const uid_t *uid2);


/*
 * Functions
 */

static int
cmpuids(const uid_t *const uid1, const uid_t *const uid2)
{
    assert(uid1 != NULL);
    assert(uid2 != NULL);

    if (*uid1 < *uid2) {
        return -1;
    };

    if (*uid1 > *uid2) {
        return 1;
    }

    return 0;
}


/*
 * Main
 */

int
main(int argc, char **argv)
{
    struct passwd *pwd;
    uid_t *seen;
    size_t nseen;
    size_t maxnseen;
    int opt;

    nseen = 0;
    maxnseen = NIDS;

    while ((opt = getopt(argc, argv, "gh")) != -1) {
        switch (opt) {
        case 'h':
            (void) puts(
"uids - print user IDs and names\n\n"
"Usage:  uids\n"
"        uids -h\n\n"
"Options:\n"
"    -h  Print this help screen.\n\n"
"Exit statuses:\n"
"     0  Success.\n"
"    64  Usage error.\n"
"    69  Too many entries.\n"
"    71  System error.\n\n"
"Copyright 2022 and 2023 Odin Kroeger.\n"
"Released under the GNU General Public License.\n"
"This programme comes with ABSOLUTELY NO WARRANTY."
            );
            return EXIT_SUCCESS;
        default:
            return EXIT_FAILURE;
        }
    }

    argc -= optind;
    argv += optind;

    if (argc > 0) {
        (void) fputs("usage: ids\n", stderr);
        return EXIT_FAILURE;
    }

    seen = malloc(maxnseen * sizeof(*seen));
    if (seen == NULL) {
        err(EXIT_FAILURE, "malloc");
    }

    errno = 0;
    setpwent();
    if (errno != 0) {
        err(EXIT_FAILURE, "setpwsent");
    }

    while ((errno = 0, pwd = getpwent()) != NULL) {
        uid_t uid = pwd->pw_uid;
        uid_t *hit;

        hit = lfind(&uid, seen, &nseen, sizeof(*seen), (Compar) cmpuids);
        if (hit == NULL) {
            char *name = pwd->pw_name;

            if ((uid_t) -1 < (uid_t) 1) {
                (void) printf("%lld %s\n", (long long) uid, name);
            } else {
                (void) printf("%llu %s\n", (unsigned long long) uid, name);
            }

            if (nseen == maxnseen) {
                maxnseen *= 2;

                if (SIZE_MAX / sizeof(*seen) > maxnseen) {
                    errx(EXIT_FAILURE, "too many entries");
                }

                seen = realloc(seen, maxnseen * sizeof(*seen));
                if (seen == NULL) {
                    err(EXIT_FAILURE, "realloc");
                }
            }

            seen[nseen++] = uid;
        }
    }

    if (errno != 0) {
        err(EXIT_FAILURE, "getpwsent");
    }

    errno = 0;
    endpwent();
    if (errno != 0) {
        err(EXIT_FAILURE, "endpwent");
    }

    return EXIT_SUCCESS;
}
