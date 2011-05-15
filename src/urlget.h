/*
  Copyright (C) 2005, 2007, 2011 Marius L. Jøhndal

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

#ifndef URLGET_H
#define URLGET_H

int urlget_file(const char *url, FILE *f);
int urlget_buffer(const char *url, void *user_data,
                  size_t (*write_buffer)(void *buffer, size_t size, size_t nmemb, void *user_data),
                  long resume_from);

#endif /* URLGET_H */
