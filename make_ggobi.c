/*-- make_ggobi.c --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/
#include <math.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"

#ifdef GTK_2_0
#undef __gtk_marshal_MARSHAL_H__
#include "marshal.h"
#endif

#include "read_xml.h"

#ifdef SUPPORT_PLUGINS
#include "plugin.h"
#endif

#ifdef USE_MYSQL
#include "read_mysql.h"
#endif

guint GGobiSignals[MAX_GGOBI_SIGNALS];


/*
  This is for handling the event calls for variable selection. Gtk doesn't have
  a marshalling function of the appropriate form!
 */

#ifndef GTK_2_0
void
gtk_marshal_NONE__INT_POINTER_POINTER_POINTER(GtkObject * object,
                                              GtkSignalFunc func,
                                              gpointer func_data,
                                              GtkArg * args)
{
  (func) (object, GTK_VALUE_INT(args[0]), GTK_VALUE_POINTER(args[1]),
          GTK_VALUE_POINTER(args[2]), GTK_VALUE_POINTER(args[3]),
          func_data);
}
#endif

gboolean read_input(InputDescription * desc, ggobid * gg);

/*-- initialize variables which don't depend on the size of the data --*/
void globals_init(ggobid * gg)
{
  colorschemed *scheme = gg->activeColorScheme;

  gg->close_pending = false;

  gg->glyph_id.type = gg->glyph_0.type = FR;
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

#ifndef GTK_2_0
#else
#include "GtkSignalDef.c"
#endif
}

/*-- initialize variables which DO depend on the size of the data --*/
static void viewmodes_init(datad * d, ggobid * gg)
{
#ifdef EDIT_EDGES_IMPLEMENTED
  extern void edgeedit_init(ggobid * gg);
  edgeedit_init(gg);
#endif
  brush_init(d, gg);
}

gboolean fileset_read_init(const gchar * ldata_in, ggobid * gg)
{
  gint howMany;
  gboolean ans;
  howMany = g_slist_length(gg->d);
  ans = fileset_read(ldata_in, gg);
  if (ans) {
    datad *d;
    gint n, i;
    /* Loop over the newly added datad elements
       and update them.
     */
    n = g_slist_length(gg->d);
    for (i = howMany; i < n; i++) {
      d = (datad *) g_slist_nth_data(gg->d, i);
      datad_init(d, gg, (i + howMany) == 0);
    }
  }

  return (ans);
}


gboolean fileset_read(const gchar * ldata_in, ggobid * gg)
{
  InputDescription *desc;
  gboolean ok = true;

  desc = fileset_generate(ldata_in, sessionOptions->data_mode, gg);

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

  ok = read_input(desc, gg);

  return ok;                    /* need to check return codes of reading routines */
}

gboolean read_input(InputDescription * desc, ggobid * gg)
{
  gboolean ok = false;
  if (desc == NULL)
    return (ok);

  switch (desc->mode) {
  case xml_data:
  case url_data:
    ok = data_xml_read(desc, gg);
    break;

  case mysql_data:
#ifdef USE_MYSQL
    {
      extern MySQLLoginInfo DefaultMySQLInfo;
      getDefaultValuesFromFile(ldata_in);
      ok = read_mysql_data(&DefaultMySQLInfo, FALSE, gg);
    }
#else
    g_printerr("No support for reading MySQL\n");
#endif
    break;

  case binary_data:
    g_printerr("No support yet for binary data\n");
    break;

  case Sprocess_data:
    break;

  case ascii_data:
    ok = read_ascii_data(desc, gg);
    break;
  default:
    if (desc->desc_read_input) {
      ok = desc->desc_read_input(desc, gg, NULL);
    } else
      g_printerr("Unknown data type in read_input\n");
    break;
  }

  if (ok && sessionOptions->verbose == GGOBI_VERBOSE) {
    showInputDescription(desc, gg);
  }

  return (ok);
}

void pipeline_init(datad * d, ggobid * gg)
{
  gint i;

  /*-- a handful of allocations and initializations --*/
  pipeline_arrays_alloc(d, gg);
  for (i = 0; i < d->nrows; i++) {
    d->sampled.els[i] = true;
  }
  /*-- maybe some points are tagged "hidden" in the data --*/
  rows_in_plot_set(d, gg);

  /*-- some initializations --*/
  viewmodes_init(d, gg);

  /*-- run the first half of the pipeline --*/
  arrayf_copy(&d->raw, &d->tform);

  limits_set(true, true, d, gg);
  vartable_limits_set(d);   /*-- does this do something here?  --*/
  vartable_stats_set(d);   /*-- does this do something here?  --*/

  tform_to_world(d, gg);
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
  make_ui(gg);

  /* If the user specified a data file on the command line, then 
     try to load that. If not, then look through the input plugins
     for the first interactive one (i.e. with an interactive="true" attribute)
     and then we try to run that. This allows input plugins that provide
     a user interface to query the user as to what to do.
   */
  if (options->data_in != NULL) {
    if (fileset_read(options->data_in, gg) > 0) {
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

  start_ggobi(gg, init_data,
              sessionOptions->info->createInitialScatterPlot);

  if (sessionOptions->restoreFile) {
    processRestoreFile(sessionOptions->restoreFile, gg);
  }

  if (processEvents) {
    gtk_main();
  }
}

void start_ggobi(ggobid * gg, gboolean init_data, gboolean createPlot)
{
  datad *d;
  if (init_data) {
    GSList *l;
    gboolean firstd = createPlot;
    for (l = gg->d; l; l = l->next) {
      d = (datad *) l->data;
      datad_init(d, gg, firstd);
      firstd = false;
    }

    /*-- destroy and rebuild the menu every time data is read in --*/
    display_menu_build(gg);
  }

  /*-- now that we've read some data, set the mode --*/
  if (createPlot && gg->d) {
    d = (datad *) gg->d->data;
    if (d != NULL) {
      if (d->ncols > 0) {
        gg->viewmode = (d->ncols == 1) ? P1PLOT : XYPLOT;
      }
    }
  } else
    gg->viewmode = NULLMODE;

  gg->prev_viewmode = gg->projection = gg->prev_projection = gg->viewmode;
        /*-- initialize the mode menus for the new mode --*/
  viewmode_submenus_update(NULLMODE, (displayd *) NULL, gg);
}
