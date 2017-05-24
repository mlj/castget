/*
  Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 Marius L. Jøhndal

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

#ifndef RSS_H
#define RSS_H

#include "channel.h"

typedef struct _rss_item {
  char *title;
  char *link;
  char *description;
  char *pub_date;
  enclosure *enclosure;
} rss_item;

enum rss_version {
  RSS_UNKNOWN,
  RSS_VERSION_0_91,
  RSS_VERSION_0_92,
  RSS_VERSION_2_0
};

typedef struct _rss_file {
  enum rss_version version;
  int num_items;
  rss_item **items;
  channel_info channel_info;
  gchar *fetched_time;
} rss_file;

rss_file *rss_open_file(const char *filename);
rss_file *rss_open_url(const char *url, int debug);
void rss_close(rss_file *f);

#endif /* RSS_H */
