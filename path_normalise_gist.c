
/* FIXME: Untested. */
enum error
path_normalise(const char *const path,
               char (*const norm)[STR_MAX], size_t *const len)
{
	size_t pos;		/* Current position in the path. */

	assert(*path != '\0');

	pos = strncmp(path, "./", 2) == 0 ? strspn(&path[2], "/") + 2 : 0;
	*len = 0;

	while (path[pos] != '\0') {
		const char *cur = &path[pos];	/* Current segment. */

		       if (strncmp(cur, "/./", 3) == 0) {
			pos += 2;
		} else if (strncmp(cur, "//", 2) == 0) {
			pos += 1;
		} else {
			size_t n;	/* Length of current segment. */
			
			n = strcspn(cur + 1, "/") + 1;
			if (*len + n >= STR_MAX) return ERR_STR_LEN;
			(void) memcpy(*norm + *len, cur, n);

			(*len) += n;
			pos += n;
		}
	}

	if (*len > 1 && strncmp(*norm + *len - 2, "/.", 2) == 0) (*len)--;
	if (*len > 1 && strncmp(*norm + *len - 1, "/", 1) == 0) (*len)--;
	(void) memset(*norm + *len, 0, STR_MAX - *len);

	return OK;
}
