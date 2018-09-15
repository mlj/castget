#
# Copyright (C) 2005-2018 Marius L. Jøhndal
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
SUBDIRS = tests src

EXTRA_DIST = \
  castgetrc.example \
  castget.1 \
  castget.1.ronn \
  castgetrc.5 \
  castgetrc.5.ronn \
  CHANGES.md \
  ChangeLog.old

man_MANS = \
  castget.1 \
  castgetrc.5

AUTOMAKE_OPTIONS = foreign

.PHONY: html

html: castget.1.html castgetrc.5.html

%.html: %.ronn
	ronn --manual="User Commands" --organization="castget @VERSION@" --html $< > $@

%: %.ronn
	ronn --manual="User Commands" --organization="castget @VERSION@" --roff $< > $@
