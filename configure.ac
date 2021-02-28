#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#
# Copyright (C) 2005-2021 Marius L. Jøhndal
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
AC_PREREQ(2.59)
AC_INIT(castget, 2.0.1, mariuslj@ifi.uio.no)
AM_INIT_AUTOMAKE([1.8 dist-bzip2 no-dist-gzip foreign])
AC_CONFIG_SRCDIR([src/castget.c])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIRS([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

# Checks for libraries.
PKG_CHECK_MODULES(GLIBS, [
  glib-2.0 >= 2.30
  libxml-2.0
])

AC_PATH_PROG(CURL_CONFIG, curl-config, no)
if test "$CURL_CONFIG" = "no" ; then
  AC_MSG_ERROR(Required library libcurl not found)
else
  CURL_CFLAGS=`$CURL_CONFIG --cflags`
  CURL_LIBS=`$CURL_CONFIG --libs`

  AC_SUBST(CURL_CFLAGS)
  AC_SUBST(CURL_LIBS)
fi

#AC_ARG_WITH(taglib, [  --without-taglib        disable taglib support])
AC_ARG_WITH(taglib, AC_HELP_STRING([--without-taglib], [disable taglib support])])
if test "x$with_taglib" != "xno"; then
  PKG_CHECK_MODULES(TAGLIB, [taglib_c])
  AC_SUBST(TAGLIB_CFLAGS)
  AC_SUBST(TAGLIB_LIBS)
  AC_DEFINE(HAVE_TAGLIB, [1], [Define to 1 if you have taglib support])
fi

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([strdup strtol])

AC_CONFIG_FILES([
  Makefile
  src/Makefile
  tests/Makefile
])
AC_OUTPUT
