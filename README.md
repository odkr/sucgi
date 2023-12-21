# suCGI

Run CGI scripts with the permissions of their owner.

suCGI checks whether a CGI script is owned by a regular user, sets the real
and the effective UID, the real and the effective GID, and the supplementary
groups of the current process to the UID, the GID, and the supplementary
groups of that user, cleans up the environment, and then runs the script.

This repository has moved to <https://www.codeberg.org/odkr/sucgi>.
