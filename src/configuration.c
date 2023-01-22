/*
  Copyright (C) 2005-2021 Marius L. Jøhndal
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

#include "configuration.h"

#include <glib/gstdio.h>
#include <stdlib.h>
#include <string.h>

static gchar *_read_channel_configuration_key(GKeyFile *kf,
                                              const gchar *identifier,
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

  if (c->artist_tag)
    g_free(c->artist_tag);

  if (c->title_tag)
    g_free(c->title_tag);

  if (c->album_tag)
    g_free(c->album_tag);

  if (c->genre_tag)
    g_free(c->genre_tag);

  if (c->year_tag)
    g_free(c->year_tag);

  if (c->comment_tag)
    g_free(c->comment_tag);

  if (c->regex_filter)
    g_free(c->regex_filter);

  g_free(c);
}

struct channel_configuration *channel_configuration_new(
    GKeyFile *kf, const gchar *identifier,
    struct channel_configuration *defaults)
{
  struct channel_configuration *c;

  g_assert(g_key_file_has_group(kf, identifier));

  /* Allocate new structure and save identifierr. */
  c = (struct channel_configuration *)g_malloc(
      sizeof(struct channel_configuration));

  c->identifier = g_strdup(identifier);

  /* Read keys from configuration file. */
  c->url = _read_channel_configuration_key(kf, identifier, "url");
  c->spool_directory = _read_channel_configuration_key(kf, identifier, "spool");
  c->filename_pattern =
      _read_channel_configuration_key(kf, identifier, "filename");
  c->playlist = _read_channel_configuration_key(kf, identifier, "playlist");
  c->artist_tag = _read_channel_configuration_key(kf, identifier, "artist_tag");
  c->title_tag = _read_channel_configuration_key(kf, identifier, "title_tag");
  c->album_tag = _read_channel_configuration_key(kf, identifier, "album_tag");
  c->genre_tag = _read_channel_configuration_key(kf, identifier, "genre_tag");
  c->year_tag = _read_channel_configuration_key(kf, identifier, "year_tag");
  c->comment_tag =
      _read_channel_configuration_key(kf, identifier, "comment_tag");
  c->regex_filter = _read_channel_configuration_key(kf, identifier, "filter");
  c->user_agent = _read_channel_configuration_key(kf, identifier, "user_agent");

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

    if (!c->artist_tag && defaults->artist_tag)
      c->artist_tag = g_strdup(defaults->artist_tag);

    if (!c->title_tag && defaults->title_tag)
      c->title_tag = g_strdup(defaults->title_tag);

    if (!c->album_tag && defaults->album_tag)
      c->album_tag = g_strdup(defaults->album_tag);

    if (!c->genre_tag && defaults->genre_tag)
      c->genre_tag = g_strdup(defaults->genre_tag);

    if (!c->year_tag && defaults->year_tag)
      c->year_tag = g_strdup(defaults->year_tag);

    if (!c->comment_tag && defaults->comment_tag)
      c->comment_tag = g_strdup(defaults->comment_tag);

    if (!c->regex_filter && defaults->regex_filter)
      c->regex_filter = g_strdup(defaults->regex_filter);

    if (!c->user_agent && defaults->user_agent)
      c->user_agent = g_strdup(defaults->user_agent);
  }

  return c;
}

int channel_configuration_verify_keys(GKeyFile *kf, const char *identifier)
{
  int i;
  gchar **key_list;

  key_list = g_key_file_get_keys(kf, identifier, NULL, NULL);

  if (!key_list) {
    fprintf(stderr, "Error reading keys in configuration of channel %s.\n",
            identifier);

    return -1;
  }

  for (i = 0; key_list[i]; i++) {
    if (!strcmp(key_list[i], "id3contentgroup"))
      fprintf(stderr, "Key id3contentgroup no longer supported.\n");
    else if (!strcmp(key_list[i], "id3leadartist"))
      fprintf(stderr,
              "Key id3leadartist no longer supported. Please use artist_tag "
              "instead.\n");
    else if (!strcmp(key_list[i], "id3title"))
      fprintf(
          stderr,
          "Key id3title no longer supported. Please use title_tag instead.\n");
    else if (!strcmp(key_list[i], "id3album"))
      fprintf(
          stderr,
          "Key id3album no longer supported. Please use album_tag instead.\n");
    else if (!strcmp(key_list[i], "id3contenttype"))
      fprintf(stderr,
              "Key id3contenttype no longer supported. Please use genre_tag "
              "instead.\n");
    else if (!strcmp(key_list[i], "id3year"))
      fprintf(
          stderr,
          "Key id3year no longer supported. Please use year_tag instead.\n");
    else if (!strcmp(key_list[i], "id3comment"))
      fprintf(stderr,
              "Key id3comment no longer supported. Please use comment_tag "
              "instead.\n");
    else if (!(!strcmp(key_list[i], "url") || !strcmp(key_list[i], "spool") ||
               !strcmp(key_list[i], "filename") ||
               !strcmp(key_list[i], "playlist") ||
               !strcmp(key_list[i], "artist_tag") ||
               !strcmp(key_list[i], "title_tag") ||
               !strcmp(key_list[i], "album_tag") ||
               !strcmp(key_list[i], "genre_tag") ||
               !strcmp(key_list[i], "year_tag") ||
               !strcmp(key_list[i], "comment_tag") ||
               !strcmp(key_list[i], "filter") ||
               !strcmp(key_list[i], "user_agent"))) {
      fprintf(stderr, "Invalid key %s in configuration of channel %s.\n",
              key_list[i], identifier);
      return -1;
    }
  }

  g_strfreev(key_list);

  return 0;
}
