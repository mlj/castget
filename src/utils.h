/*
  Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 Marius L. Jøhndal

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

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <glib.h>

int write_by_temporary_file(const gchar *filename,
                            int(*writer)(FILE *f, gpointer user_data, int debug),
                            gpointer user_data, gchar **used_filename,
                            int debug);
gchar *get_rfc822_time(void);

#endif /* UTILS_H */
