#include <glib.h>
#include <stdlib.h>
#include <stdio.h>

#include "mocks.h"

static void pattern_helper(const char *pattern, char *channel_title, char *item_title,
  char *item_pub_date, const char *expected_string)
{
  channel_info *mock_channel_info = mock_channel_info_new(channel_title);
  rss_item *mock_item = mock_rss_item_new(item_title, item_pub_date);

  gchar *string = expand_string_with_patterns(pattern, mock_channel_info, mock_item);

  g_assert_cmpstr(string, ==, expected_string);

  g_free(string);
  mock_rss_item_free(mock_item);
  mock_channel_info_free(mock_channel_info);
}

static void test_expand_string_with_empty_pattern()
{
  pattern_helper("", NULL, NULL, NULL, "");
}

static void test_expand_string_with_static_pattern()
{
  pattern_helper("static_name", NULL, NULL, NULL, "static_name");
}

static void test_expand_string_with_date_pattern()
{
  pattern_helper("%(date)", NULL, NULL, "Thu, 01 Oct 2015 09:53:38 GMT", "2015-10-01");
  pattern_helper("foo %(date) bar", NULL, NULL, "Thu, 01 Oct 2015 09:53:38 GMT", "foo 2015-10-01 bar");
}

static void test_expand_string_with_item_title_pattern()
{
  pattern_helper("%(title)", "Channel Title", "Item Title", NULL, "Item Title");
  pattern_helper("foo %(title) bar", "Channel Title", "Item Title", NULL, "foo Item Title bar");
}

static void test_expand_string_with_channel_title_pattern()
{
  pattern_helper("%(channel_title)", "Channel Title", "Item Title", NULL, "Channel Title");
  pattern_helper("foo %(channel_title) bar", "Channel Title", "Item Title", NULL, "foo Channel Title bar");
}

static void test_expand_string_with_pattern_with_slashes()
{
  pattern_helper("%(title)", "Channel Title", "Test/Title", NULL, "Test/Title");
  pattern_helper("foo/%(title)/bar", "Channel Title", "Test Title", NULL, "foo/Test Title/bar");
  pattern_helper("invalid/name", "Channel Title", NULL, NULL, "invalid/name");
}

int main (int argc, char *argv[])
{
  g_test_init(&argc, &argv, NULL);

  g_test_add_func("/patterns/expand_string_with_empty_pattern", test_expand_string_with_empty_pattern);
  g_test_add_func("/patterns/expand_string_with_static_pattern", test_expand_string_with_static_pattern);
  g_test_add_func("/patterns/expand_string_with_date_pattern", test_expand_string_with_date_pattern);
  g_test_add_func("/patterns/expand_string_with_item_title_pattern", test_expand_string_with_item_title_pattern);
  g_test_add_func("/patterns/expand_string_with_channel_title_pattern", test_expand_string_with_channel_title_pattern);
  g_test_add_func("/patterns/expand_string_with_pattern_with_slashes", test_expand_string_with_pattern_with_slashes);

  return g_test_run();
}
