/* Headers for utils.c */

/* Abort the programme with an error message. */
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
enum code str_to_ulong (const char *const s, unsigned long *n);

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
enum code str_words (const char *const restrict s, char ***subs);
