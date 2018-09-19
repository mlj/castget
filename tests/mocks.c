#include <glib.h>
//#include <stdlib.h>
//#include <stdio.h>
#include "mocks.h"

rss_item *mock_rss_item_new(char *item_title, char *item_pub_date)
{
  rss_item *mock_item;

  mock_item = g_malloc(sizeof(struct _rss_item));
  mock_item->title = item_title;
  mock_item->link = NULL;
  mock_item->description = NULL;
  mock_item->pub_date = item_pub_date;
  mock_item->enclosure = NULL;

  return mock_item;
}

void mock_rss_item_free(rss_item *item)
{
  g_free(item);
}

channel_info *mock_channel_info_new(char *title)
{
  channel_info *mock_channel_info;

  mock_channel_info = g_malloc(sizeof(struct _channel_info));
  mock_channel_info->title = title;
  mock_channel_info->link = NULL;
  mock_channel_info->description = NULL;
  mock_channel_info->language = NULL;

  return mock_channel_info;
}

void mock_channel_info_free(channel_info *info)
{
  g_free(info);
}
