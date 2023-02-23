============
Coding style
============

SEI CERT C > K & R > BSD KNF, suckless and other guy where appropriate, but

: ident is always four spaces.
: brackets are only used for operator precedence if the aide comprehension

: indent multiline statements like so:

func(...,
     ...)

: indent multiline if/for like so:

for (...
     ...)
{

}

if (...
    ...)
{

}

only initialise variables if needed. initialise close to the relevant code.
(GCC v12 is good at catching unitialised variables, this is easier to catch
than initialisation with something meaningless)