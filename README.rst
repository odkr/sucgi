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
groups of the current process to the UID, the GID, and the groups of that
user, cleans the environment, and then runs the script.


Requirements
============

suCGI requires a system that is compatible with `4.4BSD`_ and compliant with
`POSIX.1-2008`_, including its X/Open system interface extension.


Installation
============

**Do NOT use suCGI at this point!**
suCGI is work in progess and has *not* been reviewed, let alone audited.

----

You may need to install the standard build tools and M4.

Arch-based GNU/Linux systems::

    pacman -S base-devel

Debian-based GNU/Linux systems::

    apt-get install build-essential m4

RedHat-based GNU/Linux systems::

    dnf group install "C Development Tools and Libraries" "Development Tools"
    dnf install m4

macOS::

    xcode-select --install

----

Download the repository and unpack it.

Please take the time to read and evaluate the source code.

----

Generate the *makefile* by::

    ./configure

If ``configure`` succeeded, move on to the next step.

See `docs/build.rst`_ for details.

----

suCGI is configured at compile-time. Adapt *config.h* to your needs.

    cp config.h.sample config.h
    vi config.h

----

Comipile suCGI by::

    make

----

Install suCGI by::

    sudo make install

``sudo make install`` will do nothing if suCGI is already installed and
the file modification time of the installed binary is more recent than
that of the binary that has just been built.

You can uninstall suCGI by ``sudo make uninstall``.


Apache Configuration for PHP
============================

Set up Apache_ and PHP_, enable mod_userdir_ and mod_action_, install suCGI
and then add the following lines to your Apache configuration::

    <Directory "/home">
        Action application/x-httpd-php /cgi-bin/sucgi
    </Directory>

The directory should correspond to *JAIL_DIR* in *config.h*.

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

.. _4.4BSD: https://docs-legacy.freebsd.org/44doc/

.. _Apache: https://httpd.apache.org/

.. _`docs/build.rst`: docs/build.rst

.. _Clang: https://clang.llvm.org/

.. _GCC: https://gcc.gnu.org/

.. _glibc: https://www.gnu.org/software/libc/

.. _mod_action: https://httpd.apache.org/docs/2.4/mod/mod_actions.html

.. _mod_userdir: https://httpd.apache.org/docs/2.4/mod/mod_userdir.html

.. _PHP: https://www.php.net/

.. _`POSIX.1-2008`: https://pubs.opengroup.org/onlinepubs/9699919799.2008edition/

.. _realpath: https://cve.mitre.org/cgi-bin/cvekey.cgi?keyword=realpath

.. _XNU: https://github.com/apple-oss-distributions/xnu/

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
