|coverage|
|codacy|
|security|
|reliability|
|maintainability|


=====
suCGI
=====

Run CGI programmes under the UID and GID of their owner.

suCGI checks if the programme pointed to by the environment variable
*PATH_TRANSLATED* is secure, sets the process' effective UID and GID 
to the UID and the GID of that programme's owner, cleans up the
environment, and then runs the programme.


Requirements
============

A GNU/Linux system running a kernel ≥ v5.6 or macOS ≥ v11.

More precisely:

* Linux ≥ v5.6 or Apple XNU ≥ v7195.50.7.100.1.
* A C99 compiler that complies with `POSIX.1-2008`_;
  e.g., GCC_ ≥ v5.1, Clang_ ≥ v3.5, or TinyCC_ ≥ v0.9.
* A C standard library that complies with POSIX.1-2008 and 4.2BSD;
  e.g., glibc_ ≥ v2.1.3 or Apple's Libc.
* The standard utilities that POSIX.1-2008, including
  its X/Open system interface extension, mandates.

Save for Linux ≥ v5.6 and XNU ≥ v7195.50.7.100.1 respectively, any
post-2015 GNU/Linux or macOS system should meet those requirements.
You may need to install the standard build tools, however.

Arch-based GNU/Linux systems::

    pacman -S base-devel

Debian-based GNU/Linux systems::

    apt-get install build-essential m4

RedHat-based GNU/Linux systems::

    dnf group install "C Development Tools and Libraries" "Development Tools"
    dnf install m4

macOS::

    xcode-select --install


Installation 
============

**Do NOT use suCGI at this point!**

**You use suCGI at your own risk!**

----

Download the repository and unpack it.

Please take the time to read and evaluate the source code.

----

Generate *config.h* and *makefile* by::

    ./configure

If ``configure`` succeeded, move on to the next step.

Otherwise, generate *config.h* by ``m4 config.h.in >config.h`` and
*makefile* by ``m4 makefile.in >makefile``. Alternatively, configure
the build yourself (see `BUILDING.rst`_).

----

Adapt *config.h* to your needs.
Most importantly, adapt *DOC_ROOT_PAT*, *MIN_UID* and *MAX_UID*.
suCGI is configured at compile-time, you cannot do this later.

----

Compile and install suCGI by::

    make install

You can uninstall suCGI by ``make uninstall``.


Apache Configuration for PHP
============================

Set up Apache_ and PHP_, enable mod_userdir_ and mod_action_, install suCGI
and then add the following lines to your Apache configuration::

    <Directory "/home">
        Action application/x-httpd-php /cgi-bin/sucgi
    </Directory>

The directory should correspond to *DOC_ROOT_PAT* in *config.h*.

Restart Apache::

    apache2ctl -t && apache2ctl restart

PHP scripts in */home* should now be run under the UID and GID of their owner.


Documentation
=============

See the source code for more details.


Contact
=======

If there's something wrong with suCGI, please
`open an issue <https://github.com/odkr/sucgi/issues>`_.


License
=======

Copyright 2022 Odin Kroeger

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


.. _Clang: https://clang.llvm.org/

.. _GCC: https://gcc.gnu.org/

.. _glibc: https://www.gnu.org/software/libc/

.. _TinyCC: http://www.tinycc.org/ 

.. _XNU: https://github.com/apple-oss-distributions/xnu/

.. _`POSIX.1-2008`: https://pubs.opengroup.org/onlinepubs/9699919799.2008edition/

.. _`BUILDING.rst`: BUILDING.rst

.. _Apache: https://httpd.apache.org/

.. _mod_action: https://httpd.apache.org/docs/2.4/mod/mod_actions.html

.. _mod_userdir: https://httpd.apache.org/docs/2.4/mod/mod_userdir.html

.. _PHP: https://www.php.net/

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
