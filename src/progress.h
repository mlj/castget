/*
  Copyright (C) 2005-2016 Marius L. Jøhndal

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

#ifndef PROGRESS_H
#define PROGRESS_H

typedef struct _progress_bar {
  FILE *f;
  long resume_from;
  int width;
  int previous_num;
  char *buffer;
} progress_bar;

progress_bar *progress_bar_new(long resume_from);
void progress_bar_free(progress_bar *pb);
int progress_bar_cb(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);

#endif /* PROGRESS_H */
