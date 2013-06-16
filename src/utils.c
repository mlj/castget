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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#include "utils.h"

int write_by_temporary_file(const gchar *filename,
                            int(*writer)(FILE *f, gpointer user_data, int debug),
                            gpointer user_data, gchar **used_filename, int debug)
{
  int retval;
  FILE *f;
  gint fd;
  gchar *tmp_filename_used;

  if (filename) {
    tmp_filename_used = g_strconcat(filename, ".XXXXXX", NULL);

    fd = g_mkstemp(tmp_filename_used);

    if (fd < 0) {
      perror("Error opening temporary file");
      g_free(tmp_filename_used);
      return -1;
    }
  } else {
    GError *error = NULL;

    fd = g_file_open_tmp(NULL, &tmp_filename_used, &error);

    if (fd < 0) {
      g_fprintf(stderr, "Error opening temporary file: %s\n", error->message);
      return -1;
    }
  }

  f = fdopen(fd, "w");

  if (!f) {
    perror("Error opening temporary file stream");

    close(fd);
    g_free(tmp_filename_used);
    return -1;
  }

  retval = writer(f, user_data, debug);

  fclose(f);

  if (errno == ENOSPC) {
    fprintf(stderr, "No space left on device.\n");
    unlink(tmp_filename_used);
    g_free(tmp_filename_used);
    return -1;
  }

  if (retval == 0 && filename) {
    if (g_rename(tmp_filename_used, filename) < 0) {
      fprintf(stderr, "Error renaming temporary file %s to %s: %s.\n",
              tmp_filename_used, filename, strerror(errno));

      unlink(tmp_filename_used);
      g_free(tmp_filename_used);
      return -1;
    }

    if (used_filename)
      *used_filename = g_strdup(filename);
  } else {
    if (used_filename)
      *used_filename = g_strdup(tmp_filename_used);
  }

  g_free(tmp_filename_used);

  return retval;
}

#define RFC822_TIME_BUFFER_LEN 64

gchar *get_rfc822_time(void)
{
  char rfc822_time_buffer[RFC822_TIME_BUFFER_LEN];
  time_t now;

  now = time(NULL);

  if (strftime(rfc822_time_buffer, RFC822_TIME_BUFFER_LEN, "%a, %d-%b-%Y %X GMT", gmtime(&now)))
    return g_strdup(rfc822_time_buffer);
  else
    return NULL;
}
