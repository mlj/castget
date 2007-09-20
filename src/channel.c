/*
  Copyright (C) 2005, 2006, 2007 Marius L. Jøhndal
 
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
 
  $Id: channel.c,v 1.2 2007/09/20 18:10:51 mariuslj Exp $
  
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
                     const char *spool_directory, int resume)
{
  channel *c;
  xmlDocPtr doc;
  xmlNode *root_element = NULL;
  const char *s;

  c = (channel *)malloc(sizeof(struct _channel));
  c->url = g_strdup(url);
  c->channel_filename = g_strdup(channel_file);
  c->spool_directory = g_strdup(spool_directory);
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

static int _cast_channel_save_channel(FILE *f, gpointer user_data)
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

static void _cast_channel_save(channel *c)
{
  write_by_temporary_file(c->channel_filename, _cast_channel_save_channel, c, NULL);
}

void channel_free(channel *c)
{
  g_hash_table_destroy(c->downloaded_enclosures);
  g_free(c->spool_directory);
  g_free(c->channel_filename);
  g_free(c->url);
  free(c);
}

static size_t _enclosure_urlget_cb(void *buffer, size_t size, size_t nmemb, void *user_data)
{
  FILE *f = (FILE *)user_data;

  return fwrite(buffer, size, nmemb, f);
}

static rss_file *_get_rss(channel *c, void *user_data, channel_callback cb)
{
  rss_file *f;

  if (cb)
    cb(user_data, CCA_RSS_DOWNLOAD_START, NULL, NULL, NULL);

  if (!strncmp("http://", c->url, strlen("http://")))
    f = rss_open_url(c->url);
  else
    f = rss_open_file(c->url);

  if (cb)
    cb(user_data, CCA_RSS_DOWNLOAD_END, &(f->channel_info), NULL, NULL);

  return f;
}

static int _do_download(channel *c, channel_info *channel_info, rss_item *item, 
                        void *user_data, channel_callback cb, int resume)
{
  int download_failed;
  long resume_from = 0;
  gchar *enclosure_full_filename;
  FILE *enclosure_file;
  struct stat fileinfo;

  /* Check that the spool directory exists. */
  if (!g_file_test(c->spool_directory, G_FILE_TEST_IS_DIR)) {
    g_fprintf(stderr, "Spool directory %s not found.\n", c->spool_directory);
    return 1;
  }

  /* Build enclosure file name and open file. */
  enclosure_full_filename = g_build_filename(c->spool_directory, item->enclosure->filename, NULL);

  if (resume) {
    /* We're told to continue from where we are now. Get the
     * size of the file as it is now and open it for append instead.
     * Stolen from curl. */
    
    if (0 == stat(enclosure_full_filename, &fileinfo))
      /* Set offset to current file size. */
      resume_from = fileinfo.st_size;
    else
      /* Let offset be 0. */
      resume_from = 0;
  }

  enclosure_file = fopen(enclosure_full_filename, resume_from ? "ab" : "wb");

  if (!enclosure_file) {
    g_free(enclosure_full_filename);
    
    g_fprintf(stderr, "Error opening enclosure file %s.\n", enclosure_full_filename);
    return 1;
  }
  
  if (cb)
    cb(user_data, CCA_ENCLOSURE_DOWNLOAD_START, channel_info, item->enclosure, enclosure_full_filename);
  
  if (urlget_buffer(item->enclosure->url, enclosure_file, _enclosure_urlget_cb, resume_from)) {
    g_fprintf(stderr, "Error downloading enclosure from %s.\n", item->enclosure->url);

    download_failed = 1;
  } else
    download_failed = 0;
  
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
                   int no_download, int no_mark_read, int first_only, int resume)
{
  int i, download_failed;
  rss_file *f;

  /* Retrieve the RSS file. */
  f = _get_rss(c, user_data, cb);

  if (!f)
    return 1;

  /* Check enclosures in RSS file. */
  for (i = 0; i < f->num_items; i++)
    if (f->items[i]->enclosure) {
      if (!g_hash_table_lookup_extended(c->downloaded_enclosures, f->items[i]->enclosure->url, NULL, NULL)) {
        if (no_download)
          download_failed = _do_catchup(c, &(f->channel_info), f->items[i], user_data, cb);
        else
          download_failed = _do_download(c, &(f->channel_info), f->items[i], user_data, cb, resume);

        if (download_failed)
          break;

        if (!no_mark_read) {
          /* Mark enclosure as downloaded and immediately save channel
             file to ensure that it reflects the change. */
          g_hash_table_insert(c->downloaded_enclosures, f->items[i]->enclosure->url, 
                              (gpointer)get_rfc822_time());

          _cast_channel_save(c);
        }

        /* If we have been instructed to deal only with the first
           available enclosure, it is time to break out of the loop. */
        if (first_only)
          break;
      }
    }

  if (!no_mark_read) {
    /* Update the RSS last fetched time and save the channel file again. */

    if (c->rss_last_fetched)
      g_free(c->rss_last_fetched);

    c->rss_last_fetched = g_strdup(f->fetched_time);

    _cast_channel_save(c);
  }

  rss_close(f);

  return 0;
}

/* 
   Local Variables:
   mode:c
   indent-tabs-mode:nil
   c-basic-offset:2
   coding:utf-8
   End:
*/
