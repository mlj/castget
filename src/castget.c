/*
  Copyright (C) 2005-2021 Marius L. Jøhndal

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "channel.h"
#include "configuration.h"

#define _GNU_SOURCE
#include <errno.h>
#include <getopt.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#include <libxml/parser.h>
#include <string.h>
#include <unistd.h>
#ifdef HAVE_TAGLIB
#include <taglib/tag_c.h>
#endif /* HAVE_TAGLIB */

enum op { OP_UPDATE, OP_CATCHUP, OP_LIST };

static int _process_channel(const gchar *channel_directory, GKeyFile *kf,
                            const char *identifier, enum op op,
                            struct channel_configuration *defaults,
                            enclosure_filter *filter);
static void version(void);
static GKeyFile *_configuration_file_open(const gchar *rcfile);
static void _configuration_file_close(GKeyFile *kf);
#ifdef HAVE_TAGLIB
static void _set_tags(const gchar *filename,
                      const struct channel_configuration *cfg);
#endif /* HAVE_TAGLIB */
static int playlist_add(const gchar *playlist_file, const gchar *media_file);

static gboolean verbose = FALSE;
static gboolean quiet = FALSE;
static gboolean new_only = FALSE;
static option_info *opts = NULL;

int main(int argc, char **argv)
{
  enum op op = OP_UPDATE;
  int i;
  int ret = 0;
  gchar **groups;
  gchar *channeldir;
  GKeyFile *kf;
  struct channel_configuration *defaults;
  enclosure_filter *filter = NULL;
  GError *error = NULL;
  GOptionContext *context;
  static gboolean first_only = FALSE;
  static gboolean resume = FALSE;
  static gboolean debug = FALSE;
  static gboolean reverse = FALSE;
  static gboolean show_progress_bar = FALSE;
  static gchar *rcfile = NULL;
  static gchar *filter_regex = NULL;
  static gboolean list = FALSE;
  static gboolean catchup = FALSE;
  static gboolean show_version = FALSE;

  static GOptionEntry options[] = {
    { "catchup", 'c', 0, G_OPTION_ARG_NONE, &catchup,
      "catch up with channels and exit" },
    { "list", 'l', 0, G_OPTION_ARG_NONE, &list,
      "list available enclosures that have not yet been downloaded and exit" },
    { "version", 'V', 0, G_OPTION_ARG_NONE, &show_version,
      "print version and exit" },

    { "resume", 'r', 0, G_OPTION_ARG_NONE, &resume,
      "resume aborted downloads" },
    { "rcfile", 'C', 0, G_OPTION_ARG_FILENAME, &rcfile,
      "override the default configuration file name" },

    { "debug", 'd', 0, G_OPTION_ARG_NONE, &debug,
      "print connection debug information" },
    { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose,
      "print detailed progress information" },
    { "progress-bar", 'p', 0, G_OPTION_ARG_NONE, &show_progress_bar,
      "print progress bar" },

    { "new-only", 'n', 0, G_OPTION_ARG_NONE, &new_only,
      "only process new channels" },
    { "quiet", 'q', 0, G_OPTION_ARG_NONE, &quiet, "only print error messages" },
    { "first-only", '1', 0, G_OPTION_ARG_NONE, &first_only,
      "only process the most recent item from each channel" },
    { "reverse", 'R', 0, G_OPTION_ARG_NONE, &reverse,
      "process the channel in reverse order" },
    { "filter", 'f', 0, G_OPTION_ARG_STRING, &filter_regex,
      "only process items whose enclosure names match a regular expression" },

    { NULL }
  };

  context = g_option_context_new("CHANNELS");
  g_option_context_add_main_entries(context, options, NULL);

  if (!g_option_context_parse(context, &argc, &argv, &error)) {
    g_print("option parsing failed: %s\n", error->message);
    exit(1);
  }

  /* Do some additional sanity checking of options. */
  if ((verbose && quiet) || (show_progress_bar && quiet)) {
    g_print("option parsing failed: options are incompatible.\n");
    exit(1);
  }

  if ((catchup && list) || (catchup && show_version) ||
      (list && show_version)) {
    g_print(
        "option parsing failed: --catchup, --list and --version options are "
        "incompatible.\n");
    exit(1);
  }

  /* Decide on the action to take */
  if (show_version) {
    version();
    exit(0);
  }

  opts = option_info_new();
  opts->no_download = 0;
  opts->no_mark_read = 0;
  opts->first_only = first_only;
  opts->resume = resume;
  opts->debug = debug;
  opts->reverse = reverse;
  opts->show_progress_bar = show_progress_bar;

  if (catchup) {
    op = OP_CATCHUP;
    opts->no_download = 1;
    opts->no_mark_read = 0;
  }

  if (list) {
    op = OP_LIST;
    opts->no_download = 1;
    opts->no_mark_read = 1;
  }

  opts->filter = NULL;
  if (filter_regex) {
    filter = enclosure_filter_new(filter_regex, FALSE);
    opts->filter = filter;
    g_free(filter_regex);
  }

  if (verbose && new_only)
    g_print("Fetching new channels only...\n");

  LIBXML_TEST_VERSION;

  /* Build the channel directory path and ensure that it exists. */
  channeldir = g_build_filename(g_get_home_dir(), ".castget", NULL);

  if (!g_file_test(channeldir, G_FILE_TEST_IS_DIR)) {
    if (g_mkdir(channeldir, 0755) < 0) {
      perror("Error creating channel directory");
      return 1;
    }
  }

  /* Try opening configuration file. */
  if (!rcfile)
    /* Supply default path name. */
    rcfile = g_build_filename(g_get_home_dir(), ".castgetrc", NULL);

  kf = _configuration_file_open(rcfile);

  if (kf) {
    /* Read defaults. */
    if (g_key_file_has_group(kf, "*")) {
      /* Verify the keys in the global configuration. */
      if (channel_configuration_verify_keys(kf, "*") < 0)
        return -1;

      defaults = channel_configuration_new(kf, "*", NULL);
    } else
      defaults = NULL;

    /* Perform actions. */
    if (optind < argc) {
      while (optind < argc)
        _process_channel(channeldir, kf, argv[optind++], op, defaults, filter);
    } else {
      groups = g_key_file_get_groups(kf, NULL);

      for (i = 0; groups[i]; i++)
        if (strcmp(groups[i], "*"))
          _process_channel(channeldir, kf, groups[i], op, defaults, filter);

      g_strfreev(groups);
    }

    /* Clean up defaults. */
    if (defaults)
      channel_configuration_free(defaults);
  } else
    ret = 1;

  /* Clean-up. */
  g_free(channeldir);

  if (filter)
    enclosure_filter_free(filter);

  option_info_free(opts);

  g_free(rcfile);

  if (kf)
    _configuration_file_close(kf);

  xmlCleanupParser();

  return ret;
}

static void version(void)
{
  g_printf("%s %s", PACKAGE, VERSION);
#ifdef HAVE_TAGLIB
  g_printf(" with taglib support\n");
#else
  g_printf("\n");
#endif

  g_printf("Copyright (C) 2005-2021 Marius L. Jøhndal\n");
}

static void _print_item_update(const enclosure *enclosure,
                               const gchar *filename)
{
  if (enclosure->length > 0) {
    gchar *size = g_format_size(enclosure->length);
    g_printf(" * %s (%s)\n", enclosure->url, size);
    g_free(size);
  } else
    g_printf(" * %s (unknown size)\n", filename);
}

static void update_callback(void *user_data, channel_action action,
                            channel_info *channel_info, enclosure *enclosure,
                            const gchar *filename)
{
  struct channel_configuration *c = (struct channel_configuration *)user_data;

  switch (action) {
  case CCA_RSS_DOWNLOAD_START:
    if (!quiet)
      g_printf("Updating channel %s...\n", c->identifier);
    break;

  case CCA_RSS_DOWNLOAD_END:
    break;

  case CCA_ENCLOSURE_DOWNLOAD_START:
    g_assert(channel_info);
    g_assert(enclosure);

    if (verbose)
      _print_item_update(enclosure, filename);

    break;

  case CCA_ENCLOSURE_DOWNLOAD_END:
    g_assert(channel_info);
    g_assert(enclosure);
    g_assert(filename);

#ifdef HAVE_TAGLIB
    _set_tags(filename, c);
#endif /* HAVE_TAGLIB */

    /* Update playlist. */
    if (c->playlist) {
      playlist_add(c->playlist, filename);

      if (verbose)
        printf(" * Added downloaded enclosure %s to playlist %s.\n", filename,
               c->playlist);
    }
    break;
  }
}

static void catchup_callback(void *user_data, channel_action action,
                             channel_info *channel_info, enclosure *enclosure,
                             const gchar *filename)
{
  struct channel_configuration *c = (struct channel_configuration *)user_data;

  switch (action) {
  case CCA_RSS_DOWNLOAD_START:
    if (!quiet)
      g_printf("Catching up with channel %s...\n", c->identifier);
    break;

  case CCA_RSS_DOWNLOAD_END:
    break;

  case CCA_ENCLOSURE_DOWNLOAD_START:
    g_assert(channel_info);
    g_assert(enclosure);

    if (verbose)
      _print_item_update(enclosure, filename);

    break;

  case CCA_ENCLOSURE_DOWNLOAD_END:
    break;
  }
}

static void list_callback(void *user_data, channel_action action,
                          channel_info *channel_info, enclosure *enclosure,
                          const gchar *filename)
{
  struct channel_configuration *c = (struct channel_configuration *)user_data;

  switch (action) {
  case CCA_RSS_DOWNLOAD_START:
    g_printf("Listing channel %s...\n", c->identifier);
    break;

  case CCA_RSS_DOWNLOAD_END:
    break;

  case CCA_ENCLOSURE_DOWNLOAD_START:
    g_assert(channel_info);
    g_assert(enclosure);

    if (verbose)
      _print_item_update(enclosure, filename);

    break;

  case CCA_ENCLOSURE_DOWNLOAD_END:
    break;
  }
}

static int _process_channel(const gchar *channel_directory, GKeyFile *kf,
                            const char *identifier, enum op op,
                            struct channel_configuration *defaults,
                            enclosure_filter *filter)
{
  channel *c;
  gchar *channel_filename, *channel_file;
  struct channel_configuration *channel_configuration;
  enclosure_filter *per_channel_filter = NULL;

  /* Check channel identifier and read channel configuration. */
  if (!g_key_file_has_group(kf, identifier)) {
    fprintf(stderr, "Unknown channel identifier %s.\n", identifier);

    return -1;
  }

  /* Verify the keys in the channel configuration. */
  if (channel_configuration_verify_keys(kf, identifier) < 0)
    return -1;

  channel_configuration = channel_configuration_new(kf, identifier, defaults);

  /* Check that mandatory keys were set. */
  if (!channel_configuration->url) {
    fprintf(stderr, "No feed URL set for channel %s.\n", identifier);

    channel_configuration_free(channel_configuration);
    return -1;
  }

  if (!channel_configuration->spool_directory) {
    fprintf(stderr, "No spool directory set for channel %s.\n", identifier);

    channel_configuration_free(channel_configuration);
    return -1;
  }

  /* Construct channel file name. */
  channel_filename = g_strjoin(".", identifier, "xml", NULL);
  channel_file = g_build_filename(channel_directory, channel_filename, NULL);
  g_free(channel_filename);

  if (new_only && access(channel_file, F_OK) == 0) {
    /* If we are only fetching new channels, skip the channel if there is
       already a channel file present. */

    channel_configuration_free(channel_configuration);
    return 0;
  }

  c = channel_new(channel_configuration->url, channel_file,
                  channel_configuration->spool_directory,
                  channel_configuration->filename_pattern, opts->resume);
  g_free(channel_file);

  if (!c) {
    fprintf(stderr, "Error parsing channel file for channel %s.\n", identifier);

    channel_configuration_free(channel_configuration);
    return -1;
  }

  /* Set up per-channel filter unless overridden on the command
     line. */
  if (!filter && channel_configuration->regex_filter) {
    per_channel_filter =
        enclosure_filter_new(channel_configuration->regex_filter, FALSE);

    filter = per_channel_filter;
  }

  switch (op) {
  case OP_UPDATE:
    channel_update(c, channel_configuration, update_callback, opts);
    break;

  case OP_CATCHUP:
    channel_update(c, channel_configuration, catchup_callback, opts);
    break;

  case OP_LIST:
    channel_update(c, channel_configuration, list_callback, opts);
    break;
  }

  /* Clean-up. */
  if (per_channel_filter)
    enclosure_filter_free(per_channel_filter);

  channel_free(c);
  channel_configuration_free(channel_configuration);

  return 0;
}

static GKeyFile *_configuration_file_open(const gchar *rcfile)
{
  GKeyFile *kf;
  GError *error = NULL;

  kf = g_key_file_new();

  if (!g_key_file_load_from_file(kf, rcfile, G_KEY_FILE_NONE, &error)) {
    fprintf(stderr, "Error reading configuration file %s: %s.\n", rcfile,
            error->message);
    g_error_free(error);
    g_key_file_free(kf);
    kf = NULL;
  }

  return kf;
}

static void _configuration_file_close(GKeyFile *kf)
{
  g_key_file_free(kf);
}

#ifdef HAVE_TAGLIB
static void _set_tags(const gchar *filename,
                      const struct channel_configuration *cfg)
{
  if (cfg->artist_tag || cfg->title_tag || cfg->album_tag || cfg->genre_tag ||
      cfg->year_tag || cfg->comment_tag) {
    TagLib_File *file;
    TagLib_Tag *tag;

    file = taglib_file_new(filename);

    if (file == NULL) {
      fprintf(stderr, "Error setting tags for file %s.\n", filename);
      return;
    }

    tag = taglib_file_tag(file);

    if (cfg->artist_tag) {
      taglib_tag_set_artist(tag, cfg->artist_tag);

      if (verbose)
        printf(" * Set artist tag to %s.\n", cfg->artist_tag);
    }

    if (cfg->title_tag) {
      taglib_tag_set_title(tag, cfg->title_tag);

      if (verbose)
        printf(" * Set title tag to %s.\n", cfg->title_tag);
    }

    if (cfg->album_tag) {
      taglib_tag_set_album(tag, cfg->album_tag);

      if (verbose)
        printf(" * Set album tag to %s.\n", cfg->album_tag);
    }

    if (cfg->genre_tag) {
      taglib_tag_set_genre(tag, cfg->genre_tag);

      if (verbose)
        printf(" * Set genre tag to %s.\n", cfg->genre_tag);
    }

    if (cfg->year_tag) {
      taglib_tag_set_year(tag, g_ascii_strtoull(cfg->year_tag, NULL, 10));

      if (verbose)
        printf(" * Set year tag to %s.\n", cfg->year_tag);
    }

    if (cfg->comment_tag) {
      taglib_tag_set_comment(tag, cfg->comment_tag);

      if (verbose)
        printf(" * Set comment tag to %s.\n", cfg->comment_tag);
    }

    if (!taglib_file_save(file))
      fprintf(stderr, "Error setting tags for file %s.\n", filename);

    taglib_tag_free_strings();
    taglib_file_free(file);
  }
}
#endif /* HAVE_TAGLIB */

static int playlist_add(const gchar *playlist_file, const gchar *media_file)
{
  FILE *f;

  f = fopen(playlist_file, "a");

  if (!f) {
    fprintf(stderr, "Error opening playlist file %s: %s.\n", playlist_file,
            strerror(errno));
    return -1;
  }

  fprintf(f, "%s\n", media_file);
  fclose(f);
  return 0;
}
