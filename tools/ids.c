/*
 * Print user or group IDs.
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
#include <err.h>
#include <errno.h>
#include <grp.h>
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
#define NIDS 8192


/*
 * Data types
 */

/*
 * Abstraction over struct passwd and struct group.
 * Requires a 4.4BSD-compatible memory lay-out.
 */
typedef struct {
    char *name;
    char *passwd;
    id_t id;
} Entry;

/* Signature of set(pw|gr)ent. */
typedef void SetEnt(void);

/* Signature of end(pw|gr)ent. */
typedef void EndEnt(void);

/* Signature of get(pw|gr)ent. */
typedef Entry *GetEnt(void);


/*
 * Main
 */

int
main(int argc, char **argv)
{
    Entry *ent;
    SetEnt *setent;
    EndEnt *endent;
    GetEnt *getent;
    const char *label;
    id_t *seen;
    size_t nseen;
    size_t maxseen;
    int ch;

    setent = setpwent;
    endent = endpwent;
    getent = (GetEnt *) getpwent;
    label = "pw";
    nseen = 0;
    maxseen = NIDS;

    while ((ch = getopt(argc, argv, "gh")) != -1) {
        switch (ch) {
        case 'h':
            (void) puts(
"ids - print user or group IDs and names\n\n"
"Usage:  ids [-g]\n"
"        ids -h\n\n"
"Options:\n"
"    -g  Print group IDs and names.\n"
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
        case 'g':
            setent = setgrent;
            endent = endgrent;
            getent = (GetEnt *) getgrent;
            label = "gr";
            break;
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

    seen = malloc(maxseen * sizeof(*seen));
    if (seen == NULL) {
        err(EXIT_FAILURE, "malloc");
    }

    errno = 0;
    setent();
    if (errno != 0) {
        err(EXIT_FAILURE, "set%sent", label);
    }

    while ((errno = 0, ent = getent()) != NULL) {
        id_t id = ent->id;
        char *name = ent->name;

        for (size_t j = 0; j < nseen; ++j) {
            if (seen[j] == id) {
                goto next_entry;
            }
        }

        if ((id_t) -1 < (id_t) 1) {
            (void) printf("%" PRId64 " %s\n", (int64_t) id, name);
        } else {
            (void) printf("%" PRIu64 " %s\n", (uint64_t) id, name);
        }

        if (nseen == maxseen) {
            maxseen *= 2;

            if (SIZE_MAX / sizeof(*seen) > maxseen) {
                errx(EXIT_FAILURE, "too many entries");
            }

            seen = realloc(seen, maxseen * sizeof(*seen));
            if (seen == NULL) {
                err(EXIT_FAILURE, "realloc");
            }
        }

        seen[nseen++] = id;

        next_entry:;
    }

    if (errno != 0) {
        err(EXIT_FAILURE, "get%sent", label);
    }

    errno = 0;
    endent();
    if (errno != 0) {
        err(EXIT_FAILURE, "end%sent", label);
    }

    return EXIT_SUCCESS;
}
