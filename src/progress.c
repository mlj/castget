/*
  Copyright (C) 2013-2016 Marius L. Jøhndal

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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <glib.h>
#include "progress.h"

/* This code was inspired/guided by the progress-bar implementation found in
 * curl (src/tool_cb_prg.c) by Daniel Stenberg, in turn building on an
 * implementation by Lars Aas. */

int progress_bar_cb(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
  double fraction;
  int num;
  int i;

  progress_bar *pb = (progress_bar *)clientp;

  /* We expect ultotal and ulnow to be 0 since we never upload anything. */
  g_assert(ultotal == 0);
  g_assert(ulnow == 0);

  if (pb->width > 0) {
    if (dltotal == 0.0) {
      fraction = 0.0;
      num = 0;
    } else {
      long total;
      long position;

      total = (long)dltotal + pb->resume_from;
      position = MIN((long)dlnow + pb->resume_from, total);

      fraction = (double)position/(double)total;
      num = (int)((double)pb->width * fraction);
    }

    if (num != pb->previous_num) {
      for (i = 0; i < pb->width; i++)
        pb->buffer[i] = (i < num ? '#' : ' ');

      pb->buffer[pb->width] = 0;

      fprintf(pb->f, "\r%s %3d%%", pb->buffer, (int)(fraction * 100.0));
      fflush(pb->f);

      pb->previous_num = num;
    }
  }

  return 0;
}

progress_bar *progress_bar_new(long resume_from)
{
  progress_bar *pb;
  gchar **environ;
  const gchar *columns;

  pb = (progress_bar *)g_malloc(sizeof(struct _progress_bar));
  pb->resume_from = resume_from;
  pb->f = stdout;
  pb->width = 79;
  pb->previous_num = -1;

  /* Try to grab the COLUMNS environment variable to initialize pb->with with a suitable value. */
  environ = g_get_environ();
  columns = g_environ_getenv(environ, "COLUMNS");

  if (columns) {
    char *endptr;
    long num = strtol(columns, &endptr, 10);
    if ((endptr != columns) && (endptr == columns + strlen(columns)) && (num > 0)) {
      pb->width = MIN(pb->width, (int)num); /* restrict width of progress bar to avoid insane values */
    }
  }

  g_strfreev(environ);

  /* Leave a margin for printing the percentages (the longest string is " 100%") */
  pb->width = MAX(0, pb->width - 5);

  /* Allocate space for progress bar string + terminating zero. */
  pb->buffer = g_malloc(pb->width + 1);

  return pb;
}

void progress_bar_free(progress_bar *pb)
{
  fprintf(pb->f, "\n"); /* Make sure that we will start output on a new line. */
  g_free(pb->buffer);
  g_free(pb);
}
