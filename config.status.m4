changequote([, ])dnl
#!/bin/sh

# This script stores the most recent configuration detected by ./configure.
# Running this script will regenerate the files based on that configuration.

ifdef([__CC__], [ifelse(__CC__, [], [], [CC="__CC__"
])], [])dnl
ifdef([__CFLAGS__], [ifelse(__CFLAGS__, [], [], [CFLAGS="__CFLAGS__"
])], [])dnl
ifdef([__COV_CC__], [ifelse(__COV_CC__, [], [], [COV_CC="__COV_CC__"
])], [])dnl

export CC CFLAGS COV_CC
./configure -Cf __ARGS__
