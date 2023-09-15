[![Coverage](https://sonarcloud.io/api/project_badges/measure?project=odkr_sucgi&metric=coverage)](https://sonarcloud.io/component_measures?metric=Coverage&id=odkr_sucgi)
[![Sonar Quality Gate](https://sonarcloud.io/api/project_badges/measure?project=odkr_sucgi&metric=alert_status)](https://sonarcloud.io/api/project_badges/measure?project=odkr_sucgi&metric=alert_status)
[![Codacy Code Quality](https://app.codacy.com/project/badge/Grade/cb67a3bad615449589dfb242876600ac)](https://www.codacy.com/gh/odkr/sucgi/dashboard?utm_content=odkr/sucgi)

# suCGI

Run CGI scripts with the permissions of their owner.

suCGI checks whether a script is owned by a non-system user and within
the root directory of that user's homepage, cleans up the environment,
and then runs the script with the permissions of that user.


## System requirements

suCGI should work on any recent-ish Unix-like system. More precisely,
it should work on any system that complies with [POSIX.1-2008], including
its X/Open System Interface extension, and is compatible with [4.4BSD],
that is, supports *getgrouplist*(), *setgroups*(), and *vsyslog*().

Compiling suCGI requires:

* A C99 compiler (e.g., [GCC], [Clang], or [TinyCC])
* An archiver, an assembler, and a linker
  (e.g., from [GNU Binutils] or a BSD distribution)
* Make (e.g., [GNU Make], a BSD Make, [bmake], or [smake])
* M4 (e.g., [GNU M4] or a BSD M4)
* The header files of your system's standard library

suCGI comes with a script that installs
these applications if needed (see below).

[4.4BSD]: https://docs-legacy.freebsd.org/44doc/

[bmake]: https://www.crufty.net/help/sjg/bmake.html

[Clang]: https://clang.llvm.org/

[GCC]: https://gcc.gnu.org/

[GNU Binutils]: https://www.gnu.org/software/binutils/

[GNU M4]: https://www.gnu.org/software/m4/

[GNU Make]: https://www.gnu.org/software/make/

[POSIX.1-2008]: https://pubs.opengroup.org/onlinepubs/9699919799.2008edition/

[smake]: https://sourceforge.net/projects/s-make/

[TinyCC]: https://bellard.org/tcc/


## Installation

> [!WARNING]
> suCGI is work in progress, fails to run many CGI applications,
> does not validate users, and has *not* been reviewed,
> let alone audited. **Do not use it!**

Download the repository and unpack it.

Please take the time to read and evaluate the source code.

Install a C build toolchain:

    sudo ./installc

*installc* uses the system's package manger.

Generate the build configuration by:

    ./configure

See **[docs/build.md]** for details and troubleshooting.

suCGI is configured at compile-time.
Adapt **[config.h](config.h)** to your needs.

    vi -c'read config.h.tpl' config.h

Compile suCGI by:

    make

Install suCGI by:

    sudo make install

See **[docs/install.md]** and **[docs/uninstall.md]** for details.

Optionally, remove the packages installed via *installc* by:

    sudo ./installc -r


[docs/build.md]: docs/build.md

[docs/install.md]: docs/install.md

[docs/uninstall.md]: docs/uninstall.md


## Setup

If you are are using [Apache] and want to enable users to run their [PHP]
scripts under their own user and group IDs, you can do so by following
these steps.

Enable [mod_userdir]:

    a2enmod userdir

Enable [mod_action]:

    a2enmod action

Add the following lines to your Apache configuration::

    <Directory "/home/*/public_html">
        AddHandler php-scripts .php
        Action php-scripts /cgi-bin/sucgi
    </Directory>

The directory should correspond to USER_DIR in **config.h**.

Do *not* enable mod_php.

Restart Apache:

    apache2ctl -t && apache2ctl restart

[Apache]: https://httpd.apache.org/

[mod_action]: https://httpd.apache.org/docs/2.4/mod/mod_actions.html

[mod_userdir]: https://httpd.apache.org/docs/2.4/mod/mod_userdir.html

[PHP]: https://www.php.net/


## Documentation

See the **docs** sub-directory and the source code.


## Contact

If there's something wrong with suCGI, please
[open an issue](https://github.com/odkr/sucgi/issues).

You are welcome to write an email if you don't have a GitHub account.


## License

Copyright 2022 and 2023 Odin Kroeger

suCGI is free software: you can redistribute it and/or modify it
under the terms of the GNU Affero General Public License as published
by the Free Software Foundation, either version 3 of the License,
or (at your option) any later version.

suCGI is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General
Public License for more details.

You should have received a copy of the GNU Affero General Public
License along with suCGI. If not, see <https://www.gnu.org/licenses/>.
