/* tsPlot.c */
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
 *
 * Contributing author of time series code:  Nicholas Lewin-Koh
*/


#include "ggobi.h"
#include "tsdisplay.h"

#include <string.h>
#include "externs.h"


static void tsWorldToPlane(splotd *sp, GGobiData *d, ggobid *gg);
static void tsWithinPlaneToScreen(splotd *sp, displayd *display, GGobiData *d, ggobid *gg);
static gboolean tsDrawEdge_p(splotd *sp, gint m, GGobiData *d, GGobiData *e, ggobid *gg);
static gboolean tsDrawCase_p(splotd *sp, gint m, GGobiData *d, ggobid *gg);
static void tsAddPlotLabels(splotd *sp, GdkDrawable *drawable, ggobid *gg) ;
static void tsWithinDrawBinned(splotd *sp, gint m, GdkDrawable *drawable, GdkGC *gc);
static void tsShowWhiskers(splotd *sp, gint m, GdkDrawable *drawable, GdkGC *gc);
static GdkSegment * tsAllocWhiskers(GdkSegment *, splotd *sp, gint nrows, GGobiData *d);
static gchar *tsTreeLabel(splotd *sp, GGobiData *d, ggobid *gg);



void
tsWorldToPlane(splotd *sp,  GGobiData *d, ggobid *gg)
{
      xy_reproject (sp, d->world.vals, d, gg);
}

splotd *
ggobi_time_series_splot_new(displayd *dpy, ggobid *gg)
{
  timeSeriesSPlotd *bsp;
  splotd *sp;

  bsp = g_object_new(GGOBI_TYPE_TIME_SERIES_SPLOT, NULL);
  sp = GGOBI_SPLOT(bsp);


  splot_init(sp, dpy, gg);

  return(sp);
}

void
tsDestroy(splotd *sp)
{
      g_free ((gpointer) sp->whiskers);
}

void
tsWithinPlaneToScreen(splotd *sp, displayd *display, GGobiData *d, ggobid *gg)
{
      tsplot_whiskers_make (sp, display, gg);
}

gboolean
tsDrawEdge_p(splotd *sp, gint m, GGobiData *d, GGobiData *e, ggobid *gg)
{
   gboolean draw_edge = true;

   draw_edge = !(ggobi_data_is_missing(e, m, sp->xyvars.y) ||  ggobi_data_is_missing(e, m, sp->xyvars.x));
   return(draw_edge);
}

gboolean
tsDrawCase_p(splotd *sp, gint m, GGobiData *d, ggobid *gg)
{
  return !(ggobi_data_is_missing(d, m, sp->xyvars.y) || ggobi_data_is_missing(d, m, sp->xyvars.x));
}

void
tsAddPlotLabels(splotd *sp, GdkDrawable *drawable, ggobid *gg) 
{
  displayd *display = sp->displayptr;
  GList *l = display->splots;
  PangoLayout *layout = gtk_widget_create_pango_layout(sp->da, NULL);
  PangoRectangle rect;

  if (l->data == sp) {
    layout_text(layout, ggobi_data_get_transformed_col_name(display->d, sp->xyvars.x), &rect);
      gdk_draw_layout(drawable, gg->plot_GC, 
      sp->max.x - rect.width - 5,
      sp->max.y - rect.height - 5,
      layout
    );
  }
  layout_text(layout, ggobi_data_get_transformed_col_name(display->d, sp->xyvars.y), &rect);
  gdk_draw_layout(drawable, gg->plot_GC, 5, 5, layout);
  g_object_unref(G_OBJECT(layout));
}

void
tsWithinDrawBinned(splotd *sp, gint m, GdkDrawable *drawable, GdkGC *gc)
{
  gdk_draw_line (drawable, gc,
    sp->whiskers[m].x1, sp->whiskers[m].y1,
    sp->whiskers[m].x2, sp->whiskers[m].y2);
}


void
tsShowWhiskers(splotd *sp, gint m, GdkDrawable *drawable, GdkGC *gc)
{
  displayd *dpy = sp->displayptr;
     /*-- there are n-1 whiskers --*/
  if (dpy->options.whiskers_show_p && m < dpy->d->nrows_in_plot-1) 
     gdk_draw_line (drawable, gc,
       sp->whiskers[m].x1, sp->whiskers[m].y1,
       sp->whiskers[m].x2, sp->whiskers[m].y2);
}


GdkSegment * 
tsAllocWhiskers(GdkSegment *whiskers, splotd *sp, gint nrows, GGobiData *d)
{
  return((GdkSegment *) g_realloc (whiskers, (nrows-1) * sizeof (GdkSegment)));
}

gchar *
tsTreeLabel(splotd *sp, GGobiData *d, ggobid *gg)
{
  return(ggobi_data_get_col_name(d, sp->xyvars.y));
}


static gint
splotVariablesGet(splotd *sp, gint *cols, GGobiData *d)
{
	cols[0] = sp->xyvars.x;
	cols[1] = sp->xyvars.y;
	return(2);
}

/*
XX Incomplete. Need to finish off the construction of splotd's directly
 from command line language rather than as part of the displayd.
*/
splotd *
tsplotCreateWithVars(displayd *display, gint *vars, gint nvar, ggobid *gg)
{
   splotd *sp;
   if(nvar < 1) {
      g_printerr("not enough variables specified to create time series plot\n");
      return(NULL);
   }

   sp = ggobi_time_series_splot_new(display, gg);
   if(nvar > 1) {
      sp->xyvars.y = vars[1];
      sp->xyvars.x = vars[0];
   } else {
      sp->xyvars.y = vars[0];
      sp->xyvars.x = 0;
   }

   return(sp);
}

static void
splotAssignPointsToBins(GGobiData *d, splotd *sp, ggobid *gg)
{
  assign_points_to_bins (d, sp, gg);
}

static void
splotScreenToTform(cpaneld *cpanel, splotd *sp, icoords *scr,
		   fcoords *tfd, ggobid *gg)
{
  gcoords planar, world;
  gdouble precis = (gdouble) PRECISION1;
  gdouble ftmp, max, min, rdiff;
  displayd *display = (displayd *) sp->displayptr;
  GGobiData *d = display->d;
  gdouble scale_x, scale_y;
  vartabled *vtx, *vty;

  scale_x = sp->scale.x;
  scale_y = sp->scale.y;
  scale_x /= 2;
  sp->iscale.x = (gdouble) sp->max.x * scale_x;
  scale_y /= 2;
  sp->iscale.y = -1 * (gdouble) sp->max.y * scale_y;

/*
 * screen to plane 
*/
  planar.x = (scr->x - sp->max.x/2) * precis / sp->iscale.x ;
  planar.x += sp->pmid.x;
  planar.y = (scr->y - sp->max.y/2) * precis / sp->iscale.y ;
  planar.y += sp->pmid.y;

/*
 * plane to tform
*/
  /* x */
  vtx = vartable_element_get (sp->xyvars.x, d);
  max = vtx->lim.max;
  min = vtx->lim.min;
  rdiff = max - min;
  world.x = planar.x;
  ftmp = world.x / precis;
  tfd->x = (ftmp + 1.0) * .5 * rdiff;
  tfd->x += min;

  /* y */
  vty = vartable_element_get (sp->xyvars.y, d);
  max = vty->lim.max;
  min = vty->lim.min;
  rdiff = max - min;
  world.y = planar.y;
  ftmp = world.y / precis;
  tfd->y = (ftmp + 1.0) * .5 * rdiff;
  tfd->y += min;
}

void 
timeSeriesSPlotClassInit(GGobiTimeSeriesSPlotClass *klass)
{
    klass->extendedSPlotClass.splot.redraw = QUICK;
    klass->extendedSPlotClass.tree_label = tsTreeLabel;

    klass->extendedSPlotClass.within_draw_to_binned = tsWithinDrawBinned;
    klass->extendedSPlotClass.within_draw_to_unbinned = tsShowWhiskers;

    klass->extendedSPlotClass.draw_edge_p = tsDrawEdge_p;
    klass->extendedSPlotClass.draw_case_p = tsDrawCase_p;

    klass->extendedSPlotClass.add_plot_labels = tsAddPlotLabels;

    klass->extendedSPlotClass.sub_plane_to_screen = tsWithinPlaneToScreen;
    klass->extendedSPlotClass.alloc_whiskers = tsAllocWhiskers;

    /* reverse pipeline */ 
    klass->extendedSPlotClass.screen_to_tform = splotScreenToTform;

    klass->extendedSPlotClass.world_to_plane = tsWorldToPlane;

    klass->extendedSPlotClass.plotted_vars_get = splotVariablesGet;

    klass->extendedSPlotClass.createWithVars = tsplotCreateWithVars;
    klass->extendedSPlotClass.splot_assign_points_to_bins = splotAssignPointsToBins;
}


void 
timeSeriesClassInit(GGobiTimeSeriesDisplayClass *klass)
{
    klass->parent_class.binning_ok = false;

    klass->parent_class.treeLabel =  klass->parent_class.titleLabel = "Time Series";
    klass->parent_class.create = timeSeriesDisplayCreate;
    klass->parent_class.createWithVars = tsplot_new_with_vars;
    klass->parent_class.variable_select = tsplot_varsel;
    klass->parent_class.variable_plotted_p = tsplotIsVarPlotted;
    klass->parent_class.cpanel_set = tsplotCPanelSet;
    klass->parent_class.display_unset = NULL;
    klass->parent_class.display_set = tsplotDisplaySet;
    klass->parent_class.mode_ui_get = tsplot_mode_ui_get;
    klass->parent_class.varpanel_refresh = tsplotVarpanelRefresh;

    klass->parent_class.handles_interaction = tsplotHandlesInteraction;

    #ifdef STORE_SESSION_ENABLED
    klass->parent_class.xml_describe = add_xml_tsplot_variables;
    #endif

    klass->parent_class.varpanel_tooltips_set = tsplotVarpanelTooltipsSet;
    klass->parent_class.plotted_vars_get = tsplotPlottedColsGet;


    klass->parent_class.imode_control_box = tsplotCPanelWidget;
    //klass->parent_class.menus_make = tsplotMenusMake;

    klass->parent_class.event_handlers_toggle = tsplotEventHandlersToggle;

    klass->parent_class.splot_key_event_handled = tsplotKeyEventHandled;

    klass->parent_class.add_plot_labels = NULL; 

}
