#include <glib.h>
#include <stdlib.h>
#include <stdio.h>

#include "../src/filename_patterns.h"

static void filename_helper(const char *pattern, char *item_title,
  char *item_pub_date, const char *expected_filename)
{
  rss_item *mock_item;
  gchar *filename;

  mock_item = g_malloc(sizeof(struct _rss_item));
  mock_item->title = item_title;
  mock_item->link = NULL;
  mock_item->description = NULL;
  mock_item->pub_date = item_pub_date;
  mock_item->enclosure = NULL;

  filename = build_enclosure_filename("/spool", pattern, mock_item);

  g_assert_cmpstr(filename, ==, expected_filename);

  g_free(filename);
  g_free(mock_item);
}

static void test_build_enclosure_filename()
{
  filename_helper("", NULL, NULL, "/spool");

  filename_helper("static_name.mp3", NULL, NULL, "/spool/static_name.mp3");

  filename_helper("%(title).mp3", "Test Title", NULL, "/spool/Test Title.mp3");
  filename_helper("%(title).mp3", "Test/Title", NULL, "/spool/TestTitle.mp3");
  filename_helper("foo %(title) bar.mp3", "Test Title", NULL, "/spool/foo Test Title bar.mp3");

  filename_helper("%(date).mp3", NULL, "Thu, 01 Oct 2015 09:53:38 GMT", "/spool/01-10-15.mp3");
  filename_helper("foo %(date) bar.mp3", NULL, "Thu, 01 Oct 2015 09:53:38 GMT", "/spool/foo 01-10-15 bar.mp3");
}

int main (int argc, char *argv[])
{
  g_test_init(&argc, &argv, NULL);

  g_test_add_func("/filename_patterns/build_enclosure_filename", test_build_enclosure_filename);

  return g_test_run();
}
