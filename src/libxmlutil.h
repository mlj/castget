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

#ifndef LIBXMLUTIL_H
#define LIBXMLUTIL_H

#include <libxml/parser.h>
#include <libxml/tree.h>

char *libxmlutil_dup_attr(const xmlNode *node, const char *name);
const char *libxmlutil_attr_as_string(const xmlNode *node, const char *name);
long libxmlutil_attr_as_long(const xmlNode *node, const char *name);
int libxmlutil_attr_as_int(const xmlNode *node, const char *name);
char *libxmlutil_dup_value(const xmlNode *node);
int libxmlutil_count_by_tag_name(const xmlNode *node, const char *name);
void libxmlutil_iterate_by_tag_name(const xmlNode *node, const char *name, void *user_data,
                                    void(*f)(const void *user_data, int i, const xmlNode *node));
const xmlNode *libxmlutil_child_node_by_name(const xmlNode *node, const char *ns,
                                             const char *name);

#endif /* LIBXMLUTIL_H */
