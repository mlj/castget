/*
  Copyright (C) 2018 Marius L. Jøhndal

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

#ifndef MOCKS_H
#define MOCKS_H

#include "../src/channel.h"
#include "../src/patterns.h"

rss_item *mock_rss_item_new(char *item_title, char *item_pub_date);
channel_info *mock_channel_info_new(char *title);
void mock_rss_item_free(rss_item *item);
void mock_channel_info_free(channel_info *info);

#endif /* MOCKS_H */
