/*-- make_ggobi.c --*/
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

#include <math.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"

#include "read_xml.h"

#ifdef SUPPORT_PLUGINS
#include "plugin.h"
#endif

#ifdef USE_MYSQL
#include "read_mysql.h"
#endif

guint GGobiSignals[MAX_GGOBI_SIGNALS];

/*-- initialize variables which don't depend on the size of the data --*/
void globals_init(ggobid * gg)
{
  colorschemed *scheme = gg->activeColorScheme;

  gg->close_pending = false;

  gg->glyph_id.type = gg->glyph_0.type = FC;
  gg->glyph_id.size = gg->glyph_0.size = 1;
  gg->color_0 = 0;
  gg->color_id = scheme->n - 1; /* default: initialize to last color */
  /* can be overriden in xml file */

  /*-- for linking by categorical variable --*/
  gg->linkby_cv = false;

  gg->lims_use_visible = true;
  gg->buttondown = 0;  /*-- no button is pressed --*/

  gg->d = NULL;

  gg->save.window = NULL;

  gg->statusbar_p = true;
}


gboolean
fileset_read_init(const gchar * ldata_in, const gchar *pluginModeName, GGobiPluginInfo *plugin, ggobid * gg)
{
  GSList *ds = fileset_read(ldata_in, pluginModeName, plugin, gg);
  for(; ds; ds = ds->next) {
    datad_init((GGobiData*) ds->data, gg, FALSE);
  }

  return (ds != NULL);
}

// returns a list of datasets (some input types (eg. xml) may return multiple data types)
GSList*
fileset_read(const gchar * ldata_in, const gchar *pluginModeName, GGobiPluginInfo *plugin, ggobid * gg)
{
  InputDescription *desc;
  gboolean ok = true;

  desc = fileset_generate(ldata_in, pluginModeName, plugin, gg);

  if (desc == NULL) {
    g_printerr("Cannot locate the file %s\n", ldata_in);
    return (false);
  }

  if (desc->mode == unknown_data && desc->desc_read_input == NULL) {
    g_printerr("Cannot determine the format of the data in file %s\n",
               desc->fileName);
    return (false);
  }

  gg->input = desc;

  return(read_input(desc, gg));
}


// returns a list of datasets (some input types (eg. xml) may return multiple data types)
GSList*
read_input(InputDescription * desc, ggobid * gg)
{
  GSList* ds;
  if (desc == NULL)
    return (NULL);

  if (desc->desc_read_input) {
    if(!desc->baseName) 
      completeFileDesc(desc->fileName, desc);
      ds = desc->desc_read_input(desc, gg, NULL);
  } else
    g_printerr("Unknown data type in read_input\n");

  if (ds && sessionOptions->verbose == GGOBI_VERBOSE) {
    showInputDescription(desc, gg);
  }

  return (ds);
}





/*
 * the first display is initialized in datad_init, so turn on
 * event handlers there as well
*/
void
make_ggobi(GGobiOptions * options, gboolean processEvents, ggobid * gg)
{
  gboolean init_data = false;

  /*-- some initializations --*/
  gg->displays = NULL;

  globals_init(gg);  /*-- variables that don't depend on the data --*/

  special_colors_init (gg);

  wvis_init(gg);
  svis_init(gg);
  make_ui(gg);

  /* If the user specified a data file on the command line, then 
     try to load that. If not, then look through the input plugins
     for the first interactive one (i.e. with an interactive="true" attribute)
     and then we try to run that. This allows input plugins that provide
     a user interface to query the user as to what to do.
   */
  if (options->data_in != NULL) {
    if (fileset_read_init(options->data_in, sessionOptions->data_type, 
      NULL, gg))
    {
      init_data = true;
    }
  } else {
#ifdef SUPPORT_PLUGINS
    if (runInteractiveInputPlugin(gg) == NULL) {
      if (sessionOptions->data_type)
        fprintf(stderr, "No available plugin to handle input mode %s\n",
                sessionOptions->data_type);
      fflush(stderr);
    }
#endif
  }


#ifdef SUPPORT_PLUGINS
  if (sessionOptions->info != NULL)
    registerPlugins(gg, sessionOptions->info->plugins);
#endif

  resetDataMode();

  start_ggobi(gg, init_data, sessionOptions->info->createInitialScatterPlot);

  if (sessionOptions->restoreFile) {
    processRestoreFile(sessionOptions->restoreFile, gg);
  }

  gg->status_message_func = gg_write_to_statusbar;


  if (processEvents) {
    gtk_main();
  }
}

void
resetDataMode()
{
    if(sessionOptions->data_type)
	free(sessionOptions->data_type);
    sessionOptions->data_type = NULL;
    sessionOptions->data_mode = unknown_data;
}

void start_ggobi(ggobid * gg, gboolean init_data, gboolean createPlot)
{
  GGobiData *d;
  if (init_data) {
    GSList *l;
    gboolean firstd = createPlot;
    for (l = gg->d; l; l = l->next) {
      d = (GGobiData *) l->data;
      datad_init(d, gg, firstd);
      firstd = false;
    }

    /*-- destroy and rebuild the menu every time data is read in --*/
    display_menu_build(gg);
  }

  /*-- now that we've read some data, set the mode --*/
  if (createPlot && gg->d) {
    d = (GGobiData *) gg->d->data;
    if (d != NULL) {
      if (d->ncols > 0) {
        gg->pmode = (d->ncols == 1) ? P1PLOT : XYPLOT;
        gg->imode = DEFAULT_IMODE;
      }
    }
  } else {
    gg->pmode = NULL_PMODE;
    gg->imode = NULL_IMODE;
  }

  gg->pmode_prev = gg->pmode;
  gg->imode_prev = gg->imode;
  /*-- initialize the mode menus for the new mode --*/
  /*main_miscmenus_update(NULL_PMODE, NULL_IMODE, (displayd *) NULL, gg);*/
}
