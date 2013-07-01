/*
  Copyright (C) 2005-2017 Marius L. Jøhndal

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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <glib.h>

struct channel_configuration {
  gchar *identifier;
  gchar *url;
  gchar *spool_directory;
  gchar *filename_pattern;
  gchar *playlist;
  gchar *id3_lead_artist;
  gchar *id3_content_group;
  gchar *id3_title;
  gchar *id3_album;
  gchar *id3_content_type;
  gchar *id3_year;
  gchar *id3_comment;
  gchar *regex_filter;
};

struct channel_configuration *channel_configuration_new(GKeyFile *kf, const gchar *identifier,
                                                        struct channel_configuration *defaults);
void channel_configuration_free(struct channel_configuration *c);
int channel_configuration_verify_keys(GKeyFile *kf, const char *identifier);

#endif /* CONFIGURATION_H */
