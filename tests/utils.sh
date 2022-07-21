#!/bin/sh
# Functions for test scripts.

# Print an error message and exit
abort() {
	exec >&2
	name="$(basename "$0")" && [ "$name" ] && printf '%s: ' "$name"
	echo "$*"
	exit 8
}

# Clean up on exit.
cleanup() {
	status=$?
	set +e
	trap : EXIT HUP INT TERM
	kill -15 -$$ >/dev/null 2>&1
	[ "${CLEANUP-}" ] && eval "$CLEANUP"
	exit "$status"
}
