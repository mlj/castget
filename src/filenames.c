/*
  Copyright (C) 2010 Tony Armistead
  Copyright (C) 2017 Marius L. Jøhndal

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

#include <glib.h>
#include "filenames.h"
#include "patterns.h"
#include "date_parsing.h"

static gchar *guess_filename_from_url(const gchar *url);
static gchar *sanitise_filename(const gchar *filename);

/* Determine filename of enclosure. */
/* Chop off ? and # and anything following that from the basename. */
static gchar *guess_filename_from_url(const gchar *url)
{
  gchar *basename = g_path_get_basename(url);
  gchar **tokens = g_strsplit_set(basename, "?#", 2);
  gchar *guess = g_strdup(tokens[0]);

  g_free(basename);
  g_strfreev(tokens);

  return guess;
}

/* Removes unsafe and undesirable characters from a filename. The caller
   must free the returned string with g_free(). */
static gchar *sanitise_filename(const gchar *filename)
{
  static const gchar *bad_characters = "/\\?%*:|\"<>,'\n\t\r";
  gchar **parts = g_strsplit_set(filename, bad_characters, -1);
  gchar *new_filename = g_strjoinv(NULL, parts);
  g_strfreev(parts);
  return new_filename;
}

gchar *build_enclosure_filename(const char *spool_directory, const char *filename_pattern,
    const channel_info *channel_info, const rss_item *item)
{
  gchar *filename;
  gchar *sanitised_filename;
  gchar *pathname;

  if (filename_pattern)
    filename = expand_string_with_patterns(filename_pattern, channel_info, item);
  else
    filename = guess_filename_from_url(item->enclosure->url);

  sanitised_filename = sanitise_filename(filename);
  g_free(filename);

  pathname = g_build_filename(spool_directory, sanitised_filename, NULL);
  g_free(sanitised_filename);

  return pathname;
}
