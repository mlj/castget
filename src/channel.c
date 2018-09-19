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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <glib.h>
#include <glib/gprintf.h>
#include "libxmlutil.h"
#include "urlget.h"
#include "channel.h"
#include "rss.h"
#include "utils.h"
#include "progress.h"
#include "filenames.h"

static int _enclosure_pattern_match(enclosure_filter *filter,
                                    const enclosure *enclosure);

static void _enclosure_iterator(const void *user_data, int i, const xmlNode *node)
{
  const char *downloadtime;

  channel *c = (channel *)user_data;

  downloadtime = libxmlutil_attr_as_string(node, "downloadtime");

  if (downloadtime)
    downloadtime = g_strdup(downloadtime);
  else
    downloadtime = get_rfc822_time();

  g_hash_table_insert(c->downloaded_enclosures,
                      (gpointer)libxmlutil_attr_as_string(node, "url"),
                      (gpointer)downloadtime);
}

channel *channel_new(const char *url, const char *channel_file,
                     const char *spool_directory,
                     const char *filename_pattern,
                     int resume)
{
  channel *c;
  xmlDocPtr doc;
  xmlNode *root_element = NULL;
  const char *s;

  c = (channel *)malloc(sizeof(struct _channel));
  c->url = g_strdup(url);
  c->channel_filename = g_strdup(channel_file);
  c->spool_directory = g_strdup(spool_directory);
  c->filename_pattern = g_strdup(filename_pattern);
  //  c->resume = resume;
  c->rss_last_fetched = NULL;
  c->downloaded_enclosures = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);

  if (g_file_test(c->channel_filename, G_FILE_TEST_EXISTS)) {
    doc = xmlReadFile(c->channel_filename, NULL, 0);

    if (!doc) {
      g_fprintf(stderr, "Error parsing channel file %s.\n", c->channel_filename);
      return NULL;
    }

    root_element = xmlDocGetRootElement(doc);

    if (!root_element)  {
      xmlFreeDoc(doc);

      g_fprintf(stderr, "Error parsing channel file %s.\n", c->channel_filename);
      return NULL;
    }

    /* Fetch channel attributes. */
    s = libxmlutil_attr_as_string(root_element, "rsslastfetched");

    if (s)
      c->rss_last_fetched = g_strdup(s);

    /* Iterate encolsure elements. */
    libxmlutil_iterate_by_tag_name(root_element, "enclosure", c, _enclosure_iterator);

    xmlFreeDoc(doc);
  }

  return c;
}

static void _cast_channel_save_downloaded_enclosure(gpointer key, gpointer value,
                                                    gpointer user_data)
{
  FILE *f = (FILE *)user_data;
  gchar *escaped_key = g_markup_escape_text(key, -1);

  if (value)
    g_fprintf(f, "  <enclosure url=\"%s\" downloadtime=\"%s\"/>\n",
              escaped_key, (gchar *)value);
  else
    g_fprintf(f, "  <enclosure url=\"%s\"/>\n", escaped_key);

  g_free(escaped_key);
}

static int _cast_channel_save_channel(FILE *f, gpointer user_data, int debug)
{
  channel *c = (channel *)user_data;

  g_fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");

  if (c->rss_last_fetched)
    g_fprintf(f, "<channel version=\"1.0\" rsslastfetched=\"%s\">\n", c->rss_last_fetched);
  else
    g_fprintf(f, "<channel version=\"1.0\">\n");

  g_hash_table_foreach(c->downloaded_enclosures, _cast_channel_save_downloaded_enclosure, f);

  g_fprintf(f, "</channel>\n");

  return 0;
}

static void _cast_channel_save(channel *c, int debug)
{
  write_by_temporary_file(c->channel_filename, _cast_channel_save_channel, c, NULL, debug);
}

void channel_free(channel *c)
{
  g_hash_table_destroy(c->downloaded_enclosures);
  g_free(c->spool_directory);
  g_free(c->channel_filename);
  g_free(c->url);
  g_free(c->filename_pattern);
  free(c);
}

static size_t _enclosure_urlget_cb(void *buffer, size_t size, size_t nmemb, void *user_data)
{
  FILE *f = (FILE *)user_data;

  return fwrite(buffer, size, nmemb, f);
}

static rss_file *_get_rss(channel *c, void *user_data, channel_callback cb, int debug)
{
  rss_file *f;

  if (cb)
    cb(user_data, CCA_RSS_DOWNLOAD_START, NULL, NULL, NULL);

  if (!strncmp("http://", c->url, strlen("http://"))
      || !strncmp("https://", c->url, strlen("https://")))
    f = rss_open_url(c->url, debug);
  else
    f = rss_open_file(c->url);

  if (cb)
    cb(user_data, CCA_RSS_DOWNLOAD_END, &(f->channel_info), NULL, NULL);

  return f;
}

static int _do_download(channel *c, channel_info *channel_info, rss_item *item,
                        void *user_data, channel_callback cb, int resume,
                        int debug, int show_progress_bar)
{
  int download_failed;
  long resume_from = 0;
  gchar *enclosure_full_filename;
  FILE *enclosure_file;
  struct stat fileinfo;
  progress_bar *pb;

  /* Check that the spool directory exists. */
  if (!g_file_test(c->spool_directory, G_FILE_TEST_IS_DIR)) {
    g_fprintf(stderr, "Spool directory %s not found.\n", c->spool_directory);
    return 1;
  }

  /* Build enclosure filename. */
  enclosure_full_filename = build_enclosure_filename(c->spool_directory,
    c->filename_pattern, channel_info, item);

  if (g_file_test(enclosure_full_filename, G_FILE_TEST_EXISTS)) {
    /* A file with the same filename already exists. If the user has asked us
       to resume downloads, we should append to the file. Otherwise we should
       refuse to continue. If the feed uses the same filename for each
       enclosure, running in append mode will corrupt existing files. There is
       probably no practical way to avoid this, and the issue is documented in
       castget(1) and castgetrc(5). */
    if (resume) {
      /* Set resume offset to the size of the file as it is now (and use
         non-append mode if the size is zero or stat() fails). */
      if (0 == stat(enclosure_full_filename, &fileinfo))
        resume_from = fileinfo.st_size;
      else
        resume_from = 0;
    } else {
      /* File exists but user does not allow us to append so we have to abort. */
      g_fprintf(stderr, "Enclosure file %s already exists.\n", enclosure_full_filename);
      g_free(enclosure_full_filename);
      return 1;
    }
  } else
    /* By letting the offset be 0 we will write in non-append mode. */
    resume_from = 0;

  enclosure_file = fopen(enclosure_full_filename, resume_from ? "ab" : "wb");

  if (!enclosure_file) {
    g_fprintf(stderr, "Error opening enclosure file %s.\n", enclosure_full_filename);
    g_free(enclosure_full_filename);
    return 1;
  }

  if (cb)
    cb(user_data, CCA_ENCLOSURE_DOWNLOAD_START, channel_info, item->enclosure, enclosure_full_filename);

  if (show_progress_bar)
    pb = progress_bar_new(resume_from);
  else
    pb = NULL;

  if (urlget_buffer(item->enclosure->url, enclosure_file, _enclosure_urlget_cb, resume_from, debug, pb)) {
    g_fprintf(stderr, "Error downloading enclosure from %s.\n", item->enclosure->url);

    download_failed = 1;
  } else
    download_failed = 0;

  if (pb)
    progress_bar_free(pb);

  fclose(enclosure_file);

  if (cb)
    cb(user_data, CCA_ENCLOSURE_DOWNLOAD_END, channel_info, item->enclosure, enclosure_full_filename);

  g_free(enclosure_full_filename);

  return download_failed;
}

static int _do_catchup(channel *c, channel_info *channel_info, rss_item *item,
                       void *user_data, channel_callback cb)
{
  if (cb) {
    cb(user_data, CCA_ENCLOSURE_DOWNLOAD_START, channel_info, item->enclosure, NULL);

    cb(user_data, CCA_ENCLOSURE_DOWNLOAD_END, channel_info, item->enclosure, NULL);
  }

  return 0;
}

int channel_update(channel *c, void *user_data, channel_callback cb,
                   int no_download, int no_mark_read, int first_only,
                   int resume, enclosure_filter *filter, int debug,
                   int show_progress_bar)
{
  int i, download_failed;
  rss_file *f;

  /* Retrieve the RSS file. */
  f = _get_rss(c, user_data, cb, debug);

  if (!f)
    return 1;

  /* Check enclosures in RSS file. */
  for (i = 0; i < f->num_items; i++)
    if (f->items[i]->enclosure) {
      if (!g_hash_table_lookup_extended(c->downloaded_enclosures, f->items[i]->enclosure->url, NULL, NULL)) {
        rss_item *item;

        item = f->items[i];

        if (!filter || _enclosure_pattern_match(filter, item->enclosure)) {
          if (no_download)
            download_failed = _do_catchup(c, &(f->channel_info), item, user_data, cb);
          else
            download_failed = _do_download(c, &(f->channel_info), item, user_data, cb, resume, debug, show_progress_bar);

          if (download_failed)
            break;

          if (!no_mark_read) {
            /* Mark enclosure as downloaded and immediately save channel
               file to ensure that it reflects the change. */
            g_hash_table_insert(c->downloaded_enclosures, f->items[i]->enclosure->url,
                                (gpointer)get_rfc822_time());

            _cast_channel_save(c, debug);
          }

          /* If we have been instructed to deal only with the first
             available enclosure, it is time to break out of the loop. */
          if (first_only)
            break;
        }
      }
    }

  if (!no_mark_read) {
    /* Update the RSS last fetched time and save the channel file again. */

    if (c->rss_last_fetched)
      g_free(c->rss_last_fetched);

    c->rss_last_fetched = g_strdup(f->fetched_time);

    _cast_channel_save(c, debug);
  }

  rss_close(f);

  return 0;
}

/* Match the (file) name of an enclosure against a regexp. Letters
   in the pattern match both upper and lower case letters if
   'caseless' is TRUE. Returns TRUE if the pattern matches, FALSE
   otherwise. */
static gboolean _enclosure_pattern_match(enclosure_filter *filter,
                                         const enclosure *enclosure)
{
  GError *error = NULL;
  GRegexCompileFlags compile_options = 0;
  GRegexMatchFlags match_options = 0;
  GRegex *regex;
  gboolean match;

  g_assert(filter);
  g_assert(filter->pattern);
  g_assert(enclosure);

  if (filter->caseless)
    compile_options |= G_REGEX_CASELESS;

  regex = g_regex_new(filter->pattern, compile_options, match_options,
                      &error);

  if (error) {
    fprintf(stderr, "Error compiling regular expression %s: %s\n",
            filter->pattern, error->message);
    g_error_free(error);
    return FALSE;
  }

  match = g_regex_match(regex, enclosure->url, match_options, NULL);

  g_regex_unref(regex);

  return match;
}

enclosure_filter *enclosure_filter_new(const gchar *pattern,
                                       gboolean caseless)
{
  enclosure_filter *e = g_malloc(sizeof(struct _enclosure_filter));

  g_assert(pattern);

  e->pattern = g_strdup(pattern);
  e->caseless = caseless;

  return e;
}

void enclosure_filter_free(enclosure_filter *e)
{
  g_free(e->pattern);
  g_free(e);
}
