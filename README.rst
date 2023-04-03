|coverage|
|codacy|
|security|
|reliability|
|maintainability|


=====
suCGI
=====

Run CGI scripts with the permissions of their owner.

suCGI checks whether a CGI script is owned by a regular user, sets the real
and the effective UID, the real and the effective GID, and the supplementary
groups of the current process to the UID, the GID, and the supplementary
groups of that user, cleans up the environment, and then runs the script.


Requirements
============

suCGI should work on any system that is compatible with `4.4BSD`_ and
compliant with `POSIX.1-2008`_, including the X/Open System Interface
extension; any Unix-like system that has been released since 2018 and
that does *not* target embedded systems should do.


Installation
============

**Do NOT use suCGI at this point!**
suCGI is work in progess and has *not* been reviewed, let alone audited.

----

Download the repository and unpack it.

Please take the time to read and evaluate the source code.

----

You may need to install:

* A C99 compiler (GCC_ and Clang_ are known to work)
* The header files of your system's standard library
* An assembler and a linker; these are usually packaged as "binary utilities"
  (`GNU Binutils`_ and FreeBSD's binary utilities are known to work)
* Make (`GNU Make`_, FreeBSD's Make, and bmake_ are known to work)
* M4 (`GNU M4`_ and FreeBSD's M4 are known to work)

On many systems you can do so by::

	sudo ./prepare

``./prepare`` uses the package manager of your operating system, if possible.

----

Generate the *makefile*, *compat.h*, and *config.h* by::

    ./configure

See `docs/build.rst`_ for details.

----

suCGI is configured at compile-time. Adapt *config.h* to your needs.

----

Compile suCGI by::

    make

----

Install suCGI by::

    sudo make install

``sudo make install`` will do nothing if suCGI has already been
installed and the installed binary was created or modified more
recently than the binary that has just been built.

You can uninstall suCGI by ``sudo make uninstall``.


Configuration
=============

If you are are using Apache_ and want to enable users to run their PHP_
scripts under their own user and group IDs.

Enable mod_userdir_::

	a2enmod userdir

Enable mod_action_::

	a2enmod action

Add the following lines to your Apache configuration::

    <Directory "/home">
        Action application/x-httpd-php /cgi-bin/sucgi
    </Directory>

The directory should correspond to *USER_DIR* in *config.h*.

Restart Apache::

    apache2ctl -t && apache2ctl restart

PHP scripts in */home* should now be run with the permissions of
their respective owners.


Documentation
=============

See the source code for more details.


Contact
=======

If there's something wrong with suCGI, please
`open an issue <https://github.com/odkr/sucgi/issues>`_.


License
=======

Copyright 2022 and 2023 Odin Kroeger

suCGI is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option)
any later version.

suCGI is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with suCGI. If not, see <https://www.gnu.org/licenses/>.


Further Information
===================

GitHub: https://github.com/odkr/sucgi

.. _4.4BSD: https://docs-legacy.freebsd.org/44doc/

.. _Apache: https://httpd.apache.org/

.. _bmake: https://www.crufty.net/help/sjg/bmake.html

.. _Clang: https://clang.llvm.org/

.. _`docs/build.rst`: docs/build.rst

.. _GCC: https://gcc.gnu.org/

.. _`GNU Binutils`: https://www.gnu.org/software/binutils/

.. _`GNU M4`: https://www.gnu.org/software/m4/

.. _`GNU Make`: https://www.gnu.org/software/make/

.. _mod_action: https://httpd.apache.org/docs/2.4/mod/mod_actions.html

.. _mod_userdir: https://httpd.apache.org/docs/2.4/mod/mod_userdir.html

.. _PHP: https://www.php.net/

.. _`POSIX.1-2008`: https://pubs.opengroup.org/onlinepubs/9699919799.2008edition/

.. |codacy| image:: https://app.codacy.com/project/badge/Grade/cb67a3bad615449589dfb242876600ac
            :target: https://www.codacy.com/gh/odkr/sucgi/dashboard?utm_source=github.com&amp;utm_content=odkr/sucgi

.. |coverage| image:: https://app.codacy.com/project/badge/Coverage/cb67a3bad615449589dfb242876600ac
              :target: https://www.codacy.com/gh/odkr/sucgi/dashboard?utm_source=github.com&amp;utm_content=odkr/sucgi

.. |security| image:: https://sonarcloud.io/api/project_badges/measure?project=odkr_sucgi&metric=security_rating
              :target: https://sonarcloud.io/summary/new_code?id=odkr_sucgi

.. |reliability| image:: https://sonarcloud.io/api/project_badges/measure?project=odkr_sucgi&metric=reliability_rating
                 :target: https://sonarcloud.io/summary/new_code?id=odkr_sucgi

.. |maintainability| image:: https://sonarcloud.io/api/project_badges/measure?project=odkr_sucgi&metric=sqale_rating
                    :target: https://sonarcloud.io/summary/new_code?id=odkr_sucgi
