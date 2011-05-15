#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#
# Copyright (C) 2005, 2006, 2007, 2011 Marius L. Jøhndal
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
AC_INIT(castget, 1.1.0, mariuslj@ifi.uio.no)
AM_INIT_AUTOMAKE([1.8 dist-bzip2 no-dist-gzip])
AC_CONFIG_SRCDIR([src/castget.c])
AC_CONFIG_HEADER([config.h])

AH_TEMPLATE([ENABLE_ID3LIB], [Define to 1 if you want to enable support for id3lib])
AC_ARG_ENABLE(id3lib,
              [AC_HELP_STRING([--disable-id3lib],
                              [disable support for id3lib [default=no]])],
              configure_enable_id3lib=$enableval,
              configure_enable_id3lib=yes)

AH_TEMPLATE([ENABLE_GREGEX], [Define to 1 if you want to enable support for GRegex])
AC_ARG_ENABLE(gregex,
              [AC_HELP_STRING([--disable-gregex],
                              [disable support for GRegex filters [default=no]])],
              configure_enable_gregex=$enableval,
              configure_enable_gregex=yes)

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

# Checks for libraries.
if test "x$configure_enable_gregex" = "xyes"; then
  GLIB_REQUIRED_VERSION=2.14
  AC_DEFINE(ENABLE_GREGEX, [1], [Define for GRegex support])
else
  GLIB_REQUIRED_VERSION=2.0
fi
dnl AC_SUBST(GLIB_REQUIRED_VERSION)

PKG_CHECK_MODULES(GLIBS, [
  glib-2.0 >= $GLIB_REQUIRED_VERSION
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

if test "x$configure_enable_id3lib" = "xyes"; then
  AC_SEARCH_LIBS(ID3Tag_Link,
    "id3" "id3 -lstdc++" "id3 -lz" "id3 -lz -lstdc++",
    ,
    AC_MSG_ERROR(Required library id3lib not found))

  dnl Needed to link correctly with id3lib 3.8.0
  LIBS="-lz -lstdc++ $LIBS"

  AC_DEFINE(ENABLE_ID3LIB, [1], [Define for id3lib support])
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
  castget.1
  castgetrc.5
])
AC_OUTPUT
