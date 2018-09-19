/*
  Copyright (C) 2010 Tony Armistead
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <glib.h>
#include "patterns.h"
#include "date_parsing.h"

static gchar *expand_date_pattern(const rss_item *item);
static gchar *expand_title_pattern(const rss_item *item);
static gchar *expand_channel_title_pattern(const channel_info *info);
static gchar *expand_pattern(const channel_info *channel_info,
  const rss_item *item, const gchar *pattern);

/* Expands a 'date' pattern to the value of the item's pub_date. Returns
   an empty string if pub_date is absent or invalid. Formats the date
   on the format 'YYYY-MM-DD'. Caller must free returned string with g_free. */
static gchar *expand_date_pattern(const rss_item *item)
{
  gchar str_date[20]; // FIXME: suspicious

  if (item->pub_date) {
    GDate *rfc822_date = parse_rfc822_date(item->pub_date);

    if (rfc822_date) {
      g_date_strftime(str_date, sizeof(str_date), "%Y-%m-%d", rfc822_date);
      g_date_free(rfc822_date);
      return g_strdup(str_date);
    }
  }

  return g_strdup("");
}

/* Expands a 'title' pattern to the value of the item's title. Returns
   an empty string if title is absent. Caller must free returned string with
   g_free. */
static gchar *expand_title_pattern(const rss_item *item)
{
  if (item->title)
    return g_strdup(item->title);
  else
    return g_strdup("");
}

/* Expands a 'channel_title' pattern to the value of the channel's title.
   Returns an empty string if title is absent. Caller must free returned string
   with g_free. */
static gchar *expand_channel_title_pattern(const channel_info *info)
{
  if (info->title)
    return g_strdup(info->title);
  else
    return g_strdup("");
}

/* Expands a pattern to values from the RSS field's item. Returns an empty
   string if pattern is invalid or if it cannot be expanded. Caller must free
   returned string with g_free. */
static gchar *expand_pattern(const channel_info *channel_info,
  const rss_item *item, const gchar *pattern)
{
  if (g_ascii_strcasecmp(pattern, "date") == 0)
    return expand_date_pattern(item);
  else if (g_ascii_strcasecmp(pattern, "title") == 0)
    return expand_title_pattern(item);
  else if (g_ascii_strcasecmp(pattern, "channel_title") == 0)
    return expand_channel_title_pattern(channel_info);
  else
    return g_strdup("");
}

#define DIM(a) sizeof(a)/sizeof(a[0])

gchar *expand_string_with_patterns(const gchar *string,
  const channel_info *channel_info, const rss_item *item)
{
  enum { STATE_COPY, STATE_FIELD, STATE_DONE } state = STATE_COPY;
  GString *parts;
  gchar fieldname[80];
  int fieldname_idx = 0;
  gchar *expanded_field;
  const gchar *cp_start = string;
  const gchar *cp_end = string;

  parts = g_string_new(NULL);

  while (state != STATE_DONE) {
    switch (state) {
    case STATE_COPY:
      /* Process */
      if ((*cp_end == '\0' || *cp_end == '%') && cp_end > cp_start)
        g_string_append_len(parts, cp_start, cp_end - cp_start);

      /* State update */
      if (*cp_end == '%') {
        state = STATE_FIELD;
        fieldname_idx = 0;
      }
      if (*cp_end == '\0')
        state = STATE_DONE;
      break;
    case STATE_FIELD:
      /* Process */
      if (*cp_end == '\0' || *cp_end == ')') {
        fieldname[fieldname_idx] = '\0';
        expanded_field = expand_pattern(channel_info, item, fieldname);
        g_string_append(parts, expanded_field);
        g_free(expanded_field);
      }
      else if ((*cp_end != '(') && (fieldname_idx < DIM(fieldname)-1))
        fieldname[fieldname_idx++] = *cp_end;
      /* State update */
      if (*cp_end == '\0')
        state = STATE_DONE;
      else if (*cp_end == ')') {
        state = STATE_COPY;
        cp_start = cp_end + 1;
      }
      break;
    case STATE_DONE:
      break;
    }
    ++cp_end;
  }

  /* Free GString but keep actual string data. Caller must free this using g_free() */
  return g_string_free(parts, FALSE);
}
