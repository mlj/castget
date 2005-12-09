/*
  Copyright (C) 2005 Marius L. Jøhndal
 
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
 
  $Id: configuration.c,v 1.2 2005/12/09 05:53:00 mariuslj Exp $
  
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

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
  c->playlist = _read_channel_configuration_key(kf, identifier, "playlist");
  c->id3_lead_artist = _read_channel_configuration_key(kf, identifier, "id3leadartist");
  c->id3_content_group = _read_channel_configuration_key(kf, identifier, "id3contentgroup");
  c->id3_title = _read_channel_configuration_key(kf, identifier, "id3title");
  c->id3_album = _read_channel_configuration_key(kf, identifier, "id3album");
  c->id3_content_type = _read_channel_configuration_key(kf, identifier, "id3contenttype");
  c->id3_year = _read_channel_configuration_key(kf, identifier, "id3year");
  c->id3_comment = _read_channel_configuration_key(kf, identifier, "id3comment");

  /* Populate with defaults if necessary. */
  if (defaults) {
    if (!c->url && defaults->url)
      c->url = g_strdup(defaults->url);

    if (!c->spool_directory && defaults->spool_directory)
      c->spool_directory = g_strdup(defaults->spool_directory);

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
  }

  return c;
}

/*
   Local Variables:
   mode:c
   indent-tabs-mode:nil
   c-basic-offset:2
   coding:utf-8
   End:
*/
