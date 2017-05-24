/*
  Copyright (C) 2010 Tony Armitstead
  Copyright (C) 2017 Marius L. Jøhndal

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
#include <stdio.h>
#include <ctype.h>
#include <glib.h>
#include "date_parsing.h"

static const char *days[7] = {
  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

static const char *months[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

GDate *parse_rfc822_date(const char *rfc822_date_str)
{
  const char *dstr = rfc822_date_str;
  int i = 0;
  int day, month, year;
  char mstr[20];

  while (isspace(*dstr))
    dstr++;

  if (*dstr == '\0')
    return NULL;

  /* Skip past any valid day, field */
  for (i = 0; i < sizeof(days)/sizeof(days[0]); i++) {
    if (strncmp(dstr, days[i], 3) == 0)
      break;
  }

  if (i < sizeof(days)/sizeof(days[0])) {
    dstr += 3;

    while (isspace(*dstr))
      dstr++;

    if (*dstr == '\0')
      return NULL;

    if (*dstr == ',')
      dstr++;

    while (isspace(*dstr))
      dstr++;

    if (*dstr == '\0')
      return NULL;
  }

  /* Decode day, month, year */
  if (sscanf(dstr, "%d %s %d", &day, mstr, &year) != 3)
    return NULL;

  for (i=0; i < sizeof(months)/sizeof(months[0]); i++) {
    if (strncmp(mstr, months[i], 3) == 0)
      break;
  }

  if (i == sizeof(months)/sizeof(months[0]))
    return NULL;

  month = i + 1;

  if (year < 1900) {
    if (year < 50)
      year += 2000;
    else
      year += 1900;
  }

  if (!g_date_valid_dmy(day, month, year))
    return NULL;

  return g_date_new_dmy(day, month, year);
}
