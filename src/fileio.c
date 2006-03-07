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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "GGobiAPI.h"
#include "plugin.h"

/*--------------------------------------------------------------------*/
/*               Initialization                                       */
/*--------------------------------------------------------------------*/

static gboolean
isUnknownInputMode (const gchar * modeName)
{
  gboolean status;
  status = !modeName || modeName == ""
    || strcmp (modeName, DefaultUnknownInputModeName) == 0;
  return (status);
}

/*----------------------------------------------------------------------
  Initialize and populate in an InputDescription:  work out the
    mode of the data, the fully expanded name of the file, the
    base name, the name of its directory, ...
    
  The algorithm is as follows:
    If we're given a plugin, ask it for the description, if it has one, return
    Loop through all available plugins
      If the input mode is unknown
        ask the plugin to probe the file, if it can
        (Currently no input plugins support probing)
        If it can't probe or probe returns true, ask the plugin for a description.
      If the mode is known
        ask the plugin if it supports it, if it does, ask for a description.
      If we have a description, return, else continue
    Return NULL (failure)
      
----------------------------------------------------------------------*/

InputDescription *
fileset_generate (const gchar * fileName,
                  const gchar * modeName,
                  GGobiPluginInfo * plugin, ggobid * gg)
{
  GList *els;
  gboolean isUnknownMode;

  if (plugin) {
    InputDescription *desc;
    desc = callInputPluginGetDescription (fileName, modeName, plugin, gg);
    if (desc)
      return (desc);
  }

  isUnknownMode = isUnknownInputMode (modeName);
  els = sessionOptions->info->inputPlugins;
  
  if (els) {
    gint i, n;
    n = g_list_length (els);
    for (i = 0; i < n; i++) {
      gboolean handlesFile = false;
      GGobiPluginInfo *oplugin;
      GGobiInputPluginInfo *info;
      oplugin = g_list_nth_data (els, i);
      info = oplugin->info.i;

      /* Use the probe only if the user has not given us a 
         specific format/plugin. */
      if (isUnknownMode) {
        if (info->probe)
          handlesFile = info->probe (fileName, gg, oplugin);
        else
          handlesFile = true;
      }

      if ((isUnknownMode && handlesFile)
          || (modeName && oplugin
              && pluginSupportsInputMode (modeName, oplugin))) {
        InputDescription *desc;
        desc =
          callInputPluginGetDescription (fileName, modeName, oplugin, gg);
        if (desc)
          return (desc);
      }
    }
  }

  return (NULL);
}

void
completeFileDesc (const gchar * fileName, InputDescription * desc)
{
  gint n;

  if (!desc->baseName) {
    desc->baseName = g_strdup (fileName);
  }

  /* Now compute the directory name. */
  desc->dirName = g_path_get_dirname(desc->baseName);
}

void
showInputDescription (InputDescription * desc, ggobid * gg)
{
  FILE *out = stderr;
  gint i;
  fprintf (out, "Input File Information:\n");
  fprintf (out, "\tFile name: %s  (extension: %s)\n",
           desc->fileName, desc->givenExtension);
  fprintf (out, "\tDirectory: %s\n", desc->dirName);
#if 0
  fprintf (out, "\tFormat: %s (%d), verified: %s\n",
           GGOBI (getDataModeDescription) (desc->mode), desc->mode,
           desc->canVerify ? "yes" : "no");
#endif

  if (desc->extensions) {
    fprintf (out, "Auxillary files\n");
    for (i = 0; i < g_slist_length (desc->extensions); i++) {
      fprintf (out, "  %d) %s\n",
               i, (gchar *) g_slist_nth_data (desc->extensions, i));
    }
  }
  fflush (out);
}

/*--------------------------------------------------------------------*/
/*          Determining the mode of the data                          */
/*--------------------------------------------------------------------*/

/** FIXME: Need a more robust URL identification scheme */
gboolean
isURL (const gchar * fileName)
{
  return ((strncmp (fileName, "http:", 5) == 0 ||
           strncmp (fileName, "ftp:", 4) == 0));
}
