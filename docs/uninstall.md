# Uninstalling suCGI

You can uninstall suCGI by:

    make uninstall

If you changed the **makefile**'s defaults by defining Make macros on the
command line when you installed suCGI, you *must* define the same macros
with the same values when you call `make uninstall`! For example, if suCGI
was installed with `make PREFIX=/usr install`, it *must* be uninstalled
with `make PREFIX=/usr uninstall`.
