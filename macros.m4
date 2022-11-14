dnl Macros for M4 templates.
changecom()dnl
define(`default', `ifdef(`$1', `ifelse($1, `', `$2', `$1')', `$2')')dnl
define(`ifnempty', `ifdef(`$1', `ifelse($1, `', `', `$2')', `')')dnl
