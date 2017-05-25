#include <glib.h>
#include <stdlib.h>
#include <stdio.h>

#include "../src/patterns.h"

static rss_item *mock_rss_item(char *item_title, char *item_pub_date)
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

static void pattern_helper(const char *pattern, char *item_title,
  char *item_pub_date, const char *expected_string)
{
  rss_item *mock_item = mock_rss_item(item_title, item_pub_date);
  gchar *string = expand_string_with_patterns(pattern, mock_item);

  g_assert_cmpstr(string, ==, expected_string);

  g_free(string);
  g_free(mock_item);
}

static void test_expand_string_with_patterns()
{
  pattern_helper("", NULL, NULL, "");
  pattern_helper("static_name", NULL, NULL, "static_name");

  pattern_helper("%(title)", "Test Title", NULL, "Test Title");
  pattern_helper("foo %(title) bar", "Test Title", NULL, "foo Test Title bar");

  pattern_helper("%(date)", NULL, "Thu, 01 Oct 2015 09:53:38 GMT", "2015-10-01");
  pattern_helper("foo %(date) bar", NULL, "Thu, 01 Oct 2015 09:53:38 GMT", "foo 2015-10-01 bar");

  pattern_helper("%(title)", "Test/Title", NULL, "Test/Title");
  pattern_helper("foo/%(title)/bar", "Test Title", NULL, "foo/Test Title/bar");
  pattern_helper("invalid/name", NULL, NULL, "invalid/name");
}

int main (int argc, char *argv[])
{
  g_test_init(&argc, &argv, NULL);

  g_test_add_func("/patterns/expand_string_with_patterns", test_expand_string_with_patterns);

  return g_test_run();
}
