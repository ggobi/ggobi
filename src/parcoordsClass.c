/* parcoordsClass.c */
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

#include "parcoordsClass.h"

#include <string.h>
#ifdef STORE_SESSION_ENABLED
#include "write_state.h"
#endif
#include "externs.h"
#include <gdk/gdkkeysyms.h>

static gboolean parcoordsKeyEventHandled (GtkWidget *, displayd *, splotd *,
                                          GdkEventKey *, ggobid *);

static gboolean
binningPermitted (displayd * dpy)
{
  /*
     cpaneld *cpanel = &dpy->cpanel;
     if (cpanel->br_point_targets == br_select)
     return(false);
   */
  return (!dpy->options.whiskers_show_p);
}

static void
splotAssignPointsToBins (GGobiData * d, splotd * sp, ggobid * gg)
{
  if (sp == gg->current_splot)
    assign_points_to_bins (d, sp, gg);
}

gint
splot1DVariablesGet (splotd * sp, gint * cols, GGobiData * d)
{
  cols[0] = sp->p1dvar;
  return (1);
}


static gboolean
cpanelSet (displayd * dpy, cpaneld * cpanel, ggobid * gg)
{
  GtkWidget *w;
  w = GGOBI_EXTENDED_DISPLAY (dpy)->cpanelWidget;
  if (!w) {
    GGOBI_EXTENDED_DISPLAY (dpy)->cpanelWidget = w =
      cpanel_parcoords_make (gg);
  }

  cpanel_parcoords_set (dpy, cpanel, w, gg);
  cpanel_brush_set (dpy, cpanel, gg);
  cpanel_identify_set (dpy, cpanel, gg);

  return (true);                /* XX */
}

/**
 Instance initialization file.
*/
void
parcoordsDisplayInit (parcoordsDisplayd * display)
{
  GGOBI_DISPLAY (display)->p1d_orientation = VERTICAL;
}

static gboolean
handlesInteraction (displayd * dpy, InteractionMode v)
{
  return (v == BRUSH || v == IDENT || v == DEFAULT_IMODE);
}

void
start_parcoords_drag (GtkWidget * src, GdkDragContext * ctxt,
                      GtkSelectionData * data, guint info, guint time,
                      gpointer udata)
{
  gtk_selection_data_set (data, data->target, 8, (guchar *) src,
                          sizeof (splotd *));
}

void
receive_parcoords_drag (GtkWidget * src, GdkDragContext * context, int x,
                        int y, const GtkSelectionData * data,
                        unsigned int info, unsigned int event_time,
                        gpointer * udata)
{
  splotd *to = GGOBI_SPLOT (src), *from, *sp;
  displayd *display;
  display = to->displayptr;
  GList *l;
  gint k;
  GList *ivars = NULL;

  from = GGOBI_SPLOT (gtk_drag_get_source_widget (context));

  if (from->displayptr != display) {
    gg_write_to_statusbar
      ("the source and destination of the parallel coordinate plots are not from the same display.\n",
       display->ggobi);
    return;
  }

  /* Gather a list of indices */
  l = display->splots;
  while (l) {
    sp = (splotd *) l->data;
    ivars = g_list_append (ivars, GINT_TO_POINTER (sp->p1dvar));
    l = l->next;
  }

  /* Find the index of the to element */
  k = g_list_index (ivars, GINT_TO_POINTER (to->p1dvar));
  /* Remove the from element */
  ivars = g_list_remove (ivars, GINT_TO_POINTER (from->p1dvar));
  /* Insert the from element in the position of the to element */
  ivars = g_list_insert (ivars, GINT_TO_POINTER (from->p1dvar), k);


  /* Assign them to the existing plots */
  k = 0;
  l = display->splots;
  while (l) {
    sp = (splotd *) l->data;
    sp->p1dvar = GPOINTER_TO_INT (g_list_nth_data (ivars, k));
    k++;
    l = l->next;
  }
  g_list_free (ivars);

  display_tailpipe (display, FULL, display->ggobi);
  varpanel_refresh (display, display->ggobi);
}

void
parcoordsPlotDragAndDropEnable (splotd * sp, gboolean active)
{
  static GtkTargetEntry target = { "text/plain", GTK_TARGET_SAME_APP, 1001 };
  if (active) {
    gtk_drag_source_set (GTK_WIDGET (sp), GDK_BUTTON1_MASK, &target, 1,
                         GDK_ACTION_COPY);
    g_signal_connect (G_OBJECT (sp), "drag_data_get",
                      G_CALLBACK (start_parcoords_drag), NULL);
    gtk_drag_dest_set (GTK_WIDGET (sp), GTK_DEST_DEFAULT_ALL /* DROP */ ,
                       &target, 1, GDK_ACTION_COPY /*MOVE*/);
    g_signal_connect (G_OBJECT (sp), "drag_data_received",
                      G_CALLBACK (receive_parcoords_drag), NULL);
  }
  else {
    g_signal_handlers_disconnect_by_func (G_OBJECT (sp),
                                          G_CALLBACK (start_parcoords_drag),
                                          NULL);
    g_signal_handlers_disconnect_by_func (G_OBJECT (sp),
                                          G_CALLBACK (receive_parcoords_drag),
                                          NULL);
    gtk_drag_source_unset (GTK_WIDGET (sp));
    gtk_drag_dest_unset (GTK_WIDGET (sp));
  }
}

void
parcoordsDragAndDropEnable (displayd * dsp, gboolean active)
{
  GList *l;
  for (l = dsp->splots; l; l = l->next) {
    splotd *sp = (splotd *) l->data;
    parcoordsPlotDragAndDropEnable (sp, active);
  }
}

gboolean
parcoordsEventHandlersToggle (displayd * dpy, splotd * sp, gboolean state,
                              ProjectionMode pmode, InteractionMode imode)
{
/* it's necessary to disable/enable so that duplicate handlers are not
   registered */
/* it's necessary to toggle all plots, because all plots need to be
   ready to receive */
/* would be better if there was some callback when the imode changed */
  parcoordsDragAndDropEnable (dpy, false);

  switch (imode) {
  case DEFAULT_IMODE:
    p1d_event_handlers_toggle (sp, state);
    parcoordsDragAndDropEnable (dpy, true);
    break;
  case BRUSH:
    brush_event_handlers_toggle (sp, state);
    break;
  case IDENT:
    identify_event_handlers_toggle (sp, state);
    break;
  default:
    break;
  }

  return (false);
}

static gboolean
parcoordsKeyEventHandled (GtkWidget * w, displayd * display, splotd * sp,
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


gchar *
treeLabel (splotd * splot, GGobiData * d, ggobid * gg)
{
  return(ggobi_data_get_col_name(d, splot->p1dvar));
}

static GdkSegment *
allocWhiskers (GdkSegment * whiskers, splotd * sp, gint nr, GGobiData * d)
{
  return ((GdkSegment *) g_realloc (whiskers, 2 * nr * sizeof (GdkSegment)));
}

void
worldToPlane (splotd * sp, GGobiData * d, ggobid * gg)
{
  p1d_reproject (sp, d->world.vals, d, gg);
}

void
withinPlaneToScreen (splotd * sp, displayd * display, GGobiData * d,
                     ggobid * gg)
{
  sp_whiskers_make (sp, display, gg);
}

gboolean
drawEdge_p (splotd * sp, gint m, GGobiData * d, GGobiData * e, ggobid * gg)
{
  return (!ggobi_data_is_missing(e, m, sp->p1dvar));
}

gboolean
drawCase_p (splotd * sp, gint m, GGobiData * d, ggobid * gg)
{
  return (!ggobi_data_is_missing(d, m, sp->p1dvar));
}

void
withinDrawBinned (splotd * sp, gint m, GdkDrawable * drawable, GdkGC * gc)
{
  displayd *display = sp->displayptr;
  GGobiData *d = display->d;
  ggobid *gg = GGobiFromSPlot (sp);
  gint n, lwidth, ltype, gtype;

  if (!gg || !display)
    return;

  if (display->options.whiskers_show_p) {
    n = 2 * m;
    lwidth = lwidth_from_gsize (d->glyph_now.els[m].size);
    gtype = d->glyph_now.els[m].type;
    ltype = set_lattribute_from_ltype (ltype_from_gtype (gtype), gg);
    gdk_gc_set_line_attributes (gg->plot_GC, lwidth,
                                ltype, GDK_CAP_BUTT, GDK_JOIN_ROUND);
    gdk_draw_line (drawable, gc,
                   sp->whiskers[n].x1, sp->whiskers[n].y1,
                   sp->whiskers[n].x2, sp->whiskers[n].y2);
    n++;
    gdk_draw_line (drawable, gc,
                   sp->whiskers[n].x1, sp->whiskers[n].y1,
                   sp->whiskers[n].x2, sp->whiskers[n].y2);
  }
  gdk_gc_set_line_attributes (gg->plot_GC,
                              0, GDK_LINE_SOLID, GDK_CAP_ROUND,
                              GDK_JOIN_ROUND);
}

/* I think these two routines are identical ... */
void
withinDrawUnbinned (splotd * sp, gint m, GdkDrawable * drawable, GdkGC * gc)
{
  displayd *display = sp->displayptr;
  GGobiData *d = display->d;
  ggobid *gg = GGobiFromSPlot (sp);
  gint n, lwidth, ltype, gtype;

  if (!gg || !display)
    return;

  if (display->options.whiskers_show_p) {
    n = 2 * m;
    lwidth = lwidth_from_gsize (d->glyph_now.els[m].size);
    gtype = d->glyph_now.els[m].type;
    ltype = set_lattribute_from_ltype (ltype_from_gtype (gtype), gg);
    gdk_gc_set_line_attributes (gg->plot_GC, lwidth,
                                ltype, GDK_CAP_BUTT, GDK_JOIN_ROUND);
    gdk_draw_line (drawable, gc,
                   sp->whiskers[n].x1, sp->whiskers[n].y1,
                   sp->whiskers[n].x2, sp->whiskers[n].y2);
    n++;
    gdk_draw_line (drawable, gc,
                   sp->whiskers[n].x1, sp->whiskers[n].y1,
                   sp->whiskers[n].x2, sp->whiskers[n].y2);
  }
  gdk_gc_set_line_attributes (gg->plot_GC,
                              0, GDK_LINE_SOLID, GDK_CAP_ROUND,
                              GDK_JOIN_ROUND);
}


static void
addPlotLabels (displayd * display, splotd * sp, GdkDrawable * drawable,
               GGobiData * d, ggobid * gg)
{
  PangoRectangle rect;
  PangoLayout *layout =
    gtk_widget_create_pango_layout (GTK_WIDGET (sp->da), NULL);
  cpaneld *cpanel = &display->cpanel;


  layout_text (layout, ggobi_data_get_transformed_col_name(d, sp->p1dvar), &rect);
  if (cpanel->parcoords_arrangement == ARRANGE_ROW)
    gdk_draw_layout (drawable, gg->plot_GC,
                     (rect.width <=
                      sp->max.x) ? sp->max.x / 2 - rect.width / 2 : 0,
                     sp->max.y - rect.height - 5, layout);

  else
    gdk_draw_layout (drawable, gg->plot_GC, 5, 5, layout);

  g_object_unref (G_OBJECT (layout));
}


static gint
plotted (displayd * display, gint * cols, gint ncols, GGobiData * d)
{
  GList *l;
  splotd *sp;
  gint j;
  for (l = display->splots; l; l = l->next) {
    sp = (splotd *) l->data;

    for (j = 0; j < ncols; j++) {
      if (sp->xyvars.x == cols[j]) {
        return (sp->xyvars.x);
      }
    }
  }
  return (-1);
}

static gboolean
variableSelect (GtkWidget * w, displayd * dpy, splotd * sp, gint jvar,
                gint toggle, gint mouse, cpaneld * cpanel, ggobid * gg)
{
  gint jvar_prev = -1;
  return (parcoords_varsel (cpanel, sp, jvar, &jvar_prev, gg));
}


static void
varpanelRefresh (displayd * display, splotd * sp, GGobiData * d)
{
  gint j;
  GList *l;
  for (j = 0; j < d->ncols; j++) {
    varpanel_toggle_set_active (VARSEL_X, j, false, d);

    varpanel_toggle_set_active (VARSEL_Y, j, false, d);
    varpanel_widget_set_visible (VARSEL_Y, j, false, d);
    varpanel_toggle_set_active (VARSEL_Z, j, false, d);
    varpanel_widget_set_visible (VARSEL_Z, j, false, d);
  }

  l = display->splots;
  while (l) {
    j = ((splotd *) l->data)->p1dvar;
    varpanel_toggle_set_active (VARSEL_X, j, true, d);
    l = l->next;
  }
}

static void
varpanelTooltipsSet (displayd * dpy, ggobid * gg, GtkWidget * wx,
                     GtkWidget * wy, GtkWidget * xz, GtkWidget * label)
{
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wx,
                        "Toggle to append or delete; drag plots to reorder",
                        NULL);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), label,
                        "Toggle to append or delete; drag plots to reorder",
                        NULL);
}

/* Are these ordered?  Maybe so */
static gint
plottedVarsGet (displayd * display, gint * cols, GGobiData * d, ggobid * gg)
{
  GList *l;
  splotd *s;
  gint ncols = 0;
  for (l = display->splots; l; l = l->next) {
    s = (splotd *) l->data;
    if (!array_contains (cols, ncols, s->p1dvar))
      cols[ncols++] = s->p1dvar;
  }
  return (ncols);
}

static void
displaySet (displayd * display, ggobid * gg)
{
}

#ifdef STORE_SESSION_ENABLED

/*
  Write out the variables in a parallel coordinates plot
  to the current node in the  XML tree.

Should be able to make this generic.
 */
static void
add_xml_parcoords_variables (xmlNodePtr node, GList * plots, displayd * dpy)
{
  splotd *plot;
  while (plots) {
    plot = (splotd *) plots->data;
    XML_addVariable (node, plot->p1dvar, dpy->d);
    plots = plots->next;
  }
}
#endif

/*------------------------------------------------------------------------*/
/*               case highlighting for points (and edges?)                */
/*------------------------------------------------------------------------*/

/*-- add highlighting for parallel coordinates plot --*/
static void
splot_add_whisker_cues (gboolean nearest_p, gint k, splotd * sp,
                        GdkDrawable * drawable, ggobid * gg)
{
  gint n;
  displayd *display = sp->displayptr;
  GGobiData *d = display->d;
  colorschemed *scheme = gg->activeColorScheme;

  if (k < 0 || k >= d->nrows)
    return;

  if (display->options.whiskers_show_p) {
    gdk_gc_set_line_attributes (gg->plot_GC,
                                3, GDK_LINE_SOLID, GDK_CAP_ROUND,
                                GDK_JOIN_ROUND);
    gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb[d->color_now.els[k]]);

    n = 2 * k;
    gdk_draw_line (drawable, gg->plot_GC,
                   sp->whiskers[n].x1, sp->whiskers[n].y1,
                   sp->whiskers[n].x2, sp->whiskers[n].y2);
    n++;
    gdk_draw_line (drawable, gg->plot_GC,
                   sp->whiskers[n].x1, sp->whiskers[n].y1,
                   sp->whiskers[n].x2, sp->whiskers[n].y2);

    gdk_gc_set_line_attributes (gg->plot_GC,
                                0, GDK_LINE_SOLID, GDK_CAP_ROUND,
                                GDK_JOIN_ROUND);
  }

  if (nearest_p) {
    /* Add the label for the nearest point at the top as well */
    gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
    splot_add_point_label (true, k, true, sp, drawable, gg);
  }
}


static GtkWidget *
parcoordsCPanelWidget (displayd * dpy, gchar ** modeName, ggobid * gg)
{
  GtkWidget *w = GGOBI_EXTENDED_DISPLAY (dpy)->cpanelWidget;
  if (!w) {
    GGOBI_EXTENDED_DISPLAY (dpy)->cpanelWidget = w =
      cpanel_parcoords_make (gg);
  }
  *modeName = "Parcoords";
  return (w);
}

static void
splotScreenToTform (cpaneld * cpanel, splotd * sp, icoords * scr,
                    fcoords * tfd, ggobid * gg)
{
  gcoords planar, world;
  greal precis = (greal) PRECISION1;
  greal ftmp, max, min, rdiff;
  displayd *display = (displayd *) sp->displayptr;
  GGobiData *d = display->d;
  gfloat scale_x, scale_y;

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
 * plane to tform
*/

  if (sp->p1dvar != -1) {

    max = ggobi_data_get_col_max(d, sp->p1dvar);
    min = ggobi_data_get_col_min(d, sp->p1dvar);
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
  }

}

void
parcoordsDisplayClassInit (GGobiParCoordsDisplayClass * klass)
{
  klass->parent_class.loop_over_points = true;
  klass->parent_class.binningPermitted = binningPermitted;

  klass->parent_class.allow_reorientation = true;
  klass->parent_class.options_menu_p = true;

  klass->parent_class.loop_over_points = true;
  klass->parent_class.titleLabel = "Parallel Coordinates Display";
  klass->parent_class.treeLabel = "Parallel Coordinates";

  /* No create method, just createWithVars. */
  klass->parent_class.createWithVars = parcoords_new_with_vars;

  klass->parent_class.variable_select = variableSelect;
  klass->parent_class.variable_plotted_p = plotted;

  /* no unset */
  klass->parent_class.display_set = displaySet;

  klass->parent_class.mode_ui_get = parcoords_mode_ui_get;

  /* no build_symbol_vectors */

  /* ruler ranges set. */

  klass->parent_class.varpanel_refresh = varpanelRefresh;

  klass->parent_class.handles_interaction = handlesInteraction;

  klass->parent_class.cpanel_set = cpanelSet;

  #ifdef STORE_SESSION_ENABLED
  klass->parent_class.xml_describe = add_xml_parcoords_variables;
  #endif

  klass->parent_class.varpanel_tooltips_set = varpanelTooltipsSet;

  klass->parent_class.plotted_vars_get = plottedVarsGet;

  klass->parent_class.imode_control_box = parcoordsCPanelWidget;

/* menus_make */

  klass->parent_class.event_handlers_toggle = parcoordsEventHandlersToggle;
  klass->parent_class.splot_key_event_handled = parcoordsKeyEventHandled;

  klass->parent_class.add_plot_labels = addPlotLabels;
}


void
parcoordsSPlotClassInit (GGobiParCoordsSPlotClass * klass)
{
  klass->parent_class.alloc_whiskers = allocWhiskers;
  klass->parent_class.add_identify_cues = splot_add_whisker_cues;

  klass->parent_class.tree_label = treeLabel;

  /* reverse pipeline */
  klass->parent_class.screen_to_tform = splotScreenToTform;
  klass->parent_class.world_to_plane = worldToPlane;

  klass->parent_class.sub_plane_to_screen = withinPlaneToScreen;

  klass->parent_class.draw_edge_p = drawEdge_p;
  klass->parent_class.draw_case_p = drawCase_p;

  klass->parent_class.within_draw_to_binned = withinDrawBinned;
  klass->parent_class.within_draw_to_unbinned = withinDrawUnbinned;

  klass->parent_class.splot_assign_points_to_bins = splotAssignPointsToBins;

  klass->parent_class.plotted_vars_get = splot1DVariablesGet;
}


splotd *
ggobi_parcoords_splot_new (displayd * dpy, ggobid * gg)
{
  splotd *sp = g_object_new (GGOBI_TYPE_PAR_COORDS_SPLOT, NULL);
  splot_init (sp, dpy, gg);
  return (sp);
}
