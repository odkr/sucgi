#define _BSD_SOURCE
#define _DARWIN_C_SOURCE
#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <grp.h>
#include <inttypes.h>
#include <unistd.h>

int
main(void)
{
    GRP_T basegid = 0;
    GRP_T groups[1] = {0};
    int n = 1;

    getgrouplist("dummy", basegid, groups, &n);

    return 0;
}
