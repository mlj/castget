/*
  Copyright (C) 2005, 2006 Marius L. Jøhndal
 
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
 
  $Id: castget.c,v 1.17 2006/07/28 20:32:28 mariuslj Exp $
  
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#define _GNU_SOURCE
#include <getopt.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#include <errno.h>
#include <string.h>
#include <libxml/parser.h>
#ifdef ENABLE_ID3LIB
#include <id3.h>
#endif /* ENABLE_ID3LIB */
#include "configuration.h"
#include "libcastget.h"

enum op {
  OP_UPDATE,
  OP_CATCHUP,
  OP_LIST
};

static int _verify_keys(GKeyFile *kf, const char *identifier);
static int _process_channel(const gchar *channel_directory, GKeyFile *kf, const char *identifier,
                            enum op op, struct channel_configuration *defaults);
static void usage(void);
static void version(void);
static GKeyFile *_configuration_file_open(void);
static void _configuration_file_close(GKeyFile *kf);
#ifdef ENABLE_ID3LIB
static int _id3_set(const gchar *filename, int clear, const gchar *lead_artist, 
                    const gchar *content_group, const gchar *title, 
                    const gchar *album, const gchar *content_type, const gchar *year,
                    const gchar *comment);
static int _id3_check_and_set(const gchar *filename,
                              const struct channel_configuration *cfg);
#endif /* ENABLE_ID3LIB */
static int playlist_add(const gchar *playlist_file,
                        const gchar *media_file);

static int verbose = 0;
static int quiet = 0;

int main(int argc, char **argv)
{
  enum op op = OP_UPDATE;
  int c, i;
  int ret = 0;
  gchar **groups;
  gchar *channeldir;
  GKeyFile *kf;
  struct channel_configuration *defaults;

  for (;;) {
    int option_index = 0;

    static struct option long_options[] = {
      {"catchup", 0, 0, 'c'},
      {"help", 0, 0, 'h'},
      {"list", 0, 0, 'l'},
      {"verbose", 0, 0, 'v'},
      {"version", 0, 0, 'V'},
      {"quiet", 0, 0, 'q'},
      {0, 0, 0, 0}
    };
    
    c = getopt_long(argc, argv, "chlqvV", long_options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 'c':
      op = OP_CATCHUP;
      break;

    case 'l':
      op = OP_LIST;
      break;

    case 'v':
      verbose = 1;
      break;

    case 'V':
      version();
      return 1;

    case 'q':
      quiet = 1;
      break;

    case 'h':
    default:
      usage();
      return 1;
    }
  }

  /* Do some additional sanity checking of options. */
  if (verbose && quiet) {
    usage();
    return 1;
  }

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
  kf = _configuration_file_open();

  if (kf) {
    /* Read defaults. */
    if (g_key_file_has_group(kf, "*")) {
      /* Verify the keys in the global configuration. */
      if (_verify_keys(kf, "*") < 0)
        return -1;

      defaults = channel_configuration_new(kf, "*", NULL);
    } else
      defaults = NULL;

    /* Perform actions. */
    if (optind < argc) {
      while (optind < argc)
        _process_channel(channeldir, kf, argv[optind++], op, defaults);
    } else {
      groups = g_key_file_get_groups(kf, NULL);
      
      for (i = 0; groups[i]; i++)
        if (strcmp(groups[i], "*"))
          _process_channel(channeldir, kf, groups[i], op, defaults);
      
      g_strfreev(groups);
    }
  
    /* Clean up defaults. */
    if (defaults)
      channel_configuration_free(defaults);
  } else
    ret = 1;

  /* Clean-up. */
  g_free(channeldir);

  if (kf)
    _configuration_file_close(kf);
  
  xmlCleanupParser();

  return ret;
}

static void usage(void)
{
  g_printf("Usage: castget [-c|-l|-V|-h] [-v|-q] [identifier(s)]\n\n");
  g_printf("  --catchup   -c    Catch up with channels.\n");
  g_printf("  --list      -l    List available enclosures.\n");
  g_printf("  --version   -V    Print version number.\n");
  g_printf("  --help      -h    Print usage information.\n");
  g_printf("\n");
  g_printf("  --verbose   -v    Verbose mode.\n");
  g_printf("  --quiet     -q    Quiet mode.\n\n");
  g_printf("The identifiers identifies the channels affected by the selected operation.\n");
  g_printf("If no identifier is supplied all channels are affect.\n");
}

static void version(void)
{
  g_printf("%s %s\n", PACKAGE, VERSION);
  g_printf("Copyright (C) 2005, 2006 Marius L. Jøhndal <mariuslj at ifi.uio.no>\n");
}

static void update_callback(void *user_data, libcastget_channel_action action, 
                            libcastget_channel_info *channel_info,
                            libcastget_enclosure *enclosure, const gchar *filename)
{
  struct channel_configuration *c = (struct channel_configuration *)user_data;

  if (!quiet) {
    switch (action) {
    case CCA_RSS_DOWNLOAD_START:
      g_printf("Updating channel %s...\n", c->identifier);
      break;
      
    case CCA_RSS_DOWNLOAD_END:
      break;
      
    case CCA_ENCLOSURE_DOWNLOAD_START:
      g_assert(channel_info);
      g_assert(enclosure);

      if (verbose) {
        if (enclosure->length > 1024*1024*1024) {
          g_printf(" * Downloading %s (%.1f GB) from %s\n", 
                   enclosure->filename, (float)enclosure->length / (1024.0*1024.0*1024.0), 
                   channel_info->title);
        } else if (enclosure->length > 1024*1024) {
          g_printf(" * Downloading %s (%.1f MB) from %s\n", 
                   enclosure->filename, (float)enclosure->length / (1024.0*1024.0),
                   channel_info->title);
        } else if (enclosure->length > 1024) {
          g_printf(" * Downloading %s (%.1f kB) from %s\n", 
                   enclosure->filename, (float)enclosure->length / 1024.0,
                   channel_info->title);
        } else if (enclosure->length > 0) {
          g_printf(" * Downloading %s (%ld bytes) from %s\n", 
                   enclosure->filename, enclosure->length,
                   channel_info->title);
        } else {
          g_printf(" * Downloading %s from %s\n",
                   enclosure->filename,
                   channel_info->title);
        }
      }
      break;

    case CCA_ENCLOSURE_DOWNLOAD_END:
      g_assert(channel_info);
      g_assert(enclosure);
      g_assert(filename);

      /* Set media tags. */
      if (enclosure->type && !strcmp(enclosure->type, "audio/mpeg")) {
#ifdef ENABLE_ID3LIB
        _id3_check_and_set(filename, c);
#endif /* ENABLE_ID3LIB */
      }

      /* Update playlist. */
      if (c->playlist) {
        playlist_add(c->playlist, filename);

        if (verbose)
          printf(" * Added downloaded enclosure %s to playlist %s.\n", 
                 filename, c->playlist);
      }
      break;
    }
  } 
}

static void catchup_callback(void *user_data, libcastget_channel_action action, libcastget_channel_info *channel_info,
                             libcastget_enclosure *enclosure, const gchar *filename)
{
  struct channel_configuration *c = (struct channel_configuration *)user_data;

  if (!quiet) {
    switch (action) {
    case CCA_RSS_DOWNLOAD_START:
      g_printf("Catching up with channel %s...\n", c->identifier);
      break;

    case CCA_RSS_DOWNLOAD_END:
      break;

    case CCA_ENCLOSURE_DOWNLOAD_START:
      g_assert(channel_info);
      g_assert(enclosure);

      if (verbose)
        g_printf("Catching up on %s (%ld bytes) from %s\n", enclosure->url, enclosure->length,
                 channel_info->title);
      break;

    case CCA_ENCLOSURE_DOWNLOAD_END:
      break;
    }
  }
}

static void list_callback(void *user_data, libcastget_channel_action action, libcastget_channel_info *channel_info,
                          libcastget_enclosure *enclosure, const gchar *filename)
{
  struct channel_configuration *c = (struct channel_configuration *)user_data;

  if (!quiet) {
    switch (action) {
    case CCA_RSS_DOWNLOAD_START:
      g_printf("Listing channel %s...\n", c->identifier);
      break;

    case CCA_RSS_DOWNLOAD_END:
      break;

    case CCA_ENCLOSURE_DOWNLOAD_START:
      g_assert(channel_info);
      g_assert(enclosure);


      if (enclosure->length > 1024*1024*1024) {
        g_printf(" * %s (%.1f GB) from %s\n", 
                 enclosure->filename, (float)enclosure->length / (1024.0*1024.0*1024.0), 
                 channel_info->title);
      } else if (enclosure->length > 1024*1024) {
        g_printf(" * %s (%.1f MB) from %s\n", 
                 enclosure->filename, (float)enclosure->length / (1024.0*1024.0),
                 channel_info->title);
      } else if (enclosure->length > 1024) {
        g_printf(" * %s (%.1f kB) from %s\n", 
                 enclosure->filename, (float)enclosure->length / 1024.0,
                 channel_info->title);
      } else if (enclosure->length > 0) {
        g_printf(" * %s (%ld bytes) from %s\n", 
                 enclosure->filename, enclosure->length, channel_info->title);
      } else {
        g_printf(" * %s from %s\n", enclosure->filename, channel_info->title);
      }

      break;

    case CCA_ENCLOSURE_DOWNLOAD_END:
      break;
    }
  }
}

static int _verify_keys(GKeyFile *kf, const char *identifier)
{
  int i;
  gchar **key_list;

  key_list = g_key_file_get_keys(kf, identifier, NULL, NULL);

  if (!key_list) {
    fprintf(stderr, "Error reading keys in configuration of channel %s.\n", identifier);

    return -1;
  }

  for (i = 0; key_list[i]; i++) {
    if (! (!strcmp(key_list[i], "url") ||
           !strcmp(key_list[i], "spool") ||
           !strcmp(key_list[i], "playlist") ||
           !strcmp(key_list[i], "id3leadartist") ||
           !strcmp(key_list[i], "id3contentgroup") ||
           !strcmp(key_list[i], "id3title") ||
           !strcmp(key_list[i], "id3album") ||
           !strcmp(key_list[i], "id3contenttype") ||
           !strcmp(key_list[i], "id3year") ||
           !strcmp(key_list[i], "id3comment"))) {
      fprintf(stderr, "Invalid key %s in configuration of channel %s.\n", key_list[i], identifier);
      return -1;
    }
  }

  g_strfreev(key_list);

  return 0;
}

static int _process_channel(const gchar *channel_directory, GKeyFile *kf, const char *identifier, 
                            enum op op, struct channel_configuration *defaults)
{
  libcastget_channel *c;
  gchar *channel_filename, *channel_file;
  struct channel_configuration *channel_configuration;

  /* Check channel identifier and read channel configuration. */
  if (!g_key_file_has_group(kf, identifier)) {
    fprintf(stderr, "Unknown channel identifier %s.\n", identifier);

    return -1;
  }

  /* Verify the keys in the channel configuration. */
  if (_verify_keys(kf, identifier) < 0)
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
        
  c = libcastget_channel_new(channel_configuration->url, channel_file, 
                             channel_configuration->spool_directory);
  g_free(channel_file);

  if (!c) {
    fprintf(stderr, "Error parsing channel file for channel %s.\n", identifier);

    channel_configuration_free(channel_configuration);
    return -1;
  }
    
  switch (op) {
  case OP_UPDATE:
    libcastget_channel_update(c, channel_configuration, update_callback);
    break;
            
  case OP_CATCHUP:
    libcastget_channel_catchup(c, channel_configuration, catchup_callback);
    break;
            
  case OP_LIST:
    libcastget_channel_list(c, channel_configuration, list_callback);
    break;
  }
          
  libcastget_channel_free(c);
  channel_configuration_free(channel_configuration);

  return 0;
}

static GKeyFile *_configuration_file_open(void)
{
  GKeyFile *kf;
  gchar *rcfile;
  GError *error = NULL;

  kf = g_key_file_new();
  rcfile = g_build_filename(g_get_home_dir(), ".castgetrc", NULL);

  if (!g_key_file_load_from_file(kf, rcfile, G_KEY_FILE_NONE, &error)) {
    fprintf(stderr, "Error reading configuration file %s: %s.\n", rcfile, error->message);
    g_error_free(error);
    g_key_file_free(kf);
    kf = NULL;
  }

  g_free(rcfile);

  return kf;
}

static void _configuration_file_close(GKeyFile *kf)
{
  g_key_file_free(kf);
}

#ifdef ENABLE_ID3LIB
static int _id3_find_and_set_frame(ID3Tag *tag, ID3_FrameID id, const char *value)
{
  ID3Frame *frame;
  ID3Field *field;

  /* Remove existing tag to avoid issues with trashed frames. */
  while ((frame = ID3Tag_FindFrameWithID(tag, id)))
    ID3Tag_RemoveFrame(tag, frame);

  if (value && strlen(value) > 0) {
    frame = ID3Frame_NewID(id);
    g_assert(frame);
    
    ID3Tag_AttachFrame(tag, frame);
    
    field = ID3Frame_GetField(frame, ID3FN_TEXT);
    
    if (field)
      ID3Field_SetASCII(field, value); //TODO: UTF8
    else
      return 1;
  }

  return 0;
}

static int _id3_set(const gchar *filename, int clear, const gchar *lead_artist, 
                    const gchar *content_group, const gchar *title, const gchar *album, 
                    const gchar *content_type, const gchar *year, const gchar *comment)
{
  int errors = 0;
  ID3Tag *tag;

  tag = ID3Tag_New();

  if (!tag)
    return 1;

  ID3Tag_Link(tag, filename);

  if (clear)
    ID3Tag_Clear(tag); // TODO

  if (lead_artist) {
    errors += _id3_find_and_set_frame(tag, ID3FID_LEADARTIST, lead_artist);

    if (verbose)
      printf(" * Set ID3 tag lead artist to %s.\n", lead_artist);
  }

  if (content_group) {
    errors += _id3_find_and_set_frame(tag, ID3FID_CONTENTGROUP, content_group);

    if (verbose)
      printf(" * Set ID3 tag content group to %s.\n", content_group);
  }

  if (title) {
    errors += _id3_find_and_set_frame(tag, ID3FID_TITLE, title);

    if (verbose)
      printf(" * Set ID3 tag title to %s.\n", title);
  }

  if (album) {
    errors += _id3_find_and_set_frame(tag, ID3FID_ALBUM, album);

    if (verbose)
      printf(" * Set ID3 tag album to %s.\n", album);
  }

  if (content_type) {
    errors += _id3_find_and_set_frame(tag, ID3FID_CONTENTTYPE, content_type);

    if (verbose)
      printf(" * Set ID3 tag content type to %s.\n", content_type);
  }

  if (year) {
    errors += _id3_find_and_set_frame(tag, ID3FID_YEAR, year);

    if (verbose)
      printf(" * Set ID3 title year to %s.\n", year);
  }

  if (comment) {
    errors += _id3_find_and_set_frame(tag, ID3FID_COMMENT, comment);

    if (verbose)
      printf(" * Set ID3 tag comment to %s.\n", comment);
  }

  if (!errors)
    ID3Tag_Update(tag);

  ID3Tag_Delete(tag);

  return errors;
}

static int _id3_check_and_set(const gchar *filename,
                              const struct channel_configuration *cfg)
{
  if (cfg->id3_lead_artist || cfg->id3_content_group || cfg->id3_title || 
      cfg->id3_album || cfg->id3_content_type || cfg->id3_year || cfg->id3_comment)
    return _id3_set(filename, 0, cfg->id3_lead_artist, cfg->id3_content_group, 
                    cfg->id3_title, cfg->id3_album, cfg->id3_content_type, 
                    cfg->id3_year, cfg->id3_comment);
  else
    return 0;
}

#endif /* ENABLE_ID3LIB */

static int playlist_add(const gchar *playlist_file,
                        const gchar *media_file)
{
  FILE *f;

  f = fopen(playlist_file, "a");

  if (!f) {
    fprintf(stderr, "Error opening playlist file %s: %s.\n",
            playlist_file, strerror(errno));
    return -1;
  }

  fprintf(f, "%s\n", media_file); 
  fclose(f);
  return 0;
}


/* 
   Local Variables:
   mode:c
   indent-tabs-mode:nil
   c-basic-offset:2
   coding:utf-8
   End:
*/
