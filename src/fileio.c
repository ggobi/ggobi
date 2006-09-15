/* fileio.c */
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Common Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

#include "ggobi.h"
#include "unistd.h"
#include <glib/gstdio.h>

// FIXME: could probably just be blended into a misc utilities collection

/*--------------------------------------------------------------------*/
/*          Utilities                                                 */
/*--------------------------------------------------------------------*/

gchar *
absolute_path(const gchar *path)
{
  gchar *abs_path;
  if (!g_path_is_absolute(path)) { /* first make sure it is absolute */
    gchar *cwd = g_get_current_dir();
    abs_path = g_build_filename(cwd, path, NULL);
    g_free(cwd);
  } else abs_path = g_strdup(path);
  return abs_path;
}

/* Note the only way to reliably test readability on Windows is to actually 
    try to open the file. Here we are basically checking for existence. */
gboolean
file_is_readable (const gchar *fileName)
{
  if (GLIB_CHECK_VERSION(2,8,0))
    return(!g_access(fileName, R_OK));
  return !access(fileName, R_OK);
}
