/*
  Copyright (C) 2005 Marius L. Jøhndal
 
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
 
  $Id: castget.c,v 1.4 2005/11/14 00:08:54 mariuslj Exp $
  
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
#include "libcastget.h"

enum op {
  OP_UPDATE,
  OP_CATCHUP,
  OP_LIST
};

struct castget {
  int verbose;
  int quiet;
  const gchar *identifier;
  enum op op;
  GKeyFile *kf;
};

static int _process_channel(const gchar *channel_directory, GKeyFile *kf, const char *identifier, 
                            int verbose, int quiet, enum op op);
static void usage(void);
static void version(void);
static GKeyFile *_configuration_file_open(void);
static void _configuration_file_close(GKeyFile *kf);
#ifdef ENABLE_ID3LIB
static int _id3_set(const gchar *filename, int clear, int verbose, const gchar *lead_artist, 
                    const gchar *content_group, const gchar *title, 
                    const gchar *album, const gchar *content_type, const gchar *year,
                    const gchar *comment);
static int _id3_check_and_set(const gchar *filename, const struct castget *c);
#endif /* ENABLE_ID3LIB */

int main(int argc, char **argv)
{
  enum op op = OP_UPDATE;
  int verbose = 0;
  int quiet = 0;
  int c, i;
  int ret = 0;
  gchar **groups;
  gchar *channeldir;
  GKeyFile *kf;

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

  /* Read configuration file. */
  kf = _configuration_file_open();

  if (kf) {
    /* Perform actions. */
    if (optind < argc) {
      while (optind < argc)
        _process_channel(channeldir, kf, argv[optind++], verbose, quiet, op);
    } else {
      groups = g_key_file_get_groups(kf, NULL);
      
      for (i = 0; groups[i]; i++)
        _process_channel(channeldir, kf, groups[i], verbose, quiet, op);
      
      g_strfreev(groups);
    }
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
  g_printf("Copyright (C) 2005 Marius L. Jøhndal <mariuslj at ifi.uio.no>\n");
}

static void _action_callback(void *user_data, libcastget_channel_action action, libcastget_channel_info *channel_info,
                             libcastget_enclosure *enclosure, const gchar *filename)
{
  struct castget *c = (struct castget *)user_data;

  if (!c->quiet) {
    switch (action) {
    case CCA_RSS_DOWNLOAD_START:
      switch (c->op) {
      case OP_UPDATE:
        g_printf("Updating channel %s...\n", c->identifier);
        break;
        
      case OP_CATCHUP:
        g_printf("Catching up with channel %s...\n", c->identifier);
        break;
        
      case OP_LIST:
        g_printf("Listing channel %s...\n", c->identifier);
        break;
      }
      break;

    case CCA_RSS_DOWNLOAD_END:
      break;

    case CCA_ENCLOSURE_DOWNLOAD_START:
      g_assert(channel_info);
      g_assert(enclosure);

      if (c->verbose)
        switch (c->op) {
        case OP_UPDATE:
          g_printf("Downloading %s (%ld bytes) from %s\n", enclosure->url, enclosure->length, 
                   channel_info->title);
          break;

        case OP_CATCHUP:
          g_printf("Catching up on %s (%ld bytes) from %s\n", enclosure->url, enclosure->length, 
                   channel_info->title);
          break;

        case OP_LIST:
          g_printf("* %s (%ld bytes) from %s\n", enclosure->url, enclosure->length, 
                   channel_info->title);
          break;
        }

      break;

    case CCA_ENCLOSURE_DOWNLOAD_END:
      g_assert(channel_info);
      g_assert(enclosure);

      if (c->op == OP_UPDATE) {
        g_assert(filename);

        if (!strcmp(enclosure->type, "audio/mpeg")) {
#ifdef ENABLE_ID3LIB
          _id3_check_and_set(filename, c);
#endif /* ENABLE_ID3LIB */
        }
      }
      break;
    }
  }
}

static int _process_channel(const gchar *channel_directory, GKeyFile *kf, const char *identifier, 
                            int verbose, int quiet, enum op op)
{
  int ret = 0;
  libcastget_channel *c;
  gchar *url, *channel_filename, *channel_file, *spool_directory;
  GError *error = NULL;
  struct castget castget;

  castget.verbose = verbose;
  castget.quiet = quiet;
  castget.identifier = identifier;
  castget.op = op;
  castget.kf = kf;

  if (g_key_file_has_group(kf, identifier)) {
    if (g_key_file_has_key(kf, identifier, "url", &error)) {
      url = g_key_file_get_value(kf, identifier, "url", &error);

      if (g_key_file_has_key(kf, identifier, "spool", &error)) {
        spool_directory = g_key_file_get_value(kf, identifier, "spool", &error);

        /* Construct channel file name. */
        channel_filename = g_strjoin(".", identifier, "xml", NULL);
        channel_file = g_build_filename(channel_directory, channel_filename, NULL);
        g_free(channel_filename);
        
        c = libcastget_channel_new(url, channel_file, spool_directory);
        
        g_free(channel_file);

        if (c) {
          switch (op) {
          case OP_UPDATE:
            libcastget_channel_update(c, &castget, _action_callback);
            break;
            
          case OP_CATCHUP:
            libcastget_channel_catchup(c, &castget, _action_callback);
            break;
            
          case OP_LIST:
            libcastget_channel_list(c, &castget, _action_callback);
            break;
          }
          
          libcastget_channel_free(c);
        } else {
          fprintf(stderr, "Error parsing channel file for channel %s.\n", identifier);
          ret = 1;
        }
      } else {
        fprintf(stderr, "No spool directory for channel %s.\n", identifier);
        ret = 1;
      }
    } else {
      fprintf(stderr, "No feed URL for channel %s.\n", identifier);
      ret = 1;
    }
  } else {
    fprintf(stderr, "Unknown channel identifier %s.\n", identifier);
    ret = 1;
  }

  return ret;
}

static GKeyFile *_configuration_file_open(void)
{
  GKeyFile *kf;
  gchar *rcfile;
  GError *error = NULL;

  kf = g_key_file_new();
  rcfile = g_build_filename(g_get_home_dir(), ".castgetrc", NULL);

  if (!g_key_file_load_from_file(kf, rcfile, G_KEY_FILE_NONE, &error)) {
    fprintf(stderr, "Configuration file %s not found.\n", rcfile);
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

static int _id3_set(const gchar *filename, int clear, int verbose, const gchar *lead_artist, 
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
      printf("Set ID3 tag lead artist to %s.\n", lead_artist);
  }

  if (content_group) {
    errors += _id3_find_and_set_frame(tag, ID3FID_CONTENTGROUP, content_group);

    if (verbose)
      printf("Set ID3 tag content group to %s.\n", content_group);
  }

  if (title) {
    errors += _id3_find_and_set_frame(tag, ID3FID_TITLE, title);

    if (verbose)
      printf("Set ID3 tag title to %s.\n", title);
  }

  if (album) {
    errors += _id3_find_and_set_frame(tag, ID3FID_ALBUM, album);

    if (verbose)
      printf("Set ID3 tag album to %s.\n", album);
  }

  if (content_type) {
    errors += _id3_find_and_set_frame(tag, ID3FID_CONTENTTYPE, content_type);

    if (verbose)
      printf("Set ID3 tag content type to %s.\n", content_type);
  }

  if (year) {
    errors += _id3_find_and_set_frame(tag, ID3FID_YEAR, year);

    if (verbose)
      printf("Set ID3 title year to %s.\n", year);
  }

  if (comment) {
    errors += _id3_find_and_set_frame(tag, ID3FID_COMMENT, comment);

    if (verbose)
      printf("Set ID3 tag comment to %s.\n", comment);
  }

  if (!errors)
    ID3Tag_Update(tag);

  ID3Tag_Delete(tag);

  return errors;
}

static int _id3_check_and_set(const gchar *filename, const struct castget *c)
{
  const gchar *lead_artist = NULL;
  const gchar *content_group = NULL; 
  const gchar *title = NULL; 
  const gchar *album = NULL;
  const gchar *content_type = NULL;
  const gchar *year = NULL;
  const gchar *comment = NULL;
  GError *error = NULL;

  if (g_key_file_has_key(c->kf, c->identifier, "id3leadartist", &error)) {
    lead_artist = g_key_file_get_value(c->kf, c->identifier, "id3leadartist", &error);
  }
      
  if (g_key_file_has_key(c->kf, c->identifier, "id3contentgroup", &error)) {
    content_group = g_key_file_get_value(c->kf, c->identifier, "id3contentgroup", &error);
  }
      
  if (g_key_file_has_key(c->kf, c->identifier, "id3title", &error)) {
    title = g_key_file_get_value(c->kf, c->identifier, "id3title", &error);
  }
      
  if (g_key_file_has_key(c->kf, c->identifier, "id3album", &error)) {
    album = g_key_file_get_value(c->kf, c->identifier, "id3album", &error);
  }
      
  if (g_key_file_has_key(c->kf, c->identifier, "id3album", &error)) {
    album = g_key_file_get_value(c->kf, c->identifier, "id3album", &error);
  }
      
  if (g_key_file_has_key(c->kf, c->identifier, "id3contenttype", &error)) {
    content_type = g_key_file_get_value(c->kf, c->identifier, "id3contenttype", &error);
  }
      
  if (g_key_file_has_key(c->kf, c->identifier, "id3year", &error)) {
    year = g_key_file_get_value(c->kf, c->identifier, "id3year", &error);
  }
      
  if (g_key_file_has_key(c->kf, c->identifier, "id3comment", &error)) {
    comment = g_key_file_get_value(c->kf, c->identifier, "id3comment", &error);
  }
      
  if (lead_artist || content_group || title || album || content_type || year || comment)
    return _id3_set(filename, 0, c->verbose, lead_artist, content_group, title, album, content_type, year, comment);
  else
    return 0;
}

#endif /* ENABLE_ID3LIB */

/* 
   Local Variables:
   mode:c
   indent-tabs-mode:nil
   c-basic-offset:2
   coding:utf-8
   End:
*/
