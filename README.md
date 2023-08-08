[![Coverage](https://sonarcloud.io/api/project_badges/measure?project=odkr_sucgi&metric=coverage)](https://sonarcloud.io/component_measures?metric=Coverage&id=odkr_sucgi)
[![Sonar Quality Gate](https://sonarcloud.io/api/project_badges/measure?project=odkr_sucgi&metric=alert_status)](https://sonarcloud.io/api/project_badges/measure?project=odkr_sucgi&metric=alert_status)
[![Codacy Code Quality](https://app.codacy.com/project/badge/Grade/cb67a3bad615449589dfb242876600ac)](https://www.codacy.com/gh/odkr/sucgi/dashboard?utm_content=odkr/sucgi)

# SuCGI

Run CGI scripts with the permissions of their owner.

SuCGI checks whether a script is owned by a non-system user and installed
under that user's user directory, cleans up the environment, and then runs
the script with the permissions of that user.


## System requirements

SuCGI should work on any Unix-like system released since 2010; more precisely,
it should work on any system that is compatible with [4.4BSD] and compliant
with [POSIX.1-2008], including the X/Open System Interface extension.

However, you have to compile suCGI yourself; and for that you need:

* A C99 compiler (e.g., [GCC] and [Clang])
* An assembler and a linker
  (e.g., [GNU Binutils] or FreeBSD's binary utilities)
* Make (e.g., [GNU Make], FreeBSD's Make, or [bmake])
* M4 (e.g., [GNU M4] or FreeBSD's M4)
* The header files of your system's standard library

SuCGI ships with a script that installs these for you (see below).

[4.4BSD]: https://docs-legacy.freebsd.org/44doc/

[bmake]: https://www.crufty.net/help/sjg/bmake.html

[Clang]: https://clang.llvm.org/

[GCC]: https://gcc.gnu.org/

[GNU Binutils]: https://www.gnu.org/software/binutils/

[GNU M4]: https://www.gnu.org/software/m4/

[GNU Make]: https://www.gnu.org/software/make/

[POSIX.1-2008]: https://pubs.opengroup.org/onlinepubs/9699919799.2008edition/


## Installation

**Do NOT use suCGI!**
SuCGI is work in progress, does not yet validate users,
and has *not* been reviewed, let alone audited.

Download the repository and unpack it.

Please take the time to read and evaluate the source code.

Install the tools needed to compile suCGI by:

    sudo ./prepare

*prepare* uses the package manager of your operating system, if possible.

Generate the build configuration by:

    ./configure

See **[docs/build.md]** for details and troubleshooting.

SuCGI is configured at compile-time. Adapt **config.h** to your needs.

Compile suCGI by:

    make

Install suCGI by:

    sudo make install

See **[docs/install.md]** and **[docs/uninstall.md]** for details.

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

    <Directory "/home">
        Action application/x-httpd-php /cgi-bin/sucgi
    </Directory>

The directory should correspond to USER_DIR in **config.h**.

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
You are welcome to write an email if you do not have a GitHub account.


## License

Copyright 2022 and 2023 Odin Kroeger

SuCGI is free software: you can redistribute it and/or modify it
under the terms of the GNU Affero General Public License as published
by the Free Software Foundation, either version 3 of the License,
or (at your option) any later version.

SuCGI is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General
Public License for more details.

You should have received a copy of the GNU Affero General Public
License along with SuCGI. If not, see <https://www.gnu.org/licenses/>.
