/*
  Copyright (C) 2005, 2006, 2007 Marius L. Jøhndal
 
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
 
  $Id: libcastget.h,v 1.1 2007/09/20 17:49:23 mariuslj Exp $
  
*/

#ifndef LIBCASTGET_H
#define LIBCASTGET_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum {
  CCA_RSS_DOWNLOAD_START,
  CCA_RSS_DOWNLOAD_END,
  CCA_ENCLOSURE_DOWNLOAD_START,
  CCA_ENCLOSURE_DOWNLOAD_END
} libcastget_channel_action;

typedef struct _libcastget_channel libcastget_channel;

typedef struct _libcastget_channel_info {
  char *title;
  char *link;
  char *description;
  char *language;
} libcastget_channel_info;

typedef struct _libcastget_enclosure {
  char *url;
  long length;
  char *type;
  char *filename;
} libcastget_enclosure;

typedef void (*libcastget_channel_callback)(void *user_data, 
                                         libcastget_channel_action action, 
                                         libcastget_channel_info *channel_info, 
                                         libcastget_enclosure *enclosure,
                                         const char *filename);

libcastget_channel *libcastget_channel_new(const char *url, const char *channel_file, 
                                           const char *spool_directory, int resume);
void libcastget_channel_free(libcastget_channel *c);
int libcastget_channel_update(libcastget_channel *c, void *user_data, libcastget_channel_callback cb,
                              int no_download, int no_mark_read, int first_only, int resume);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBCASTGET_H */

/* 
   Local Variables:
   mode:c
   indent-tabs-mode:nil
   c-basic-offset:2
   coding:utf-8
   End:
*/
