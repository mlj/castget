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
 
  $Id: channel.c,v 1.4 2005/11/13 23:01:27 mariuslj Exp $
  
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <string.h>
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

  libcastget_channel *c = (libcastget_channel *)user_data;
  
  downloadtime = libxmlutil_attr_as_string(node, "downloadtime");

  if (downloadtime)
    downloadtime = g_strdup(downloadtime);
  else
    downloadtime = libcastget_get_rfc822_time();

  g_hash_table_insert(c->downloaded_enclosures, 
                      (gpointer)libxmlutil_attr_as_string(node, "url"), 
                      (gpointer)downloadtime);
}

libcastget_channel *libcastget_channel_new(const char *url, const char *channel_file, const char *spool_directory)
{
  libcastget_channel *c;
  xmlDocPtr doc;
  xmlNode *root_element = NULL;
  const char *s;

  c = (libcastget_channel *)malloc(sizeof(struct _libcastget_channel));
  c->url = g_strdup(url);
  c->channel_filename = g_strdup(channel_file);
  c->spool_directory = g_strdup(spool_directory);
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

  if (value)
    g_fprintf(f, "  <enclosure url=\"%s\" downloadtime=\"%s\"/>\n", 
              (gchar *)key, (gchar *)value);
  else
    g_fprintf(f, "  <enclosure url=\"%s\"/>\n", (gchar *)key);
}

static int _cast_channel_save_channel(FILE *f, gpointer user_data)
{
  libcastget_channel *c = (libcastget_channel *)user_data;

  g_fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");

  if (c->rss_last_fetched)
    g_fprintf(f, "<channel version=\"1.0\" rsslastfetched=\"%s\">\n", c->rss_last_fetched);
  else
    g_fprintf(f, "<channel version=\"1.0\">\n");

  g_hash_table_foreach(c->downloaded_enclosures, _cast_channel_save_downloaded_enclosure, f);

  g_fprintf(f, "</channel>\n");

  return 0;
}

static void _cast_channel_save(libcastget_channel *c)
{
  libcastget_write_by_temporary_file(c->channel_filename, _cast_channel_save_channel, c, NULL);
}

void libcastget_channel_free(libcastget_channel *c)
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

static rss_file *_get_rss(libcastget_channel *c, void *user_data, libcastget_channel_callback cb)
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

int libcastget_channel_update(libcastget_channel *c, void *user_data, libcastget_channel_callback cb)
{
  int i;
  rss_file *f;
  gchar *enclosure_filename, *enclosure_full_filename;
  FILE *enclosure_file;
  gchar *rss_last_fetched;

  /* Check that the spool directory exists. */
  if (!g_file_test(c->spool_directory, G_FILE_TEST_IS_DIR)) {
    g_fprintf(stderr, "Spool directory %s not found.\n", c->spool_directory);
    return 1;
  }

  f = _get_rss(c, user_data, cb);

  if (!f)
    return 1;

  /* Get the current time and keep it around for later use as the new RSS last fetched time. */
  rss_last_fetched = libcastget_get_rfc822_time();

  if (!rss_last_fetched) {
    g_fprintf(stderr, "Error retrieving current time.\n");

    rss_close(f);

    return 1;
  }

  /* Check enclosures in RSS file. */
  for (i = 0; i < f->num_items; i++)
    if (f->items[i]->enclosure) {
      if (!g_hash_table_lookup_extended(c->downloaded_enclosures, f->items[i]->enclosure->url, NULL, NULL)) {
        enclosure_filename = g_path_get_basename(f->items[i]->enclosure->url);
        enclosure_full_filename = g_build_filename(c->spool_directory, enclosure_filename, NULL);
        g_free(enclosure_filename);

        enclosure_file = fopen(enclosure_full_filename, "w");

        if (!enclosure_file) {
          g_free(enclosure_full_filename);

          g_fprintf(stderr, "Error opening enclosure file %s.\n", enclosure_full_filename);
          break;
        }

        if (cb)
          cb(user_data, CCA_ENCLOSURE_DOWNLOAD_START, &(f->channel_info), f->items[i]->enclosure, enclosure_full_filename);

        if (libcastget_urlget_buffer(f->items[i]->enclosure->url, enclosure_file, _enclosure_urlget_cb)) {
          g_fprintf(stderr, "Error downloading enclosure from %s.\n", f->items[i]->enclosure->url);
        } else {
          g_hash_table_insert(c->downloaded_enclosures, f->items[i]->enclosure->url, 
                              (gpointer)libcastget_get_rfc822_time());
        }

        fclose(enclosure_file);

        /* Save the channel file for each downloaded enclosure to make
           sure that it remains up to date. */
        _cast_channel_save(c);

        if (cb)
          cb(user_data, CCA_ENCLOSURE_DOWNLOAD_END, &(f->channel_info), f->items[i]->enclosure, enclosure_full_filename);

        g_free(enclosure_full_filename);
      }
    }

  /* Update the RSS last fetched time and save the channel file again. */
  if (c->rss_last_fetched)
    g_free(c->rss_last_fetched);

  c->rss_last_fetched = rss_last_fetched;

  _cast_channel_save(c);

  rss_close(f);

  return 0;
}

int libcastget_channel_catchup(libcastget_channel *c, void *user_data, libcastget_channel_callback cb)
{
  int i;
  rss_file *f;

  f = _get_rss(c, user_data, cb);

  if (!f)
    return 1;

  for (i = 0; i < f->num_items; i++)
    if (f->items[i]->enclosure) {
      if (!g_hash_table_lookup_extended(c->downloaded_enclosures, f->items[i]->enclosure->url, NULL, NULL)) {
        if (cb)
          cb(user_data, CCA_ENCLOSURE_DOWNLOAD_START, &(f->channel_info), f->items[i]->enclosure, NULL);
        
        g_hash_table_insert(c->downloaded_enclosures, f->items[i]->enclosure->url, 
                            (gpointer)libcastget_get_rfc822_time());

        if (cb)
          cb(user_data, CCA_ENCLOSURE_DOWNLOAD_END, &(f->channel_info), f->items[i]->enclosure, NULL);
      }
    }

  _cast_channel_save(c);

  rss_close(f);

  return 0;
}

int libcastget_channel_list(libcastget_channel *c, void *user_data, libcastget_channel_callback cb)
{
  int i;
  rss_file *f;

  f = _get_rss(c, user_data, cb);

  if (!f)
    return 1;

  for (i = 0; i < f->num_items; i++)
    if (f->items[i]->enclosure) {
      if (!g_hash_table_lookup_extended(c->downloaded_enclosures, f->items[i]->enclosure->url, NULL, NULL))
        if (cb)
          cb(user_data, CCA_ENCLOSURE_DOWNLOAD_START, &(f->channel_info), f->items[i]->enclosure, NULL);

        if (cb)
          cb(user_data, CCA_ENCLOSURE_DOWNLOAD_END, &(f->channel_info), f->items[i]->enclosure, NULL);
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
