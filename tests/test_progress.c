#include <glib.h>
#include <stdlib.h>
#include <stdio.h>

#include "../src/progress.h"

static void test_progress_bar_new()
{
  char *old_columns = getenv("COLUMNS");
  progress_bar *pb;

  setenv("COLUMNS", "120", 1);
  pb = progress_bar_new(0);
  g_assert_cmpint(pb->width, ==, 74);
  progress_bar_free(pb);

  setenv("COLUMNS", "50", 1);
  pb = progress_bar_new(0);
  g_assert_cmpint(pb->width, ==, 45);
  progress_bar_free(pb);

  setenv("COLUMNS", "abc", 1);
  pb = progress_bar_new(0);
  g_assert_cmpint(pb->width, ==, 74);
  progress_bar_free(pb);

  if (old_columns) {
    setenv("COLUMNS", old_columns, 1);
  }
}

static void test_progress_bar_cb()
{
  char *old_columns = getenv("COLUMNS");
  progress_bar *pb;

  setenv("COLUMNS", "78", 1);
  pb = progress_bar_new(0);
  /* Silence progress bar output by directing it to a temporary file */
  pb->f = tmpfile();

  progress_bar_cb(pb, 300, 0, 0, 0);
  g_assert_cmpstr(pb->buffer, ==, "                                                                         ");
  progress_bar_cb(pb, 300, 150, 0, 0);
  g_assert_cmpstr(pb->buffer, ==, "####################################                                     ");
  progress_bar_cb(pb, 300, 450, 0, 0);
  g_assert_cmpstr(pb->buffer, ==, "#########################################################################");
  progress_bar_cb(pb, 300, -1, 0, 0);
  g_assert_cmpstr(pb->buffer, ==, "                                                                         ");
  pb->resume_from = 150;
  progress_bar_cb(pb, 150, 0, 0, 0);
  g_assert_cmpstr(pb->buffer, ==, "####################################                                     ");

  fclose(pb->f);
  progress_bar_free(pb);

  if (old_columns) {
    setenv("COLUMNS", old_columns, 1);
  }
}

int main (int argc, char *argv[])
{
  g_test_init(&argc, &argv, NULL);

  g_test_add_func("/progress/progress_bar_new", test_progress_bar_new);
  g_test_add_func("/progress/progress_bar_cb", test_progress_bar_cb);

  return g_test_run();
}
