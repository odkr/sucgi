/* Headers for utils.c */

#include "../attr.h"
#include "../err.h"

/* Abort the programme with an error message. */
// This is not a call to the access() function.
// flawfinder: ignore
__attribute__((noreturn, access(read_only, 1), format(printf, 1, 2)))
void die(const char *const message, ...);

/*
 * Covert str to an unsigned long and
 * store its value in the variable pointed to by n.
 *
 * Return code:
 *      OK            Success.
 *      ERR           Trailing nun-numeric characters.
 *      ERR_SYS       System error. errno(2) should be set.
 */
// This is not a call to the access() function.
// flawfinder: ignore
__attribute__((access(read_only, 1)))
error str_to_ulong (const char *const s, unsigned long *n);

/*
 * Split s at each occurrence of a whitespace character and
 * store the substrings in the array pointed to by subs and the
 * number of substrings in the variable pointed to by n.
 *
 * Return code:
 *      OK            Success.
 *      ERR_STR_LEN   s is longer than STR_MAX_LEN.
 *      ERR           s consists of more than STR_MAX_SUBS words.
 *      ERR_SYS       System error. errno(2) should be set.
 */
// This is not a call to the access() function.
// flawfinder: ignore
__attribute__((access(read_only, 1)))
error str_words (const char *const restrict s, char ***subs);
