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
 
  $Id: utils.c,v 1.2 2005/11/13 21:53:00 mariuslj Exp $
  
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "utils.h"

int libcastget_write_by_temporary_file(const gchar *filename, 
                                       int(*writer)(FILE *f, gpointer user_data), 
                                       gpointer user_data, gchar **used_filename)
{
  int retval;
  FILE *f;
  gint fd;
  gchar *tmp_filename_used;
  GError *error = NULL;

  fd = g_file_open_tmp(NULL, &tmp_filename_used, &error);
  
  if (fd < 0) {
    g_fprintf(stderr, "Error opening temporary file: %s\n", error->message);
    return -1;
  }

  f = fdopen(fd, "w");

  if (!f) {
    perror("Error opening temporary file stream");

    close(fd);
    g_free(tmp_filename_used);
    return -1;
  }

  retval = writer(f, user_data);

  fclose(f);
  close(fd);

  if (filename) {
    if (g_rename(tmp_filename_used, filename) < 0) {
      perror("Error renaming temporary file");
      
      unlink(tmp_filename_used);
      g_free(tmp_filename_used);
      return -1;
    }
  }

  if (used_filename) {
    if (filename)
      *used_filename = g_strdup(filename);
    else 
      *used_filename = g_strdup(tmp_filename_used);
  }

  g_free(tmp_filename_used);

  return retval;
}

/* 
   Local Variables:
   mode:c
   indent-tabs-mode:nil
   c-basic-offset:2
   coding:utf-8
   End:
*/
