/*
  Copyright (C) 2005, 2006 Marius L. Jøhndal
 
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
 
  $Id: rss.c,v 1.7 2006/06/09 21:58:44 mariuslj Exp $
  
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
#include "htmlent.h"
#include "rss.h"

#define MRSS_NAMESPACE "http://search.yahoo.com/mrss"

static char *_dup_child_node_value(const xmlNode *node, const gchar *tag)
{
  const xmlNode *n;
  
  n = libxmlutil_child_node_by_name(node, NULL, tag);

  if (n)
    return libxmlutil_dup_value(n);
  else
    return NULL;
}

static void _item_iterator(const void *user_data, int i, const xmlNode *node)
{
  rss_file *f = (rss_file *)user_data;
  const xmlNode *enclosure;
  const xmlNode *mrss_content;
  const xmlNode *mrss_group;

  /* Allocate item structure. */
  f->items[i] = (rss_item *)malloc(sizeof(struct _rss_item));

  /* Copy item meta-information. */
  f->items[i]->title = _dup_child_node_value(node, "title");
  f->items[i]->link = _dup_child_node_value(node, "link");
  f->items[i]->description = _dup_child_node_value(node, "description");

  /* Look for mrss information first, if there is any. It may be
     located either directly under the "item" tag, or inside an mrss
     "group" tag. */
  mrss_content = libxmlutil_child_node_by_name(node, MRSS_NAMESPACE, 
                                               "content");

  if (!mrss_content) {
    mrss_group = libxmlutil_child_node_by_name(node, MRSS_NAMESPACE, 
                                               "group");

    if (mrss_group)
      mrss_content = libxmlutil_child_node_by_name(mrss_group, 
                                                   MRSS_NAMESPACE, "content");
  }

  /* Figure out if there is an "enclosure" tag here. */
  enclosure = libxmlutil_child_node_by_name(node, NULL, "enclosure");

  if (mrss_content || enclosure) {
    f->items[i]->enclosure = (libcastget_enclosure *)malloc(sizeof(struct _libcastget_enclosure));
    f->items[i]->enclosure->url = NULL;
    f->items[i]->enclosure->length = 0;
    f->items[i]->enclosure->type = NULL;

    /* Now read attributes. Prefer mrss over enclosure. */
    if (mrss_content) {
      f->items[i]->enclosure->url = libxmlutil_dup_attr(mrss_content, "url");
      f->items[i]->enclosure->length = libxmlutil_attr_as_long(mrss_content, "fileSize");
      f->items[i]->enclosure->type = libxmlutil_dup_attr(enclosure, "type");
    }

    if (enclosure) {
      if (!f->items[i]->enclosure->url)
        f->items[i]->enclosure->url = libxmlutil_dup_attr(enclosure, "url");

      if (!f->items[i]->enclosure->length)
        f->items[i]->enclosure->length = libxmlutil_attr_as_long(enclosure, "length");

      if (!f->items[i]->enclosure->type)
        f->items[i]->enclosure->type = libxmlutil_dup_attr(enclosure, "type");
    }
  } else
    f->items[i]->enclosure = NULL;

  /* Determine filename of enclosure. */
  if (f->items[i]->enclosure && f->items[i]->enclosure->url)
    f->items[i]->enclosure->filename = g_path_get_basename(f->items[i]->enclosure->url);
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
  channel = libxmlutil_child_node_by_name(root_element, NULL, "channel");

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

static xmlEntityPtr _get_entity(void *ctxt, const xmlChar *name)
{
  static GHashTable *html_entities = NULL;
  xmlEntityPtr entity;
  gchar *contents;

  /* Check if entity is any of the predefined entities such as &amp; */
  entity = xmlGetPredefinedEntity(name);

  if (!entity) {
    /* Some of the RSS "specifications" are vague on whether HTML
       entities are allowed or not, so we will assume that they are,
       and look up HTML entities whenever we encounter them. */

    if (!html_entities)
      html_entities = htmlent_hash_new();

    contents = (gchar *)g_hash_table_lookup(html_entities, name);

    if (contents) {
      /* TODO: Where do we free this memory? */
      entity = (xmlEntityPtr)g_new0(xmlEntity, 1);
      entity->type = XML_ENTITY_DECL;
      entity->name = name;
      entity->orig = contents;
      entity->content = contents;
      entity->length = g_utf8_strlen(contents, -1);
      entity->etype = XML_INTERNAL_PREDEFINED_ENTITY;
    }
  }

  return entity;
}

rss_file *rss_open_file(const char *filename)
{
  xmlParserCtxtPtr ctxt;
  xmlDocPtr doc;
  rss_file *f;
  xmlNode *root_element = NULL;

  ctxt = xmlNewParserCtxt();
  ctxt->sax->getEntity = _get_entity;
  doc = xmlSAXParseFile(ctxt->sax, filename, 0);

  if (!doc) {
    fprintf(stderr, "Error parsing RSS file %s.\n", filename);
    xmlFreeParserCtxt(ctxt);

    return NULL;
  }

  root_element = xmlDocGetRootElement(doc);

  if (!root_element)  {
    xmlFreeDoc(doc);
    xmlFreeParserCtxt(ctxt);

    fprintf(stderr, "Error parsing RSS file %s.\n", filename);
    return NULL;
  }

  f = rss_parse(filename, root_element);
 
  xmlFreeDoc(doc);
  xmlFreeParserCtxt(ctxt);

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

  if (libcastget_write_by_temporary_file(NULL, _rss_open_url_cb, url, &rss_filename))
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

      if (item->enclosure->filename)
        free(item->enclosure->filename);

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
