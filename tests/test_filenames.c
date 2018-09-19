#include <glib.h>
#include <stdlib.h>
#include <stdio.h>

#include "mocks.h"
#include "../src/filenames.h"

static void filename_helper(const char *pattern, char *channel_title, char *item_title,
  char *item_pub_date, const char *expected_filename)
{
  channel_info *mock_channel_info = mock_channel_info_new(channel_title);
  rss_item *mock_item = mock_rss_item_new(item_title, item_pub_date);
  gchar *filename = build_enclosure_filename("/spool", pattern, mock_channel_info, mock_item);

  g_assert_cmpstr(filename, ==, expected_filename);

  g_free(filename);
  mock_rss_item_free(mock_item);
  mock_channel_info_free(mock_channel_info);
}

static void test_build_enclosure_filename()
{
  filename_helper("", NULL, NULL, NULL, "/spool");
  filename_helper("static_name.mp3", NULL, NULL, NULL, "/spool/static_name.mp3");

  filename_helper("%(title).mp3", NULL, "Test Title", NULL, "/spool/Test Title.mp3");
  filename_helper("foo %(title) bar.mp3", NULL, "Test Title", NULL, "/spool/foo Test Title bar.mp3");

  filename_helper("%(date).mp3", NULL, NULL, "Thu, 01 Oct 2015 09:53:38 GMT", "/spool/2015-10-01.mp3");
  filename_helper("foo %(date) bar.mp3", NULL, NULL, "Thu, 01 Oct 2015 09:53:38 GMT", "/spool/foo 2015-10-01 bar.mp3");

  /* Suspicious values */
  filename_helper("%(title).mp3", NULL, "Test/Title", NULL, "/spool/TestTitle.mp3");
  filename_helper("foo/%(title)/bar.mp3", NULL, "Test Title", NULL, "/spool/fooTest Titlebar.mp3");
  filename_helper("invalid/name.mp3", NULL, NULL, NULL, "/spool/invalidname.mp3");
}

int main (int argc, char *argv[])
{
  g_test_init(&argc, &argv, NULL);

  g_test_add_func("/filename_patterns/build_enclosure_filename", test_build_enclosure_filename);

  return g_test_run();
}
