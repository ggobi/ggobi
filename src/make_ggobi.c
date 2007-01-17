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
#include "plugin.h"

#include "ggobi-stage-subset.h"
#include "ggobi-stage-transform.h"
#include "ggobi-input-source-factory.h"

#ifdef USE_MYSQL
#include "read_mysql.h"
#endif

guint GGobiSignals[MAX_GGOBI_SIGNALS];

static void
pipeline_create_cb(GGobiPipelineFactory *factory, GGobiStage *root, GGobiSession *gg)
{
  GObject *subset, *filter, *domain_adj, *transform;
  
  subset = g_object_new(GGOBI_TYPE_STAGE_SUBSET, 
    "name", GGOBI_MAIN_STAGE_SUBSET, "parent", root, NULL);
  
  /* Note: there is no way to control the order of property settings with
     g_object_new, so we have to set the filter col here so that it
     comes after the parent */
  ggobi_stage_filter_set_filter_column(GGOBI_STAGE_FILTER(subset), 
    ggobi_stage_get_col_index_for_name(root, "_sampled"));
  
  // FIXME: 'excluded' is actually 'included' now
  filter = g_object_new(GGOBI_TYPE_STAGE_FILTER, 
    "name", GGOBI_MAIN_STAGE_FILTER, "parent", subset, NULL);
  
  ggobi_stage_filter_set_filter_column(GGOBI_STAGE_FILTER(filter),
    ggobi_stage_get_col_index_for_name(root, "_excluded"));
  
  domain_adj = g_object_new(GGOBI_TYPE_STAGE_TRANSFORM,
    "name", GGOBI_MAIN_STAGE_DOMAIN_ADJ, "parent", filter, NULL);
  transform = g_object_new(GGOBI_TYPE_STAGE_TRANSFORM,
    "name", GGOBI_MAIN_STAGE_TRANSFORM, "parent", domain_adj, NULL);
  
  // FIXME: get rid of these lines ASAP
  // There is absolutely no reason for the pipeline to depend on GGobiSession
  GGOBI_STAGE(subset)->gg = gg;
  GGOBI_STAGE(filter)->gg = gg;
  GGOBI_STAGE(domain_adj)->gg = gg;
  GGOBI_STAGE(transform)->gg = gg;
  
  g_object_unref(subset);
  g_object_unref(filter);
  g_object_unref(domain_adj);
  g_object_unref(transform);
}

GGobiPipelineFactory *
ggobi_create_pipeline_factory(GGobiSession *gg)
{
  GObject *factory = ggobi_pipeline_factory_new();
  g_signal_connect(factory, "build", G_CALLBACK(pipeline_create_cb), gg);
  return(GGOBI_PIPELINE_FACTORY(factory));
}

/*-- initialize variables which don't depend on the size of the data --*/
void
globals_init (GGobiSession * gg)
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

  gg->statusbar_p = true;
  
  gg->pipeline_factory = ggobi_create_pipeline_factory(gg);
}


GSList *
load_data (const gchar * uri, const gchar * mode_name, GGobiSession * gg)
{
  GGobiInputSource *source = create_input_source(uri, mode_name);
  GSList *ds = NULL;
  if (source) { // FIXME: report this error in gg->io_context
    ds = load_data_source(source, gg);
    g_object_unref(G_OBJECT(source));
  }
  return (ds);
}

// returns a list of datasets (some input types (eg. xml) may return 
// multiple data types)
GSList *
load_data_source (GGobiInputSource *source, GGobiSession * gg)
{
  GGobiDataFactory *factory;
  GSList *datasets = NULL;
  
  factory = create_data_factory(gg, source);
  if (factory == NULL) {
    // FIXME: we should have some unified way of graphically reporting errors
    // from some sort of IO context
    g_critical("No data factory capable of parsing the data");
    return NULL;
  }
  
  datasets = ggobi_data_factory_create(factory, source);
  for (; datasets; datasets = datasets->next) {
    GGobiStage *dataset = GGOBI_STAGE(datasets->data);
    /* hack: if there are no variables in the dataset (ie an edge set)
       at least make sure there are attributes.
       This will go away once we move to just storing attributes as variables */
    if (!dataset->n_cols)
      ggobi_data_add_attributes (GGOBI_DATA (dataset));
    ggobi_pipeline_factory_build(gg->pipeline_factory, dataset);
    /* eventually ggobi_stage_attach will happen implicitly when the 
       dataset is added to the main context. Right now we are sort of hacking
       it by attaching the transform stage rather than the dataset. The _attach()
       method knows when to go back to the root. */
    ggobi_stage_attach(ggobi_stage_find(dataset, GGOBI_MAIN_STAGE_TRANSFORM), gg, FALSE);
  }
  
  return (datasets);
}

GGobiDataFactory *
create_data_factory (GGobiSession *gg, GGobiInputSource *source)
{
  GType *factories;
  GGobiDataFactory *factory = NULL;
  guint i, n_factories;

  // FIXME: it may be better to make our own registry for GGobiDataFactories
  // that holds an instance, so that we aren't always instantiating them
  factories = g_type_children(GGOBI_TYPE_DATA_FACTORY, &n_factories);
  
  for (i = 0; i < n_factories && !factory; i++) {
    GObject *factory_obj = g_object_new(factories[i], NULL);
    if (ggobi_data_factory_supports_source(GGOBI_DATA_FACTORY(factory_obj), source))
      factory = GGOBI_DATA_FACTORY(factory_obj);
    else g_object_unref(factory_obj);
  }

  g_free(factories);
  
  return (factory);
}

#include <libxml/uri.h>

static gint
scheme_compare_func(gconstpointer list_scheme, gconstpointer scheme)
{
  return (list_scheme || scheme) && (!list_scheme || !scheme ||
    g_ascii_strcasecmp(list_scheme, scheme));
}

GGobiInputSource *
create_input_source(const gchar *uri, const gchar *mode)
{
  // FIXME: eventually we should have a registry of factory instances
  GGobiInputSourceFactory *factory = g_object_new(GGOBI_TYPE_INPUT_SOURCE_FACTORY, NULL);
  GGobiInputSource *source = ggobi_input_source_factory_create(factory, uri, mode);
  g_object_unref(G_OBJECT(factory));
  return source;
}

/*
 * the first display is initialized in datad_attach, so turn on
 * event handlers there as well
*/
void
make_ggobi (GGobiOptions * options, gboolean processEvents, GGobiSession * gg)
{
  gboolean init_data = false;

  /*-- some initializations --*/
  gg->displays = NULL;

  globals_init (gg); /*-- variables that don't depend on the data --*/

  special_colors_init (gg);

  wvis_init (gg);
  svis_init (gg);
  make_ui (gg);

  /* If the user specified a data file on the command line, then 
     try to load that.
   */
  if (options->data_in || options->data_type) {
    if (load_data (options->data_in, options->data_type, gg)) {
      init_data = true;
    }
  }

  if (options->info != NULL)
    registerPlugins (gg, options->info->plugins);
  
  if (options->timingp) {
    // Initialize the time counter
    set_time(gg);
  }

  start_ggobi (gg, init_data, options->info->createInitialScatterPlot);

  if (options->restoreFile) {
    processRestoreFile (options->restoreFile, gg);
  }

  gg->status_message_func = gg_write_to_statusbar;

  if (options->timingp) {
    run_timing_tests (gg);
  }

  if (processEvents) {
    gtk_main ();
  }
}

// FIXME: when this refactored, we need to emit a signal on GGobi "start"
// to allow stuff like batch execution via plugins
void
start_ggobi (GGobiSession * gg, gboolean init_data, gboolean createPlot)
{
  GGobiStage *d;
  if (init_data) {
    GSList *l;
    gboolean firstd = createPlot;
    for (l = gg->d; l; l = l->next) {
      ggobi_stage_attach(l->data, gg, firstd);
      firstd = false;
    }

    /*-- destroy and rebuild the menu every time data is read in --*/
    display_menu_build (gg);
  }

  /*-- now that we've read some data, set the mode --*/
  if (createPlot && gg->d) {
    d = (GGobiStage *)gg->d->data;
    if (d != NULL && ggobi_stage_has_vars(d)) {
      gg->pmode = (d->n_cols == 1) ? P1PLOT : XYPLOT;
      gg->imode = DEFAULT_IMODE;
    }
  }
  else {
    gg->pmode = NULL_PMODE;
    gg->imode = NULL_IMODE;
  }

  gg->pmode_prev = gg->pmode;
  gg->imode_prev = gg->imode;
  /*-- initialize the mode menus for the new mode --*/
  /*main_miscmenus_update(NULL_PMODE, NULL_IMODE, (displayd *) NULL, gg); */
}
