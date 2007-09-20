/*
  Copyright (C) 2005, 2007 Marius L. Jøhndal
 
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
 
  $Id: channel.h,v 1.2 2007/09/20 18:10:51 mariuslj Exp $
  
*/

#ifndef CHANNEL_H
#define CHANNEL_H

typedef enum {
  CCA_RSS_DOWNLOAD_START,
  CCA_RSS_DOWNLOAD_END,
  CCA_ENCLOSURE_DOWNLOAD_START,
  CCA_ENCLOSURE_DOWNLOAD_END
} channel_action;

typedef struct _channel {
  gchar *url;
  gchar *channel_filename;
  gchar *spool_directory;
  GHashTable *downloaded_enclosures;
  gchar *rss_last_fetched;
} channel;

typedef struct _channel_info {
  char *title;
  char *link;
  char *description;
  char *language;
} channel_info;

typedef struct _enclosure {
  char *url;
  long length;
  char *type;
  char *filename;
} enclosure;

typedef void (*channel_callback)(void *user_data, 
                                 channel_action action, 
                                 channel_info *channel_info, 
                                 enclosure *enclosure,
                                 const char *filename);

channel *channel_new(const char *url, const char *channel_file, 
                     const char *spool_directory, int resume);
void channel_free(channel *c);
int channel_update(channel *c, void *user_data, channel_callback cb, int no_download, 
                   int no_mark_read, int first_only, int resume);


#endif /* CHANNEL_H */

/* 
   Local Variables:
   mode:c
   indent-tabs-mode:nil
   c-basic-offset:2
   coding:utf-8
   End:
*/

