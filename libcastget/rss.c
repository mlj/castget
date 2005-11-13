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
 
  $Id: rss.c,v 1.2 2005/11/13 21:53:00 mariuslj Exp $
  
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include "libcastget.h"
#include "libxmlutil.h"
#include "urlget.h"
#include "rss.h"

static char *_dup_child_node_value(const xmlNode *node, const gchar *tag)
{
  const xmlNode *n;
  
  n = libxmlutil_child_node_by_name(node, tag);

  if (n)
    return libxmlutil_dup_value(n);
  else
    return NULL;
}

static void _item_iterator(const void *user_data, int i, const xmlNode *node)
{
  rss_file *f = (rss_file *)user_data;
  const xmlNode *enclosure;

  /* Allocate item structure. */
  f->items[i] = (rss_item *)malloc(sizeof(struct _rss_item));

  /* Copy item meta-information. */
  f->items[i]->title = _dup_child_node_value(node, "title");
  f->items[i]->link = _dup_child_node_value(node, "link");
  f->items[i]->description = _dup_child_node_value(node, "description");

  /* Figure out if there is an enclosure. */
  enclosure = libxmlutil_child_node_by_name(node, "enclosure");

  if (enclosure) {
    f->items[i]->enclosure = (libcastget_enclosure *)malloc(sizeof(struct _libcastget_enclosure));
    f->items[i]->enclosure->url = libxmlutil_dup_attr(enclosure, "url");
    f->items[i]->enclosure->length = libxmlutil_attr_as_long(enclosure, "length");
    f->items[i]->enclosure->type = libxmlutil_dup_attr(enclosure, "type");
    f->items[i]->enclosure->description = libxmlutil_dup_attr(enclosure, "description");
  } else
    f->items[i]->enclosure = NULL;
}

static rss_file *rss_parse(const gchar *url, const xmlNode *root_element)
{
  const char *version_string;
  const xmlNode *channel;
  rss_file *f;
  enum rss_version version;

  /* Do some sanity checking and extract the RSS version number. */
  if (strcmp((char *)root_element->name, "rss")) {
    fprintf(stderr, "Error parsing RSS file %s: Unrecognized top-level element %s.\n", 
            url, (char *)root_element->name);
    return NULL;
  }

  version_string = libxmlutil_attr_as_string(root_element, "version");

  if (!strcmp(version_string, "2.0")) {
    version = RSS_VERSION_2_0;
  } else if (!strcmp(version_string, "0.91")) {
    version = RSS_VERSION_0_91;
  } else if (!strcmp(version_string, "0.92")) {
    version = RSS_VERSION_0_92;
  } else {
    version = RSS_UNKNOWN;
  }

  /* Find the channel tag and parse it. */
  channel = libxmlutil_child_node_by_name(root_element, "channel");

  if (channel) {
    /* Allocate RSS file structure and room for pointers to all entries. */
    f = (rss_file *)malloc(sizeof(struct _rss_file));
    f->num_items = libxmlutil_count_by_tag_name(channel, "item");
    f->items = (rss_item **)malloc(sizeof(rss_item *) * f->num_items);

    f->version = version;

    /* Copy channel meta-information. */
    f->channel_info.title = _dup_child_node_value(channel, "title");
    f->channel_info.link = _dup_child_node_value(channel, "link");
    f->channel_info.description = _dup_child_node_value(channel, "description");
    f->channel_info.language = _dup_child_node_value(channel, "language");

    libxmlutil_iterate_by_tag_name(channel, "item", f, _item_iterator);
  } else
    f = NULL;

  return f;
}

rss_file *rss_open_file(const char *filename)
{
  xmlDocPtr doc;
  rss_file *f;
  xmlNode *root_element = NULL;

  doc = xmlReadFile(filename, NULL, 0);

  if (!doc) {
    fprintf(stderr, "Error parsing RSS file %s.\n", filename);
    return NULL;
  }

  root_element = xmlDocGetRootElement(doc);

  if (!root_element)  {
    xmlFreeDoc(doc);

    fprintf(stderr, "Error parsing RSS file %s.\n", filename);
    return NULL;
  }

  f = rss_parse(filename, root_element);
 
  xmlFreeDoc(doc);

  return f;
}

static int _rss_open_url_cb(FILE *f, gpointer user_data)
{
  gchar *url = (gchar *)user_data;

  return libcastget_urlget_file(url, f);
}

rss_file *rss_open_url(const char *url)
{
  rss_file *f;
  gchar *rss_filename;

  if (libcastget_write_by_temporary_file(rss_filename, _rss_open_url_cb, url, &rss_filename))
    return NULL;

  f = rss_open_file(rss_filename);

  unlink(rss_filename);
  g_free(rss_filename);

  return f;
}

void rss_close(rss_file *f)
{
  int i;
  rss_item *item;
  
  for (i = 0; i < f->num_items; i++) {
    item = f->items[i];

    if (item->enclosure) {
      if (item->enclosure->url)
        free(item->enclosure->url);
      
      if (item->enclosure->type)
        free(item->enclosure->type);

      if (item->enclosure->description)
        free(item->enclosure->description);

      free(item->enclosure);
    }

    if (item->title)
      free(item->title);

    free(item);
  }

  if (f->channel_info.title)
    free(f->channel_info.title);

  free(f);
}

long rss_total_enclosure_size(rss_file *f)
{
  int i;
  long n = 0;

  for (i = 0; i < f->num_items; i++)
    if (f->items[i]->enclosure)
      n += f->items[i]->enclosure->length;
    
  return n;
}

int libcastget_enclosure_count(rss_file *f)
{
  int i;
  int n = 0;

  for (i = 0; i < f->num_items; i++)
    if (f->items[i]->enclosure)
      n++;
    
  return n;
}

/* 
   Local Variables:
   mode:c
   indent-tabs-mode:nil
   c-basic-offset:2
   coding:utf-8
   End:
*/
