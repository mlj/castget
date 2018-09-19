/*
  Copyright (C) 2010 Tony Armitstead
  Copyright (C) 2017-2018 Marius L. Jøhndal

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#ifndef FILENAME_PATTERN_H
#define FILENAME_PATTERN_H

#include "rss.h"

gchar *build_enclosure_filename(const char *spool_directory,
                                const char *filename_pattern,
                                const channel_info *channel_info,
                                const rss_item *item);

#endif /* FILENAME_PATTERN_H */
