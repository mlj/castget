/*
  Copyright (C) 2005-2017 Marius L. Jøhndal

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

#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <curl/curl.h>
#include "urlget.h"
#include "progress.h"

int urlget_file(const char *url, FILE *f, int debug)
{
  return urlget_buffer(url, (void *)f, NULL, 0, debug, NULL);
}

int urlget_buffer(const char *url, void *user_data,
                  size_t (*write_buffer)(void *buffer, size_t size, size_t nmemb, void *user_data),
                  long resume_from, int debug, progress_bar *pb)
{
  CURL *easyhandle;
  CURLcode success;
  char errbuf[CURL_ERROR_SIZE];
  int ret = 0;
  gchar *user_agent;

  /* Construct user agent string. */
  user_agent = g_strdup_printf("%s (%s rss enclosure downloader)", PACKAGE_STRING, PACKAGE);

  /* Initialise curl. */
  easyhandle = curl_easy_init();

  if (easyhandle) {
    curl_easy_setopt(easyhandle, CURLOPT_URL, url);
    curl_easy_setopt(easyhandle, CURLOPT_ERRORBUFFER, errbuf);
    curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, write_buffer);
    curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, user_data);
    curl_easy_setopt(easyhandle, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(easyhandle, CURLOPT_USERAGENT, user_agent);
    curl_easy_setopt(easyhandle, CURLOPT_ACCEPT_ENCODING, "");

    if (pb) {
      curl_easy_setopt(easyhandle, CURLOPT_NOPROGRESS, 0);
      curl_easy_setopt(easyhandle, CURLOPT_PROGRESSFUNCTION, progress_bar_cb);
      curl_easy_setopt(easyhandle, CURLOPT_PROGRESSDATA, pb);
    } else
      curl_easy_setopt(easyhandle, CURLOPT_NOPROGRESS, 1);

    if (resume_from)
      curl_easy_setopt(easyhandle, CURLOPT_RESUME_FROM_LARGE, (curl_off_t)resume_from);

    curl_easy_setopt(easyhandle, CURLOPT_VERBOSE, debug);

    success = curl_easy_perform(easyhandle);

    curl_easy_cleanup(easyhandle);

    if (success != CURLE_OK) {
      if (pb)
        /* Insert an extra CR on stdout as we may have started printing a progress bar there. */
        fprintf(stdout, "\n");

      if (success == CURLE_WRITE_ERROR) {
        fprintf(stderr, "Error retrieving %s: %s: ", url, errbuf);
        perror(NULL);
        fprintf(stderr, "\n");
      } else
        fprintf(stderr, "Error retrieving %s: %s\n", url, errbuf);

      ret = 1;
    }
  } else
    ret = 1;

  g_free(user_agent);

  return ret;
}
