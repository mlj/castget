/*
  Copyright (C) 2005-2018 Marius L. Jøhndal
  Copyright (C) 2010 Tony Armitstead

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */
#include <string.h>
#include <stdlib.h>
#include <glib/gstdio.h>

#include "configuration.h"

static gchar *_read_channel_configuration_key(GKeyFile *kf, const gchar *identifier,
                                              const gchar *key)
{
  GError *error = NULL;

  if (g_key_file_has_key(kf, identifier, key, &error))
    return g_strdup(g_key_file_get_value(kf, identifier, key, &error));

  return NULL;
}

void channel_configuration_free(struct channel_configuration *c)
{
  g_free(c->identifier);

  if (c->url)
    g_free(c->url);

  if (c->spool_directory)
    g_free(c->spool_directory);

  if (c->filename_pattern)
    g_free(c->filename_pattern);

  if (c->playlist)
    g_free(c->playlist);

  if (c->id3_lead_artist)
    g_free(c->id3_lead_artist);

  if (c->id3_content_group)
    g_free(c->id3_content_group);

  if (c->id3_title)
    g_free(c->id3_title);

  if (c->id3_album)
    g_free(c->id3_album);

  if (c->id3_content_type)
    g_free(c->id3_content_type);

  if (c->id3_year)
    g_free(c->id3_year);

  if (c->id3_comment)
    g_free(c->id3_comment);

  if (c->regex_filter)
    g_free(c->regex_filter);

  g_free(c);
}

struct channel_configuration *channel_configuration_new(GKeyFile *kf, const gchar *identifier,
                                                        struct channel_configuration *defaults)
{
  struct channel_configuration *c;

  g_assert(g_key_file_has_group(kf, identifier));

  /* Allocate new structure and save identifierr. */
  c = (struct channel_configuration *)g_malloc(sizeof(struct channel_configuration));

  c->identifier = g_strdup(identifier);

  /* Read keys from configuration file. */
  c->url = _read_channel_configuration_key(kf, identifier, "url");
  c->spool_directory = _read_channel_configuration_key(kf, identifier, "spool");
  c->filename_pattern = _read_channel_configuration_key(kf, identifier, "filename");
  c->playlist = _read_channel_configuration_key(kf, identifier, "playlist");
  c->id3_lead_artist = _read_channel_configuration_key(kf, identifier, "id3leadartist");
  c->id3_content_group = _read_channel_configuration_key(kf, identifier, "id3contentgroup");
  c->id3_title = _read_channel_configuration_key(kf, identifier, "id3title");
  c->id3_album = _read_channel_configuration_key(kf, identifier, "id3album");
  c->id3_content_type = _read_channel_configuration_key(kf, identifier, "id3contenttype");
  c->id3_year = _read_channel_configuration_key(kf, identifier, "id3year");
  c->id3_comment = _read_channel_configuration_key(kf, identifier, "id3comment");
  c->regex_filter = _read_channel_configuration_key(kf, identifier, "filter");

  /* Populate with defaults if necessary. */
  if (defaults) {
    if (!c->url && defaults->url)
      c->url = g_strdup(defaults->url);

    if (!c->spool_directory && defaults->spool_directory)
      c->spool_directory = g_strdup(defaults->spool_directory);

    if (!c->filename_pattern && defaults->filename_pattern)
      c->filename_pattern = g_strdup(defaults->filename_pattern);

    if (!c->playlist && defaults->playlist)
      c->playlist = g_strdup(defaults->playlist);

    if (!c->id3_lead_artist && defaults->id3_lead_artist)
      c->id3_lead_artist = g_strdup(defaults->id3_lead_artist);

    if (!c->id3_content_group && defaults->id3_content_group)
      c->id3_content_group = g_strdup(defaults->id3_content_group);

    if (!c->id3_title && defaults->id3_title)
      c->id3_title = g_strdup(defaults->id3_title);

    if (!c->id3_album && defaults->id3_album)
      c->id3_album = g_strdup(defaults->id3_album);

    if (!c->id3_content_type && defaults->id3_content_type)
      c->id3_content_type = g_strdup(defaults->id3_content_type);

    if (!c->id3_year && defaults->id3_year)
      c->id3_year = g_strdup(defaults->id3_year);

    if (!c->id3_comment && defaults->id3_comment)
      c->id3_comment = g_strdup(defaults->id3_comment);

    if (!c->regex_filter && defaults->regex_filter)
      c->regex_filter = g_strdup(defaults->regex_filter);
  }

  return c;
}

int channel_configuration_verify_keys(GKeyFile *kf, const char *identifier)
{
  int i;
  gchar **key_list;

  key_list = g_key_file_get_keys(kf, identifier, NULL, NULL);

  if (!key_list) {
    fprintf(stderr, "Error reading keys in configuration of channel %s.\n", identifier);

    return -1;
  }

  for (i = 0; key_list[i]; i++) {
    if (! (!strcmp(key_list[i], "url") ||
           !strcmp(key_list[i], "spool") ||
           !strcmp(key_list[i], "filename") ||
           !strcmp(key_list[i], "playlist") ||
           !strcmp(key_list[i], "id3leadartist") ||
           !strcmp(key_list[i], "id3contentgroup") ||
           !strcmp(key_list[i], "id3title") ||
           !strcmp(key_list[i], "id3album") ||
           !strcmp(key_list[i], "id3contenttype") ||
           !strcmp(key_list[i], "id3year") ||
           !strcmp(key_list[i], "id3comment") ||
           !strcmp(key_list[i], "filter"))) {
      fprintf(stderr, "Invalid key %s in configuration of channel %s.\n", key_list[i], identifier);
      return -1;
    }
  }

  g_strfreev(key_list);

  return 0;
}
