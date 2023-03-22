dnl Macros for M4 templates.
dnl
dnl Copyright 2022 and 2023 Odin Kroeger
dnl
dnl This file is part of suCGI.
dnl
dnl suCGI is free software: you can redistribute it and/or modify it under
dnl the terms of the GNU General Public License as published by the Free
dnl Software Foundation, either version 3 of the License, or (at your option)
dnl any later version.
dnl
dnl suCGI is distributed in the hope that it will be useful, but WITHOUT
dnl ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
dnl FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
dnl for more details.
dnl
dnl You should have received a copy of the GNU General Public License along
dnl with suCGI. If not, see <https://www.gnu.org/licenses>.
changecom()dnl
define(`default', `ifdef(`$1', `ifelse(`$1', `', `$2', `$1')', `$2')')dnl
define(`findcmd', `syscmd(`printf %s $(command -v "$1" 2>/dev/null)')')dnl
define(`hascmd', `syscmd(`command -v "$1" >/dev/null 2>&1')sysval')dnl
define(`hasfile', `syscmd(`test -e "$1" >/dev/null 2>&1')sysval')dnl
define(`ifnempty', `ifdef(`$1', `ifelse(`$1', `', `$3', `$2')', `$3')')dnl
define(`ifhascmd', `ifelse(hascmd(`$1'), `0', `$2', `$3')')dnl
define(`ifhasfile', `ifelse(hasfile(`$1'), `0', `$2', `$3')')dnl
