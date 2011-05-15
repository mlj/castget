/*
  Copyright (C) 2005, 2006, 2011 Marius L. Jøhndal

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
#include "libxmlutil.h"

char *libxmlutil_dup_attr(const xmlNode *node, const char *name)
{
  xmlChar *s;

  s = xmlGetProp((xmlNode *)node, (const xmlChar *)name);

  if (s)
    return strdup((char *)s);
  else
    return NULL;
}

const char *libxmlutil_attr_as_string(const xmlNode *node, const char *name)
{
  return (const char *)xmlGetProp((xmlNode *)node, (const xmlChar *)name);
}

long libxmlutil_attr_as_long(const xmlNode *node, const char *name)
{
  xmlChar *s;

  s = xmlGetProp((xmlNode *)node, (const xmlChar *)name);

  if (s)
    return strtol((char *)s, (char **)NULL, 10);
  else
    return -1;
}

int libxmlutil_attr_as_int(const xmlNode *node, const char *name)
{
  return (int)libxmlutil_attr_as_long(node, name);
}

char *libxmlutil_dup_value(const xmlNode *node)
{
  xmlChar *s;

  s = xmlNodeListGetString(node->doc, node->xmlChildrenNode, 1);

  if (s)
    return strdup((char *)s);
  else
    return NULL;
}

int libxmlutil_count_by_tag_name(const xmlNode *node, const char *name)
{
  int n = 0;

  for (node = node->children; node; node = node->next)
    if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, name))
      n++;

  return n;
}

void libxmlutil_iterate_by_tag_name(const xmlNode *node, const char *name, void *user_data,
                                     void(*f)(const void *user_data, int i, const xmlNode *node))
{
  int i = 0;

  for (node = node->children; node; node = node->next)
    if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, name)) {
      f(user_data, i, node);
      i++;
    }
}

const xmlNode *libxmlutil_child_node_by_name(const xmlNode *node, const char *ns,
                                             const char *name)
{
  for (node = node->children; node; node = node->next)
    if (node->type == XML_ELEMENT_NODE && !strcmp((char *)node->name, name)
        && (!ns || !strcmp((char *)node->ns->href, ns)))
      return node;

  return NULL;
}
