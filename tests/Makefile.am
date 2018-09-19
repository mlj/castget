#
# Copyright (C) 2017 Wilhelm Schuster
# Copyright (C) 2017-2018 Marius L. Jøhndal
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
INCLUDES = $(GLIBS_CFLAGS) $(CURL_CFLAGS)

AUTOMAKE_OPTIONS = subdir-objects

TESTS = \
  test_patterns \
  test_progress \
  test_filenames

check_PROGRAMS = \
  test_patterns \
  test_progress \
  test_filenames

test_progress_SOURCES = test_progress.c ../src/progress.c ../src/progress.h

test_progress_LDADD = $(GLIBS_LIBS) $(CURL_LIBS)

test_patterns_SOURCES = test_patterns.c ../src/date_parsing.c ../src/date_parsing.h ../src/patterns.c ../src/patterns.h mocks.c mocks.h

test_patterns_LDADD = $(GLIBS_LIBS) $(CURL_LIBS)

test_filenames_SOURCES = test_filenames.c ../src/filenames.c ../src/filenames.h ../src/date_parsing.c ../src/date_parsing.h ../src/patterns.c ../src/patterns.h mocks.c mocks.h

test_filenames_LDADD = $(GLIBS_LIBS) $(CURL_LIBS)
