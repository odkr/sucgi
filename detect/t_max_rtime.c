#include <sys/types.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>

#define START (CHAR_MAX + 1)

int
main(void)
{
    uintmax_t i;

    if ((T) -1 < (T) 0) {
        for (i = START; i > 0 && (T) (i - 1) == (i - 1); i *= 2) {
            printf("%lld\n", (long long) i - 1);
        }
    } else {
        for (i = START; i > 0 && (T) (i - 1) == (i - 1); i *= 2) {
            printf("%llu\n", (unsigned long long) i - 1);
        }
    }

    return 0;
}
