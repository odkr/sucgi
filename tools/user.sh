#!/bin/sh
# Print the current UID and GID for main.sh to verify.

set -eu

uid="$(id -u)" && [ "$uid" ] || exit 8
gid="$(id -g)" && [ "$gid" ] || exit 8

echo "$uid:$gid"
