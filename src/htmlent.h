/*
  Copyright (C) 2006 Marius L. Jøhndal
 
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
 
  $Id: htmlent.h,v 1.1 2007/09/20 17:49:23 mariuslj Exp $
  
*/

#ifndef HTMLENT_H
#define HTMLENT_H

#include <glib.h>

GHashTable *htmlent_hash_new(void);
void htmlent_hash_destroy(GHashTable *h);

#endif /* HTMLENT_H */

/* 
   Local Variables:
   mode:c
   indent-tabs-mode:nil
   c-basic-offset:2
   coding:utf-8
   End:
*/

