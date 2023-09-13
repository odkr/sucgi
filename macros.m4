dnl Macros for M4 templates.
dnl
dnl Copyright 2022 and 2023 Odin Kroeger
dnl
dnl This file is part of suCGI.
dnl
dnl suCGI is free software: you can redistribute it and/or modify it
dnl under the terms of the GNU Affero General Public License as published
dnl by the Free Software Foundation, either version 3 of the License,
dnl or (at your option) any later version.
dnl
dnl suCGI is distributed in the hope that it will be useful, but WITHOUT
dnl ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
dnl or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General
dnl Public License for more details.
dnl
dnl You should have received a copy of the GNU Affero General Public
dnl License along with suCGI. If not, see <https://www.gnu.org/licenses/>.
changecom()dnl
define(`ifnempty', `ifdef(`$1', `ifelse(`$1', `', `ifdef(`$3', `$3', `')', `$2')', `$3')')dnl
define(`default', `ifnempty(`$1', `$1', `$2')')dnl
define(`ifcontains', `ifelse(index(`$1', `$2'), `-1', `$4', `$3')')dnl
define(`ifcflag', `ifcontains(default(`__CFLAGS'), `$1', `$2', `$3')')dnl
define(`ifhascmd', `syscmd(`command -v "$1" >/dev/null 2>&1')ifelse(sysval, `0', `$2', `$3')')dnl
