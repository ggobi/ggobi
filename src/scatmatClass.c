/* scatmatClass.c */
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

#include "scatmatClass.h"
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include <math.h>
#include <string.h>

#include "externs.h"

static gboolean
cpanelSet (displayd * dpy, cpaneld * cpanel, ggobid * gg)
{
  cpanel_scatmat_set (dpy, cpanel, gg);
  cpanel_brush_set (dpy, cpanel, gg);
  cpanel_identify_set (dpy, cpanel, gg);
  return (true);
}

static void
movePointsMotionCb (displayd * display, splotd * sp, GtkWidget * w,
                    GdkEventMotion * event, ggobid * gg)
{
  if (sp->p1dvar == -1)
    scatterplotMovePointsMotionCb (display, sp, w, event, gg);
}

static void
movePointsButtonCb (displayd * display, splotd * sp, GtkWidget * w,
                    GdkEventButton * event, ggobid * gg)
{
  if (sp->p1dvar == -1)
    scatterplotMovePointsButtonCb (display, sp, w, event, gg);
}

/* XXX duncan and dfs: you need to sort this out
static void
worldToRaw(displayd *display, splotd *sp, gint pt, GGobiData *d, ggobid *gg)
{
 if (sp->p1dvar == -1) {
    world_to_raw_by_var (pt, sp->xyvars.x, display, d, gg);
    world_to_raw_by_var (pt, sp->xyvars.y, display, d, gg);
  }
}
*/


static gint
variablePlottedP (displayd * display, gint * cols, gint ncols, GGobiData * d)
{
  GList *l;
  gint j;
  splotd *sp;
  for (l = display->splots; l; l = l->next) {
    sp = (splotd *) l->data;

    for (j = 0; j < ncols; j++) {
      if (sp->p1dvar == -1) {
        if (sp->xyvars.x == cols[j]) {
          return (sp->xyvars.x);
        }
        if (sp->xyvars.y == cols[j]) {
          return (sp->xyvars.y);
        }
      }
      else if (sp->p1dvar == cols[j]) {
        return (sp->p1dvar);
      }
    }
  }
  return (-1);
}

static gboolean
variableSelect (GtkWidget * w, displayd * dpy, splotd * sp, gint jvar,
                gint toggle, gint mouse, cpaneld * cpanel, ggobid * gg)
{
  gint jvar_prev;
  return (scatmat_varsel_simple (cpanel, sp, jvar, &jvar_prev, gg));
}

static void
varpanelRefresh (displayd * display, splotd * sp, GGobiData * d)
{
  gint j, n, *vars;
  ggobid *gg = GGobiFromDisplay (display);

  for (j = 0; j < d->ncols; j++) {
    varpanel_toggle_set_active (VARSEL_X, j, false, d);

    varpanel_toggle_set_active (VARSEL_Y, j, false, d);
    varpanel_widget_set_visible (VARSEL_Y, j, false, d);
    varpanel_toggle_set_active (VARSEL_Z, j, false, d);
    varpanel_widget_set_visible (VARSEL_Z, j, false, d);
  }

  vars = (gint *) g_malloc (d->ncols * sizeof (gint));
  n =
    GGOBI_EXTENDED_DISPLAY_GET_CLASS (display)->plotted_vars_get (display,
                                                                  vars, d,
                                                                  gg);

  for (j = 0; j < n; j++)
    varpanel_toggle_set_active (VARSEL_X, vars[j], true, d);

  g_free (vars);
}

static void
varpanelTooltipsSet (displayd * display, ggobid * gg, GtkWidget * wx,
                     GtkWidget * wy, GtkWidget * wz, GtkWidget * label)
{
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wx,
                        "Toggle to append or delete; drag along the plot diagonal to reorder",
                        NULL);
  gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), label,
                        "Toggle to append or delete; drag along the plot diagonal to reorder",
                        NULL);
}

static gint
plottedVarsGet (displayd * display, gint * vars, GGobiData * d, ggobid * gg)
{
  GList *l;
  GtkTableChild *child;
  GtkWidget *da;
  splotd *sp;
  gint nvars = 0;

  /* First count the number of variables */
  for (l = (GTK_TABLE (display->table))->children; l; l = l->next) {
    child = (GtkTableChild *) l->data;
    da = child->widget;
    sp = (splotd *) g_object_get_data (G_OBJECT (da), "splotd");
    if (sp->p1dvar != -1)
      nvars += 1;
  }

  /* Then populate the vector of variables */
  for (l = (GTK_TABLE (display->table))->children; l; l = l->next) {
    child = (GtkTableChild *) l->data;
    da = child->widget;
    sp = (splotd *) g_object_get_data (G_OBJECT (da), "splotd");
    if (sp->p1dvar != -1) {
      vars[child->left_attach] = sp->p1dvar;
    }
  }

  return nvars;
}

displayd *
createWithVars (gboolean missing_p, gint nvars, gint * vars, GGobiData * d,
                ggobid * gg)
{
  return (GGOBI (newScatmat) (vars, vars, nvars, nvars, d, gg));
}

#ifdef STORE_SESSION_ENABLED

void
add_xml_scatmat_variables (xmlNodePtr node, GList * plots, displayd * dpy)
{
  splotd *plot = plots->data;
  int n, n1, i;

  n1 = g_list_length (plots);
  n = sqrt (n1);

  for (i = 0; i < n1; i += n) {
    plot = (splotd *) g_list_nth_data (plots, i);
    XML_addVariable (node, plot->xyvars.x, dpy->d);
  }
}
#endif

static gboolean
scatmatKeyEventHandled (GtkWidget * w, displayd * display, splotd * sp,
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

    case GDK_s:
    case GDK_S:
      imode = SCALE;
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

static void
displaySet (displayd * display, ggobid * gg)
{
}

static gboolean
handlesInteraction (displayd * display, InteractionMode v)
{
  return (v == SCALE || v == BRUSH || v == IDENT || /*v == MOVEPTS || */
          v == DEFAULT_IMODE);
}

/*-------------------------------------------------------------------*/
/*--------------- Drag and Drop -------------------------------------*/
/*-------------------------------------------------------------------*/

void
start_scatmat_drag (GtkWidget * src, GdkDragContext * ctxt,
                    GtkSelectionData * data, guint info, guint time,
                    gpointer udata)
{
  gtk_selection_data_set (data, data->target, 8, (guchar *) src,
                          sizeof (splotd *));
}

void
receive_scatmat_drag (GtkWidget * src, GdkDragContext * context, int x, int y,
                      const GtkSelectionData * data, unsigned int info,
                      unsigned int event_time, gpointer * udata)
{
  splotd *to = GGOBI_SPLOT (src), *from, *sp;
  displayd *display;
  GList *l;
  gint k, n, sprow, spcol;
  GtkWidget *da;
  GtkTableChild *child;
  GList *ivars = NULL;
  gint nvars, *vars;
  GGobiData *d;
  ggobid *gg;

  display = to->displayptr;
  d = display->d;
  gg = GGobiFromDisplay (display);
  from = GGOBI_SPLOT (gtk_drag_get_source_widget (context));

  if (from->displayptr != display) {
    gg_write_to_statusbar
      ("the source and destination of the scatterplots are not from the same display.\n",
       display->ggobi);
    return;
  }

  /* Require symmetry, and require drag and drop motions in which
     either the row or the column is held constant -- actually, start
     by requiring that both plots be along the diagonal. */

  if (from->p1dvar != -1 && to->p1dvar != -1) {

    vars = (gint *) g_malloc (d->ncols * sizeof (gint));
    nvars =
      GGOBI_EXTENDED_DISPLAY_GET_CLASS (display)->plotted_vars_get (display,
                                                                    vars, d,
                                                                    gg);

    /* Easier to do this with a linked list, perhaps */
    for (n = 0; n < nvars; n++)
      ivars = g_list_append (ivars, GINT_TO_POINTER (vars[n]));
    /* Find the index of the to element */
    k = g_list_index (ivars, GINT_TO_POINTER (to->p1dvar));
    /* Remove the from element */
    ivars = g_list_remove (ivars, GINT_TO_POINTER (from->p1dvar));
    /* Insert the from element in the position of the to element */
    ivars = g_list_insert (ivars, GINT_TO_POINTER (from->p1dvar), k);

    /* Loop through the plots setting the values of xyvars and
       p1dvar */
    for (l = (GTK_TABLE (display->table))->children; l; l = l->next) {
      child = (GtkTableChild *) l->data;
      da = child->widget;
      sp = (splotd *) g_object_get_data (G_OBJECT (da), "splotd");
      sprow = child->top_attach;  /* 0, ..., nrows-1 */
      spcol = child->left_attach; /* 0, ..., ncols-1 */
      if (sprow == spcol) {
        sp->p1dvar = GPOINTER_TO_INT (g_list_nth_data (ivars, sprow));
      }
      else {
        sp->p1dvar = -1;
        sp->xyvars.x = GPOINTER_TO_INT (g_list_nth_data (ivars, spcol));
        sp->xyvars.y = GPOINTER_TO_INT (g_list_nth_data (ivars, sprow));
      }
    }

    display_tailpipe (display, FULL, display->ggobi);
    varpanel_refresh (display, display->ggobi);

    g_free (vars);
  }
}

void
scatmatPlotDragAndDropEnable (splotd * sp, gboolean active)
{
  static GtkTargetEntry target = { "text/plain", GTK_TARGET_SAME_APP, 1001 };
  if (active) {
    gtk_drag_source_set (GTK_WIDGET (sp), GDK_BUTTON1_MASK, &target, 1,
                         GDK_ACTION_COPY);
    g_signal_connect (G_OBJECT (sp), "drag_data_get",
                      G_CALLBACK (start_scatmat_drag), NULL);
    gtk_drag_dest_set (GTK_WIDGET (sp), GTK_DEST_DEFAULT_ALL /* DROP */ ,
                       &target, 1, GDK_ACTION_COPY /*MOVE*/);
    g_signal_connect (G_OBJECT (sp), "drag_data_received",
                      G_CALLBACK (receive_scatmat_drag), NULL);
  }
  else {
    g_signal_handlers_disconnect_by_func (G_OBJECT (sp),
                                          G_CALLBACK (start_scatmat_drag),
                                          NULL);
    g_signal_handlers_disconnect_by_func (G_OBJECT (sp),
                                          G_CALLBACK (receive_scatmat_drag),
                                          NULL);
    gtk_drag_source_unset (GTK_WIDGET (sp));
    gtk_drag_dest_unset (GTK_WIDGET (sp));
  }
}

void
scatmatDragAndDropEnable (displayd * dsp, gboolean active)
{
  GList *l;
  for (l = dsp->splots; l; l = l->next) {
    splotd *sp = (splotd *) l->data;
    if (sp->p1dvar != -1)
      scatmatPlotDragAndDropEnable (sp, active);
  }
}

gboolean
scatmatEventHandlersToggle (displayd * dpy, splotd * sp, gboolean state,
                            ProjectionMode pmode, InteractionMode imode)
{
  scatmatDragAndDropEnable (dpy, false);

  switch (imode) {
  case DEFAULT_IMODE:
    switch (sp->p1dvar) {
    case -1:
      xyplot_event_handlers_toggle (sp, state);
      break;
    default:
      p1d_event_handlers_toggle (sp, state);
    }
    scatmatDragAndDropEnable (dpy, true);
    break;
  case SCALE:
    scale_event_handlers_toggle (sp, state);
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



/*-------------------------------------------------------------------*/

static GtkWidget *
scatmatCPanelWidget (displayd * dpy, gchar ** modeName, ggobid * gg)
{
  GtkWidget *w = GGOBI_EXTENDED_DISPLAY (dpy)->cpanelWidget;
  if (!w) {
    GGOBI_EXTENDED_DISPLAY (dpy)->cpanelWidget = w = cpanel_scatmat_make (gg);
  }
  *modeName = "Scatterplot Matrix";
  return (w);
}

static void
splotAssignPointsToBins (GGobiData * d, splotd * sp, ggobid * gg)
{
  if (sp == gg->current_splot)
    assign_points_to_bins (d, sp, gg);
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
 * plane to world
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
  else {

    /* x */
    max = ggobi_data_get_col_max(d, sp->xyvars.x);
    min = ggobi_data_get_col_min(d, sp->xyvars.x);
    rdiff = max - min;
    world.x = planar.x;
    ftmp = world.x / precis;
    tfd->x = (ftmp + 1.0) * .5 * rdiff;
    tfd->x += min;

    /* y */
    max = ggobi_data_get_col_max(d, sp->xyvars.y);
    min = ggobi_data_get_col_min(d, sp->xyvars.y);
    rdiff = max - min;
    world.y = planar.y;
    ftmp = world.y / precis;
    tfd->y = (ftmp + 1.0) * .5 * rdiff;
    tfd->y += min;
  }
}

void
scatmatDisplayClassInit (GGobiScatmatDisplayClass * klass)
{
  klass->parent_class.show_edges_p = true;
  klass->parent_class.treeLabel = klass->parent_class.titleLabel =
    "Scatterplot Matrix";

  klass->parent_class.cpanel_set = cpanelSet;
  klass->parent_class.imode_control_box = scatmatCPanelWidget;

  #ifdef STORE_SESSION_ENABLED
  klass->parent_class.xml_describe = add_xml_scatmat_variables;
  #endif
  klass->parent_class.move_points_motion_cb = movePointsMotionCb;
  klass->parent_class.move_points_button_cb = movePointsButtonCb;
/* XXX duncan and dfs: you need to sort this out
	klass->parent_class.world_to_raw = worldToRaw;
*/
  klass->parent_class.variable_plotted_p = variablePlottedP;
  klass->parent_class.variable_select = variableSelect;
  klass->parent_class.varpanel_refresh = varpanelRefresh;
  klass->parent_class.varpanel_tooltips_set = varpanelTooltipsSet;
  klass->parent_class.plotted_vars_get = plottedVarsGet;
  klass->parent_class.createWithVars = createWithVars;
  klass->parent_class.display_set = displaySet;
  klass->parent_class.mode_ui_get = scatmat_mode_ui_get;
  klass->parent_class.handles_interaction = handlesInteraction;

  klass->parent_class.event_handlers_toggle = scatmatEventHandlersToggle;
  klass->parent_class.splot_key_event_handled = scatmatKeyEventHandled;
}


/* */
static gchar *
treeLabel (splotd * splot, GGobiData * d, ggobid * gg)
{
  return (g_strdup_printf("%s v %s", 
    ggobi_data_get_col_name(d, splot->xyvars.x), 
    ggobi_data_get_col_name(d, splot->xyvars.y)
  ));
}


static void
worldToPlane (splotd * sp, GGobiData * d, ggobid * gg)
{
  if (sp->p1dvar == -1)
    xy_reproject (sp, d->world.vals, d, gg);
  else
    p1d_reproject (sp, d->world.vals, d, gg);
}

static void
addIdentifyCues (gboolean nearest_p, gint k, splotd * sp,
                 GdkDrawable * drawable, ggobid * gg)
{
  colorschemed *scheme = gg->activeColorScheme;

  if (nearest_p)
    splot_add_diamond_cue (k, sp, drawable, gg);

  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
  splot_add_point_label (nearest_p, k, false, sp, drawable, gg);
}

gboolean
drawEdgeP (splotd * sp, gint m, GGobiData * d, GGobiData * e, ggobid * gg)
{
  gboolean draw_edge = true;
  if (sp->p1dvar != -1) {
    if (ggobi_data_is_missing(e, m, sp->p1dvar))
      draw_edge = false;
  }
  else {
    if (ggobi_data_is_missing(e, m, sp->xyvars.x) || ggobi_data_is_missing(e, m, sp->xyvars.y)) {
      draw_edge = false;
    }
  }
  return (draw_edge);
}

gboolean
drawCaseP (splotd * sp, gint m, GGobiData * d, ggobid * gg)
{
  gboolean draw_case = true;
  if (sp->p1dvar != -1) {
    if (ggobi_data_is_missing(d, m, sp->p1dvar))
      draw_case = false;
  }
  else {
    if (ggobi_data_is_missing(d, m, sp->xyvars.x) || ggobi_data_is_missing(d, m, sp->xyvars.y)) {
      draw_case = false;
    }
  }
  return (draw_case);
}

void
addPlotLabels (splotd * sp, GdkDrawable * drawable, ggobid * gg)
{
  if (sp->p1dvar == -1)
    scatterXYAddPlotLabels (sp, drawable, gg->plot_GC);
  else {
           /*-- 1dplot: center the label --*/
    scatter1DAddPlotLabels (sp, drawable, gg->plot_GC);
  }
}


static gint
splotVariablesGet (splotd * sp, gint * cols, GGobiData * d)
{
  if (sp->p1dvar > -1) {
    cols[0] = sp->p1dvar;
    return (1);
  }
  else {
    cols[0] = sp->xyvars.x;
    cols[1] = sp->xyvars.y;
    return (2);
  }
}


void
scatmatSPlotClassInit (GGobiScatmatSPlotClass * klass)
{
  klass->parent_class.tree_label = treeLabel;

  /* reverse pipeline */
  klass->parent_class.screen_to_tform = splotScreenToTform;
  klass->parent_class.world_to_plane = worldToPlane;

  klass->parent_class.draw_case_p = drawCaseP;
  klass->parent_class.draw_edge_p = drawEdgeP;
  klass->parent_class.add_plot_labels = addPlotLabels;
  klass->parent_class.add_identify_cues = addIdentifyCues;

  klass->parent_class.splot_assign_points_to_bins = splotAssignPointsToBins;

  klass->parent_class.plotted_vars_get = splotVariablesGet;
}
