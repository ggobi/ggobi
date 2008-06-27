/* barchartClass.c */
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
 * Contributing author of barchart and histogram code:  Heike Hofmann
*/



#include "ggobi.h"
#include "barchartDisplay.h"

#include <string.h>

#include "externs.h"
#include <gdk/gdkkeysyms.h>


 /* Making these available to ggobiClass.c */
static gboolean barchartVarSel (GtkWidget * w, displayd * display,
                                splotd * sp, gint jvar, gint toggle,
                                gint mouse, cpaneld * cpanel, ggobid * gg);
static gint barchartVarIsPlotted (displayd * dpy, gint * cols, gint ncols,
                                  GGobiData * d);
static gboolean barchartCPanelSet (displayd * dpy, cpaneld * cpanel,
                                   ggobid * gg);
static void barchartDisplaySet (displayd * dpy, ggobid * gg);
static void barchartDestroy (GtkObject *);
static void barchartPlaneToScreen (splotd * sp, GGobiData * d, ggobid * gg);
void barchart_clean_init (barchartSPlotd * sp);
void barchart_recalc_counts (barchartSPlotd * sp, GGobiData * d, ggobid * gg);

static gboolean barchart_build_symbol_vectors (cpaneld *, GGobiData *,
                                               ggobid *);
static void barchartVarpanelRefresh (displayd * display, splotd * sp,
                                     GGobiData * d);
static gboolean barchartHandlesInteraction (displayd * dpy, gint action);
static void barchartVarpanelTooltipsSet (displayd * dpy, ggobid * gg,
                                         GtkWidget * wx, GtkWidget * wy,
                                         GtkWidget * wz, GtkWidget * label);
static gint barchartPlottedColsGet (displayd * display, gint * cols,
                                    GGobiData * d, ggobid * gg);
static GtkWidget *barchartCPanelWidget (displayd * dpy,
                                        gchar ** modeName, ggobid * gg);
static gboolean barchartEventHandlersToggle (displayd * dpy, splotd * sp,
                                             gboolean state, ProjectionMode,
                                             InteractionMode);
static gboolean barchartKeyEventHandled (GtkWidget *, displayd *, splotd *,
                                         GdkEventKey *, ggobid *);
void barchartRulerRangesSet (gboolean, displayd *, splotd *, ggobid *);

static void
setShowAxesOption (displayd * display, gboolean active)
{

g_printerr("(setShowAxesOption) active %d\n", active);
  switch (display->cpanel.pmode) {
  case EXTENDED_DISPLAY_PMODE:
    scatterplot_show_vrule (display, active);
    break;
  default:
    break;
  }
}


static gboolean
varpanelHighd (displayd * display)
{
  gint proj = display->cpanel.pmode;
  return (proj == TOUR1D || proj == TOUR2D3 || proj == TOUR2D
          || proj == COTOUR);
}

static gint
barchart_is_variable_plotted (displayd * display, gint * cols, gint ncols,
                              GGobiData * d)
{
  gint j;
  ggobid *gg = display->d->gg;
  splotd *sp = gg->current_splot;
  gint jplotted = -1;
  for (j = 0; j < ncols; j++) {
    if (sp->p1dvar == cols[j]) {
      jplotted = sp->p1dvar;
      return jplotted;
    }
  }

  return (-1);
}


/* barchart splot methods*/
static gchar *
barchart_tree_label (splotd * sp, GGobiData * d, ggobid * gg)
{
  return(ggobi_data_get_col_name(d, sp->p1dvar));
}


gboolean
barchartVarSel (GtkWidget * w, displayd * display, splotd * sp, gint jvar,
                gint toggle, gint mouse, cpaneld * cpanel, ggobid * gg)
{
  gint jvar_prev = -1;
  gboolean redraw = false;
  GGobiData *d = display->d;

  switch (cpanel->pmode) {
    case TOUR1D:
      redraw = tour1d_varsel (w, jvar, toggle, mouse, d, gg);
    default:
      redraw = p1d_varsel(sp, jvar, &jvar_prev, toggle, mouse);
    break;
  }

  if (redraw) {
    barchart_clean_init (GGOBI_BARCHART_SPLOT (sp));
    barchart_recalc_counts (GGOBI_BARCHART_SPLOT (sp), d, d->gg);
  }

  return (true);
}

gint
barchartVarIsPlotted (displayd * dpy, gint * cols, gint ncols, GGobiData * d)
{
  int j;
  splotd *sp = (splotd *) dpy->splots->data;
  for (j = 0; j < ncols; j++) {
    if (sp->p1dvar == cols[j]) {
      return (sp->p1dvar);
    }
  }

  return (-1);
}

gboolean
barchartCPanelSet (displayd * dpy, cpaneld * cpanel, ggobid * gg)
{
  GtkWidget *w;
  w = GGOBI_EXTENDED_DISPLAY (dpy)->cpanelWidget;
  if (!w) {
    GGOBI_EXTENDED_DISPLAY (dpy)->cpanelWidget = w =
      cpanel_barchart_make (gg);
  }
  cpanel_barchart_set (dpy, cpanel, w, gg);
  cpanel_brush_set (dpy, cpanel, gg);
  cpanel_identify_set (dpy, cpanel, gg);

  return (true);
}

void
barchartDisplaySet (displayd * dpy, ggobid * gg)
{
}


/* This is for the barchart SPlot Class */
static void
barchartDestroy (GtkObject * obj)
{
  if (obj && GGOBI_BARCHART_SPLOT (obj)->bar) {
    GtkObjectClass *klass;
    barchartSPlotd *sp;

    sp = GGOBI_BARCHART_SPLOT (obj);

    /* Goal here is to get the class object for the parent
       of the GGOBI_TYPE_EXTENDED_SPLOT class so that we can call its
       destroy method.
       Need to get the class of the barchart and then constrain it to the 
       extended splot class. */
    klass = g_type_class_peek_parent (GGOBI_EXTENDED_SPLOT_GET_CLASS (sp));

    barchart_free_structure (sp);
    vectori_free (&sp->bar->index_to_rank);
    g_free ((gpointer) sp->bar);
    sp->bar = NULL;

    klass->destroy (GTK_OBJECT (sp));
  }
}


void
barchartPlaneToScreen (splotd * sp, GGobiData * d, ggobid * gg)
{
  barchartSPlotd *bsp = GGOBI_BARCHART_SPLOT (sp);

  barchart_recalc_dimensions (sp, d, gg);
  barchart_recalc_group_dimensions (bsp, gg);
}

void
barchartWorldToPlane (splotd * sp, GGobiData * d, ggobid * gg)
{
  barchartSPlotd *bsp = GGOBI_BARCHART_SPLOT (sp);

/* Commenting these two lines out to fix this bug:  After
   dragging to reset the number of bars, leaving the barplot
   interaction mode sets the number of bars.  Question:  Will
   this introduce new bugs????   dfs, June 2008
  barchart_clean_init(bsp);
  barchart_recalc_counts(bsp, d, gg);
*/
}


/*----------------------------------------------------------------------*/
/*      local helper function for barcharts,                            */
/*      called by build_symbol_vectors                                  */
/*----------------------------------------------------------------------*/

gboolean
barchart_build_symbol_vectors (cpaneld * cpanel, GGobiData * d, ggobid * gg)
{
  gboolean changed = FALSE;
  gint j, m;
  gint nd = g_slist_length (gg->d);

  for (j = 0; j < d->nrows_in_plot; j++) {
    m = d->rows_in_plot.els[j];

    switch (cpanel->br.point_targets) {
    case br_candg:  /*-- color and glyph --*/
      changed = update_color_vectors (m, changed,
                                      d->pts_under_brush.els, d, gg);
      changed = update_glyph_vectors (m, changed,
                                      d->pts_under_brush.els, d, gg);
      break;
    case br_color:
      changed = update_color_vectors (m, changed,
                                      d->pts_under_brush.els, d, gg);
      break;
    case br_glyph:  /*-- glyph type and size --*/
      changed = update_glyph_vectors (m, changed,
                                      d->pts_under_brush.els, d, gg);
      break;
    case br_shadow:
      changed = update_hidden_vectors (m, changed,
                                       d->pts_under_brush.els, d, gg);
      break;
    case br_unshadow:
      changed = bizarro_update_hidden_vectors (m, changed,
                                               d->pts_under_brush.els, d, gg);
      break;
    case br_off:
      ;
      break;
    }

    /*-- link by id --*/
    if (!gg->linkby_cv && nd > 1)
      symbol_link_by_id (false, j, d, gg);
    /*-- --*/
  }

  return changed;
}

void
barchartVarpanelRefresh (displayd * display, splotd * sp, GGobiData * d)
{
  cpaneld *cpanel = &display->cpanel;
  gint j;

  switch (cpanel->pmode) {
    case TOUR1D:
      for (j=0; j<d->ncols; j++) {
        varpanel_toggle_set_active (VARSEL_X, j, false, d);
        varpanel_toggle_set_active (VARSEL_Y, j, false, d);
        varpanel_widget_set_visible (VARSEL_Y, j, false, d);
        varpanel_toggle_set_active (VARSEL_Z, j, false, d);
        varpanel_widget_set_visible (VARSEL_Z, j, false, d);
      }
      for (j=0; j<display->t1d.nsubset; j++) {
        varpanel_toggle_set_active (VARSEL_X,
          display->t1d.subset_vars.els[j], true, d);
      }
      break;
    default:
      for (j = 0; j < d->ncols; j++) {
        varpanel_toggle_set_active(VARSEL_X, j, (j == sp->p1dvar), d);

        varpanel_toggle_set_active(VARSEL_Y, j, false, d);
        varpanel_widget_set_visible(VARSEL_Y, j, false, d);
        varpanel_toggle_set_active(VARSEL_Z, j, false, d);
        varpanel_widget_set_visible(VARSEL_Z, j, false, d);
      }
    break;
  }
}

gboolean
barchartHandlesInteraction (displayd * dpy, gint action)
{
  InteractionMode imode = (InteractionMode) action;
  return ( /*imode == SCALE || */ imode == BRUSH || imode == IDENT
          || imode == DEFAULT_IMODE);
}



/*--------------------------------------------------------------------*/
/*                      Barchart: Options menu                        */
/*--------------------------------------------------------------------*/

void
barchartVarpanelTooltipsSet (displayd * dpy, ggobid * gg, GtkWidget * wx,
                             GtkWidget * wy, GtkWidget * wz,
                             GtkWidget * label)
{
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wx,
                        "Click to replace a variable", NULL);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), label,
                        "Click to replace a variable", NULL);
}


gint
barchartPlottedColsGet (displayd * display, gint * cols, GGobiData * d,
                        ggobid * gg)
{
  gint ncols = 0;
  cols[ncols++] = gg->current_splot->p1dvar;
  return (ncols);
}

GtkWidget *
barchartCPanelWidget (displayd * dpy, gchar ** modeName, ggobid * gg)
{
  GtkWidget *w = GGOBI_EXTENDED_DISPLAY (dpy)->cpanelWidget;
  if (!w) {
    GGOBI_EXTENDED_DISPLAY (dpy)->cpanelWidget = w =
      cpanel_barchart_make (gg);
  }
  *modeName = "Bar Chart";
  return (w);
}

gboolean
barchartEventHandlersToggle (displayd * dpy, splotd * sp, gboolean state,
                             ProjectionMode pmode, InteractionMode imode)
{
  if (imode == IDENT) {
    identify_event_handlers_toggle (sp, state); // Generic method
  }
  else if (imode == BRUSH) {
    brush_event_handlers_toggle (sp, state);  // Generic method
  }
  else if (imode == DEFAULT_IMODE) {
    barchart_event_handlers_toggle (dpy, sp, state, pmode, imode);
  }

  return (false);
}


static gboolean
barchartKeyEventHandled (GtkWidget * w, displayd * display, splotd * sp,
                         GdkEventKey * event, ggobid * gg)
{
  gboolean ok = true;
  ProjectionMode pmode = NULL_PMODE;
  InteractionMode imode = DEFAULT_IMODE;

  if (event->state == 0 || event->state == GDK_CONTROL_MASK) {

    switch (event->keyval) {
    case GDK_h:
    case GDK_H:
      pmode = EXTENDED_DISPLAY_PMODE;
      break;
    case GDK_t:
    case GDK_T:
      pmode = TOUR1D;
      break;
    case GDK_b:
    case GDK_B:
      imode = BRUSH;
      break;
    case GDK_i:
    case GDK_I:
      imode = IDENT;
      break;

    default:
      ok = false;
      break;
    }

    if (ok) {
      GGOBI (full_viewmode_set) (pmode, imode, gg);
    }
  }
  else {
    ok = false;
  }

  return ok;
}

/* This is an abbreviated version of the scatterplot version */
void
barchartScreenToTform (cpaneld * cpanel, splotd * sp, icoords * scr,
                       fcoords * tfd, ggobid * gg)
{
  gcoords planar, world;
  greal precis = (greal) PRECISION1;
  greal ftmp, max, min, rdiff;
  displayd *display = (displayd *) sp->displayptr;
  GGobiData *d = display->d;
  gfloat scale_x, scale_y;
  vartabled *vt;

  scale_x = sp->scale.x;
  scale_y = sp->scale.y;
  scale_x /= 2;
  sp->iscale.x = (greal) sp->max.x * scale_x;
  scale_y /= 2;
  sp->iscale.y = -1 * (greal) sp->max.y * scale_y;

/*
 * screen to plane 
*/
  planar.x = (scr->x - sp->max.x / 2) * precis / sp->iscale.x;
  planar.x += sp->pmid.x;
  planar.y = (scr->y - sp->max.y / 2) * precis / sp->iscale.y;
  planar.y += sp->pmid.y;

/*
 * plane to world
*/

  switch (cpanel->pmode) {
  case TOUR1D:
    break;

    /* Sheesh, I haven't even decided what I want the pmode to be */
  case EXTENDED_DISPLAY_PMODE:
  case DEFAULT_PMODE:
    vt = vartable_element_get (sp->p1dvar, d);
    max = vt->lim.max;
    min = vt->lim.min;
    rdiff = max - min;

    if (display->p1d_orientation == HORIZONTAL) {
      /* x */
      world.x = planar.x;
      ftmp = world.x / precis;
      tfd->x = (ftmp + 1.0) * .5 * rdiff;
      tfd->x += min;
    }
    else {
      /* y */
      world.y = planar.y;
      ftmp = world.y / precis;
      tfd->y = (ftmp + 1.0) * .5 * rdiff;
      tfd->y += min;
    }
    break;

  default:
    break;
  }
}

void
barchartDisplayClassInit (GGobiBarChartDisplayClass * klass)
{
  klass->parent_class.treeLabel = klass->parent_class.titleLabel = "Barchart";
  klass->parent_class.create = barchart_new;
  klass->parent_class.createWithVars = barchart_new_with_vars;
  klass->parent_class.variable_select = barchartVarSel;
  klass->parent_class.variable_plotted_p = barchartVarIsPlotted;
  klass->parent_class.cpanel_set = barchartCPanelSet;
  klass->parent_class.display_unset = NULL;
  klass->parent_class.display_set = barchartDisplaySet;
  klass->parent_class.mode_ui_get = barchart_mode_ui_get;
  klass->parent_class.variable_plotted_p = barchart_is_variable_plotted;

  klass->parent_class.build_symbol_vectors = barchart_build_symbol_vectors;

/* Because of the kludgey implementation of the barchart class, I can't
   make this quit trying to set ruler ranges.  Instead, I'm going to
   create a function that's simply a noop.  dfs June 2008
*/
//  klass->parent_class.ruler_ranges_set = ruler_ranges_set;
  klass->parent_class.ruler_ranges_set = barchartRulerRangesSet;

  klass->parent_class.varpanel_highd = varpanelHighd;
  klass->parent_class.varpanel_refresh = barchartVarpanelRefresh;

  klass->parent_class.handles_interaction = barchartHandlesInteraction;

  klass->parent_class.xml_describe = NULL;

  klass->parent_class.varpanel_tooltips_set = barchartVarpanelTooltipsSet;

  klass->parent_class.plotted_vars_get = barchartPlottedColsGet;

  klass->parent_class.imode_control_box = barchartCPanelWidget;

  klass->parent_class.allow_reorientation = false;

  klass->parent_class.binning_ok = false;
  klass->parent_class.event_handlers_toggle = barchartEventHandlersToggle;
  klass->parent_class.splot_key_event_handled = barchartKeyEventHandled;

  klass->parent_class.set_show_axes_option = setShowAxesOption;
}

void
barchart_identify_cues_draw (gboolean nearest_p, gint k, splotd * rawsp,
                             GdkDrawable * drawable, ggobid * gg)
{
  barchartSPlotd *sp = GGOBI_BARCHART_SPLOT (rawsp);
  PangoLayout *layout =
    gtk_widget_create_pango_layout (GTK_WIDGET (rawsp->da), NULL);
  gint i, nbins;
  gchar *string;
  icoords mousepos = rawsp->mousepos;
  colorschemed *scheme = gg->activeColorScheme;
  gint level;
  gint j;

  nbins = sp->bar->nbins;
  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);

  if (sp->bar->low_pts_missing && sp->bar->bar_hit[0]) {
    string = g_strdup_printf ("%ld point%s < %.2f", sp->bar->low_bin->count,
                              sp->bar->low_bin->count == 1 ? "" : "s",
                              sp->bar->breaks[0] + sp->bar->offset);

    gdk_draw_rectangle (drawable, gg->plot_GC, FALSE,
                        sp->bar->low_bin->rect.x, sp->bar->low_bin->rect.y,
                        sp->bar->low_bin->rect.width,
                        sp->bar->low_bin->rect.height);
    layout_text (layout, string, NULL);
    gdk_draw_layout (drawable, gg->plot_GC, mousepos.x, mousepos.y, layout);
    g_free (string);
  }
  for (i = 1; i < nbins + 1; i++) {
    if (sp->bar->bar_hit[i]) {
      if (sp->bar->is_histogram) {
        string = g_strdup_printf ("%ld point%s in (%.2f,%.2f)",
                                  sp->bar->bins[i - 1].count,
                                  sp->bar->bins[i - 1].count == 1 ? "" : "s",
                                  sp->bar->breaks[i - 1] + sp->bar->offset,
                                  sp->bar->breaks[i] + sp->bar->offset);
      }
      else {
        gchar *levelName;
        vartabled *var;
        var = (vartabled *) g_slist_nth_data (rawsp->displayptr->d->vartable,
                                              rawsp->p1dvar);
/* dfs */
        j = i - 1;
        level = checkLevelValue (var, (gdouble) sp->bar->bins[j].value);

        if (level == -1) {
          string = g_strdup_printf ("%ld point%s missing",
                                    sp->bar->bins[j].count,
                                    sp->bar->bins[j].count == 1 ? "" : "s");
        }
        else {
          levelName = var->level_names[level];
          string = g_strdup_printf ("%ld point%s in %s",
                                    sp->bar->bins[j].count,
                                    sp->bar->bins[j].count == 1 ? "" : "s",
                                    levelName);
        }
/* --- */
#ifdef PREV
        levelName = var->level_names[i - 1];
        sprintf (string, "%ld point%s in %s",
                 sp->bar->bins[i - 1].count,
                 sp->bar->bins[i - 1].count == 1 ? "" : "s", levelName);
#endif
      }
      gdk_draw_rectangle (drawable, gg->plot_GC, FALSE,
                          sp->bar->bins[i - 1].rect.x,
                          sp->bar->bins[i - 1].rect.y,
                          sp->bar->bins[i - 1].rect.width,
                          sp->bar->bins[i - 1].rect.height);
      layout_text (layout, string, NULL);
      gdk_draw_layout (drawable, gg->plot_GC, mousepos.x, mousepos.y, layout);
      g_free (string);
    }
  }

  if (sp->bar->high_pts_missing && sp->bar->bar_hit[nbins + 1]) {
    string = g_strdup_printf ("%ld point%s > %.2f", sp->bar->high_bin->count,
                              sp->bar->high_bin->count == 1 ? "" : "s",
                              sp->bar->breaks[nbins] + sp->bar->offset);

    gdk_draw_rectangle (drawable, gg->plot_GC, FALSE,
                        sp->bar->high_bin->rect.x, sp->bar->high_bin->rect.y,
                        sp->bar->high_bin->rect.width,
                        sp->bar->high_bin->rect.height);
    layout_text (layout, string, NULL);
    gdk_draw_layout (drawable, gg->plot_GC, mousepos.x, mousepos.y, layout);
    g_free (string);
  }
  g_object_unref (G_OBJECT (layout));

}

void
barchartSPlotClassInit (GGobiBarChartSPlotClass * klass)
{
  /* barcharts need more attention than redrawing the brush */
  klass->extendedSPlotClass.splot.redraw = FULL;
  klass->extendedSPlotClass.tree_label = barchart_tree_label;

  klass->extendedSPlotClass.identify_notify = barchart_identify_bars;
  klass->extendedSPlotClass.add_markup_cues = barchart_add_bar_cues;
#if BARCHART_SCALE
  klass->extendedSPlotClass.add_scaling_cues =
    barchart_scaling_visual_cues_draw;
#endif
  klass->extendedSPlotClass.add_identify_cues = barchart_identify_cues_draw;
  klass->extendedSPlotClass.add_plot_labels = barchart_splot_add_plot_labels;
  klass->extendedSPlotClass.redraw = barchart_redraw;

  klass->extendedSPlotClass.screen_to_tform = barchartScreenToTform;
  klass->extendedSPlotClass.world_to_plane = barchartWorldToPlane;
  klass->extendedSPlotClass.plane_to_screen = barchartPlaneToScreen;

  klass->extendedSPlotClass.active_paint_points =
    barchart_active_paint_points;

  GTK_OBJECT_CLASS (klass)->destroy = barchartDestroy;

  klass->extendedSPlotClass.plotted_vars_get = splot1DVariablesGet;
}
