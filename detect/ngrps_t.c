#define _BSD_SOURCE
#define _DARWIN_C_SOURCE
#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <sys/types.h>
#include <grp.h>
#include <unistd.h>

int
main(void)
{
    NGRPS_T ngroups = 0;
    gid_t gidset[1] = {0};

    (void) setgroups(ngroups, gidset);

    return 0;
}
