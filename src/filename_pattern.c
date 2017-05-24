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
#include "filename_pattern.h"
#include "date_parsing.h"

static gchar *remove_bad_filename_chars(const char *filename);
static gchar *form_field_string(const rss_item *item, const gchar *field_name);
static gchar *form_userspec_filename(const gchar *user_file_spec,
                                     const rss_item *item);

gchar *build_enclosure_filename(const char *spool_directory,
                                const char *filename_pattern,
                                const rss_item *item)
{
  const char *filename;

  if (filename_pattern)
    filename = form_userspec_filename(filename_pattern, item);
  else
    filename = item->enclosure->filename;

  return g_build_filename(spool_directory, filename, NULL);
}

/* Returns a string with all characters removed which may cause problems if
 * included as part of a filename.
 * @param filename the input filename
 * @return a gchar* string which is the input string with risky chars removed.
 *         This string should be freed with g_free().
 */
static gchar *remove_bad_filename_chars(const char *filename)
{
  static const gchar *bad_filename_chars = "/\\?%*:|\"<>.,'";
  gchar **parts = g_strsplit_set(filename,  bad_filename_chars, -1);
  gchar *good_filename = g_strjoinv(NULL, parts);
  g_strfreev(parts);
  return good_filename;
}

/* Forms the field string for one of the know field names. Supported field names
 * are:
 *  	date 	- returns the publication date or the current date if no publication
 *  		  date was included in the rss entry
 *  	title	- the episode title which has been modified to remove any chars
 *  		  which could cause problems if used as part of a filename
 *  If an unknown field name is specified it is mapped to a single '_' character.
 *  @param item the episode rss info
 *  @param field_name one of the supported field names
 */
static gchar *form_field_string(const rss_item *item, const gchar *field_name)
{
  if (g_ascii_strcasecmp("date", field_name) == 0) {
    gchar str_date[20];

    GDate *rfc822_date = parse_rfc822_date(item->pub_date);

    if (!rfc822_date) {
      /* If pubDate from the feed is invalid, use today's date. */
      GTimeVal today;
      g_get_current_time(&today);
      g_date_set_time_val(rfc822_date, &today);
    }
    g_date_strftime(str_date, sizeof(str_date), "%d-%m-%y", rfc822_date);
    g_date_free(rfc822_date);
    return g_strdup(str_date);
  } else if (g_ascii_strcasecmp("title", field_name) == 0) {
    return remove_bad_filename_chars(item->title);
  }

  /* Unknown field string so just use an '_' */
  return g_strnfill(1, '_');
}

/*
 * Forms a user specified filename for an rss episode.
 * @param user_file_spec the pattern string which specifies how we construct
 * 	  the filename part
 * @param item the rss episode info
 */

#define DIM(a) sizeof(a)/sizeof(a[0])

static gchar *form_userspec_filename(const gchar *user_file_spec,
                                     const rss_item *item)
{
  enum { STATE_COPY, STATE_FIELD, STATE_DONE } state = STATE_COPY;
  GString *filename_parts;
  gchar fieldname[80];
  int fieldname_idx = 0;
  gchar *processed_filename;
  gchar *expanded_field;
  const gchar *cp_start = user_file_spec;
  const gchar *cp_end = user_file_spec;

  filename_parts = g_string_new(NULL);

  while (state != STATE_DONE) {
    switch (state) {
    case STATE_COPY:
      /* Process */
      if ((*cp_end == '\0' || *cp_end == '%') && cp_end > cp_start)
        g_string_append_len(filename_parts, cp_start, cp_end - cp_start);

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
        expanded_field = form_field_string(item, fieldname);
        g_string_append(filename_parts, expanded_field);
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
  processed_filename = g_string_free(filename_parts, FALSE);

  return processed_filename;
}
