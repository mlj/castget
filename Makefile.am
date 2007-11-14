#
# Copyright (C) 2005, 2006, 2007 Marius L. Jøhndal
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
#   $Id: Makefile.am,v 1.7 2007/11/14 14:21:07 mariuslj Exp $
#
SUBDIRS = src

EXTRA_DIST = \
  castgetrc.example \
  castget.1.in \
  castgetrc.5.in

man_MANS = \
  castget.1 \
  castgetrc.5

preview-man: castget.1
	groff -Tutf8 -man castget.1 | less
