/* brush.c */
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

#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*----------------------------------------------------------------------*/
/*             Glyph utility:                                           */
/*----------------------------------------------------------------------*/

/* convert a glyph size into a line width */
gint
lwidth_from_gsize (gint size)
{
  return ((size < 3) ? 0 : (size - 2) * 2);
}

/* This refers to the stored ltype, apparently */
gint
ltype_from_gtype (gint gtype)
{
  gint ltype;
  
  if (gtype == FC || gtype == FR)
    ltype = SOLID;
  else if (gtype == OC || gtype == OR)
    ltype = WIDE_DASH;
  else
    ltype = NARROW_DASH;

  return ltype;
}

/* sets dashes and returns a gtk line attribute */
gint
set_lattribute_from_ltype (gint ltype, ggobid * gg)
{
  gint8 dash_list[2];
  gint lattr = GDK_LINE_SOLID;

  switch (ltype) {    /* one of the three EDGETYPES; should be an enum */
  case SOLID:
    lattr = GDK_LINE_SOLID;
    break;
  case WIDE_DASH:
    lattr = GDK_LINE_ON_OFF_DASH;
    dash_list[0] = 8;
    dash_list[1] = 2;
    gdk_gc_set_dashes (gg->plot_GC, 0, dash_list, 2);
    break;
  case NARROW_DASH:
    lattr = GDK_LINE_ON_OFF_DASH;
    dash_list[0] = 4;
    dash_list[1] = 2;
    gdk_gc_set_dashes (gg->plot_GC, 0, dash_list, 2);
    break;
  }
  return lattr;
}


/*----------------------------------------------------------------------*/
/*                      Dealing with the brush                          */
/*----------------------------------------------------------------------*/

static void
brush_boundaries_set (cpaneld * cpanel,
                      icoords * obin0, icoords * obin1,
                      icoords * imin, icoords * imax, GGobiData * d,
                      ggobid * gg)
{
  icoords *bin0 = &d->brush.bin0;
  icoords *bin1 = &d->brush.bin1;

  // For transient brushing, need to update points in current AND previous bin
  if (cpanel->br.mode == BR_TRANSIENT) {
    imin->x = MIN (bin0->x, obin0->x);
    imin->y = MIN (bin0->y, obin0->y);
    imax->x = MAX (bin1->x, obin1->x);
    imax->y = MAX (bin1->y, obin1->y);
  } else {
    imin->x = bin0->x;
    imin->y = bin0->y;
    imax->x = bin1->x;
    imax->y = bin1->y;
  }
}

void
brush_draw_label (splotd * sp, GdkDrawable * drawable, GGobiData * d,
                  ggobid * gg)
{
  PangoRectangle rect;
  
  if (d->npts_under_brush == 0)
   return;
  
  PangoLayout *layout =
    gtk_widget_create_pango_layout (GTK_WIDGET (sp->da), NULL);

  gchar *str = g_strdup_printf ("%d", d->npts_under_brush);
  layout_text (layout, str, &rect);
  gdk_draw_layout (drawable, gg->plot_GC,
                   sp->max.x - rect.width - 5, 5, layout);

  g_free (str);
  g_object_unref (G_OBJECT (layout));
}

void
brush_draw_brush (splotd * sp, GdkDrawable * drawable, GGobiData * d,
                  ggobid * gg)
{
/*
 * Use brush_pos to draw the brush.
*/
  displayd *display = sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  gboolean point_painting_p = (cpanel->br.point_targets != br_off);
  gboolean edge_painting_p = (cpanel->br.edge_targets != br_off);
  colorschemed *scheme = gg->activeColorScheme;

  brush_coords *brush_pos = &sp->brush_pos;
  gint x1 = MIN (brush_pos->x1, brush_pos->x2);
  gint x2 = MAX (brush_pos->x1, brush_pos->x2);
  gint y1 = MIN (brush_pos->y1, brush_pos->y2);
  gint y2 = MAX (brush_pos->y1, brush_pos->y2);

  if (cpanel->br.mode == BR_TRANSIENT)
    gdk_gc_set_line_attributes (gg->plot_GC,
                                0, GDK_LINE_ON_OFF_DASH, GDK_CAP_ROUND,
                                GDK_JOIN_ROUND);

  if (point_painting_p) {

    /* set brush color for points */
    if (cpanel->br.point_targets == br_shadow) {
      gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_hidden);
    }
    else if (cpanel->br.point_targets == br_unshadow) {
      gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
    }
    else if ((scheme->rgb[gg->color_id].red != scheme->rgb_bg.red) ||
             (scheme->rgb[gg->color_id].blue != scheme->rgb_bg.blue) ||
             (scheme->rgb[gg->color_id].green != scheme->rgb_bg.green)) {
      gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb[gg->color_id]);
    }
    else {
      /* I don't remember what this is for ... -- dfs */
      gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
    }

    gdk_draw_rectangle (drawable, gg->plot_GC, false,
                        x1, y1, (x2 > x1) ? (x2 - x1) : (x1 - x2),
                        (y2 > y1) ? (y2 - y1) : (y1 - y2));
    /* Mark the corner to which the cursor will be attached */
    gdk_draw_rectangle (drawable, gg->plot_GC, true,
                        brush_pos->x2 - 1, brush_pos->y2 - 1, 2, 2);

    /*
     * highlight brush: but only in the current display
     */
    if (cpanel->br.brush_on_p && display == gg->current_display) {
      gdk_draw_rectangle (drawable, gg->plot_GC, false,
                          x1 - 1, y1 - 1,
                          (x2 > x1) ? (x2 - x1 + 2) : (x1 - x2 + 2),
                          (y2 > y1) ? (y2 - y1 + 2) : (y1 - y2 + 2));

      /* Mark the corner to which the cursor will be attached */
      gdk_draw_rectangle (drawable, gg->plot_GC, true,
                          brush_pos->x2 - 2, brush_pos->y2 - 2, 4, 4);
    }
  }

  if (edge_painting_p) {

    /* set brush color for edges */
    if (cpanel->br.edge_targets == br_shadow) {
      gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_hidden);
    }
    else if (cpanel->br.point_targets == br_unshadow) {
      gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
    }
    else if ((scheme->rgb[gg->color_id].red != scheme->rgb_bg.red) ||
             (scheme->rgb[gg->color_id].blue != scheme->rgb_bg.blue) ||
             (scheme->rgb[gg->color_id].green != scheme->rgb_bg.green)) {
      gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb[gg->color_id]);
    }
    else {
      /* I don't remember what this is for ... -- dfs */
      gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
    }

    gdk_draw_line (drawable, gg->plot_GC,
                   x1 + (x2 - x1) / 2, y1, x1 + (x2 - x1) / 2, y2);
    gdk_draw_line (drawable, gg->plot_GC,
                   x1, y1 + (y2 - y1) / 2, x2, y1 + (y2 - y1) / 2);

    if (cpanel->br.brush_on_p) {
      gdk_draw_line (drawable, gg->plot_GC,
                     x1 + (x2 - x1) / 2 + 1, y1, x1 + (x2 - x1) / 2 + 1, y2);
      gdk_draw_line (drawable, gg->plot_GC,
                     x1, y1 + (y2 - y1) / 2 + 1, x2, y1 + (y2 - y1) / 2 + 1);
    }

  }
  if (cpanel->br.mode == BR_TRANSIENT)
    gdk_gc_set_line_attributes (gg->plot_GC,
                                0, GDK_LINE_SOLID, GDK_CAP_ROUND,
                                GDK_JOIN_ROUND);
}



void
brush_set_pos (gint x, gint y, splotd * sp)
{
  brush_coords *brush = &sp->brush_pos;
  brush_coords *obrush = &sp->brush_pos_o;
  gint xdist = brush->x2 - brush->x1;
  gint ydist = brush->y2 - brush->y1;

  /*-- copy the current coordinates to the backup brush structure --*/
  obrush->x1 = brush->x1;
  obrush->y1 = brush->y1;
  obrush->x2 = brush->x2;
  obrush->y2 = brush->y2;

  /*
   * (x2,y2) is the corner that's moving.
   */
  brush->x1 = x - xdist;
  brush->x2 = x;
  brush->y1 = y - ydist;
  brush->y2 = y;
}


void
brush_undo (GGobiData * d)
{
  gint m, i;
  g_return_if_fail(d);

  for (m = 0; m < d->nrows_in_plot; m++) {
    i = d->rows_in_plot.els[m];
    ggobi_data_reset_attr(d, i, ATTR_SET_PERSISTENT);
  }
}

void
reinit_transient_brushing (displayd * dsp, ggobid * gg)
{
/*
 * If a new variable is selected or a variable is transformed
 * during transient brushing, we may want to restore all points to
 * the permanent value.  After calling this routine, re-execute
 * brush_once() to brush the points that are now underneath the brush. 
 * For now, don't make the same change for persistent brushing.
*/
  gint i, m, k;
  GGobiData *d = dsp->d;
  GGobiData *e = dsp->e;
  cpaneld *cpanel = &dsp->cpanel;
  gboolean point_painting_p = (cpanel->br.point_targets != br_off);
  gboolean edge_painting_p = (cpanel->br.edge_targets != br_off);

  if (point_painting_p) {
    for (m = 0; m < d->nrows_in_plot; m++) {
      i = d->rows_in_plot.els[m];
      ggobi_data_reset_attr(d, i, ATTR_SET_TRANSIENT);
    }
  }
  if (edge_painting_p && e) {
    for (k = 0; k < e->edge.n; k++)
      ggobi_data_reset_attr(e, k, ATTR_SET_TRANSIENT);
  }
}

static gboolean
binning_permitted (displayd * display, ggobid * gg)
{
  if (gg->linkby_cv)
    return false;

  GGobiExtendedDisplayClass *klass;
  klass = GGOBI_EXTENDED_DISPLAY_GET_CLASS (display);
  if (klass->binningPermitted) {
    return (klass->binningPermitted (display));
  }
  return (klass->binning_ok);

}



gboolean
brush_motion (icoords * mouse, gboolean button1_p, gboolean button2_p,
              cpaneld * cpanel, splotd * sp, ggobid * gg)
{
  displayd *display = sp->displayptr;
  brush_coords *brush_pos = &sp->brush_pos;

  if (button1_p) {
    brush_set_pos (mouse->x, mouse->y, sp);
  } else if (button2_p) {
    brush_pos->x2 = mouse->x;
    brush_pos->y2 = mouse->y;
  }

  return brush_once_and_redraw(true, sp, display, gg); /* binning permitted */
}


/*
 * Determine whether point k is under the brush.
*/
static gboolean
under_brush (gint k, splotd * sp)
{
  brush_coords *brush_pos = &sp->brush_pos;
  gint x1 = MIN (brush_pos->x1, brush_pos->x2);
  gint x2 = MAX (brush_pos->x1, brush_pos->x2);
  gint y1 = MIN (brush_pos->y1, brush_pos->y2);
  gint y2 = MAX (brush_pos->y1, brush_pos->y2);

  return (sp->screen[k].x <= x2 && sp->screen[k].y <= y2 &&
          sp->screen[k].x >= x1 && sp->screen[k].y >= y1);
}

/*
 * Determine whether edge k intersects the brush
*/
static gboolean
xed_by_brush (gint k, displayd * display, ggobid * gg)
{
  GGobiData *d = display->d;
  GGobiData *e = display->e;
  splotd *sp = gg->current_splot;
  gboolean intersect;
  glong x1 = sp->brush_pos.x1;
  glong y1 = sp->brush_pos.y1;
  glong x2 = sp->brush_pos.x2;
  glong y2 = sp->brush_pos.y2;
  gint a, b;

  endpointsd *endpoints;
  endpoints = resolveEdgePoints (e, d);

  if (!endpoints || !edge_endpoints_get (k, &a, &b, d, endpoints, e))
    return false;

  /*-- test for intersection with the vertical edge --*/
  intersect = lines_intersect (x1 + (x2 - x1) / 2, y1, x1 + (x2 - x1) / 2, y2,
                               sp->screen[a].x, sp->screen[a].y,
                               sp->screen[b].x, sp->screen[b].y);
/*
  intersect = isCrossed (x1 + (x2 - x1)/2, y1, x1 + (x2 - x1)/2, y2,
    (gdouble) sp->screen[a].x, (gdouble) sp->screen[a].y,
    (gdouble) sp->screen[b].x, (gdouble) sp->screen[b].y);
*/

/*-- I wonder if Lee's method is truly faster --- it requires
     doubles, which forces me to do a lot of casting.  I should
     figure out how to test it --*/
  if (intersect != 1) {
    intersect =
      lines_intersect (x1, y1 + (y2 - y1) / 2, x2, y1 + (y2 - y1) / 2,
                       sp->screen[a].x, sp->screen[a].y, sp->screen[b].x,
                       sp->screen[b].y);
/*
    intersect = isCrossed (x1, y1 + (y2 - y1)/2, x2, y1 + (y2 - y1)/2,
      (gdouble) sp->screen[a].x, (gdouble) sp->screen[a].y,
      (gdouble) sp->screen[b].x, (gdouble) sp->screen[b].y);
*/
  }

  return (intersect == 1);
}

/*----------------------------------------------------------------------*/
/*         Handle all symbols in one loop through a bin                 */
/*----------------------------------------------------------------------*/

static gboolean
paint_points (cpaneld * cpanel, GGobiData * d, ggobid * gg)
{
  gint ih, iv, m, j;
  /*-- these look suspicious -- dfs --*/
  static icoords obin0 = { BRUSH_NBINS / 2, BRUSH_NBINS / 2 };
  static icoords obin1 = { BRUSH_NBINS / 2, BRUSH_NBINS / 2 };
  icoords imin, imax;
  gboolean changed = false;
  gint nd = g_slist_length (gg->d);
  gboolean (*f) (cpaneld *, GGobiData *, ggobid *) = NULL;

  splotd *sp = gg->current_splot;
  displayd *display = (displayd *) sp->displayptr;
  f = GGOBI_EXTENDED_DISPLAY_GET_CLASS (display)->build_symbol_vectors;
  if (f)
    return f(cpanel, d, gg);

  brush_boundaries_set(cpanel, &obin0, &obin1, &imin, &imax, d, gg);

  for (ih = imin.x; ih <= imax.x; ih++) {
    for (iv = imin.y; iv <= imax.y; iv++) {
      for (m = 0; m < d->brush.binarray[ih][iv].nels; m++) {
        j = d->rows_in_plot.els[d->brush.binarray[ih][iv].els[m]];
        changed = ggobi_data_brush_point(d, j, d->pts_under_brush.els[j], 
          cpanel->br.point_targets, cpanel->br.mode) || changed;

        if (!gg->linkby_cv && nd > 1)
          symbol_link_by_id (false, j, d, gg);
      }
    }
  }

  obin0.x = d->brush.bin0.x;
  obin0.y = d->brush.bin0.y;
  obin1.x = d->brush.bin1.x;
  obin1.y = d->brush.bin1.y;

  return changed;
}


static gboolean
paint_edges (cpaneld * cpanel, GGobiData * e, ggobid * gg)
{
  gint i;
  gboolean changed = false;
  gint nd = g_slist_length (gg->d);

  for (i = 0; i < e->edge.n; i++) {
    changed = ggobi_data_brush_point(e, i, e->edge.xed_by_brush.els[i], 
      cpanel->br.edge_targets, cpanel->br.mode) || changed;

    if (!gg->linkby_cv && nd > 1)
      symbol_link_by_id (false, i, e, gg);
  }

  return (changed);
}

/**
 * update_points_under_brush:
 * @d: #GGobiData
 * @splotd: plot in which brush is located
 * 
 * Update points under brush in #GGobiData object to
 * reflect which points are under the brush in the specified
 * plot.
 *
 * Will use points_update_paint method of #splotd object, if available
**/
static void
update_points_under_brush(GGobiData *d, splotd *sp)
{
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  guint ih, iv, j, pt;
  BrushTargetType ttype = cpanel->br.point_targets;
  gint (*f) (splotd * sp, GGobiData *, ggobid *) = NULL;

  f = GGOBI_EXTENDED_SPLOT_GET_CLASS (sp)->active_paint_points;
  if (f) {
    d->npts_under_brush = f (sp, d, d->gg);
    return;
  }

  /* Zero out pts_under_brush[] before looping */
  d->npts_under_brush = 0;
  for (j = 0; j < d->nrows_in_plot; j++)
    d->pts_under_brush.els[d->rows_in_plot.els[j]] = false;

  for (ih = d->brush.bin0.x; ih <= d->brush.bin1.x; ih++) {
    for (iv = d->brush.bin0.y; iv <= d->brush.bin1.y; iv++) {
      for (j = 0; j < d->brush.binarray[ih][iv].nels; j++) {
        pt = d->rows_in_plot.els[d->brush.binarray[ih][iv].els[j]];

        /*
         * Ignore hidden cases unless shadow or unshadow brushing.
         */
        if (ggobi_data_get_attr_hidden(d, pt) && ttype != br_unshadow)
          continue;

        if (splot_plot_case(pt, d, sp, display, d->gg) && under_brush (pt, sp)) {
          d->npts_under_brush++;
          d->pts_under_brush.els[pt] = true;
        } else {
          d->pts_under_brush.els[pt] = false;
        }
      }
    }
  }

}

/**
 * update_edges_under_brush:
 * @d: #GGobiData
 * @splotd: plot in which brush is located
 * 
 * Update edges under brush in #GGobiData object to
 * reflect which edges are under the brush in the specified
 * plot.
**/
void
update_edges_under_brush(GGobiData *d, splotd *sp)
{
  gint k;
  displayd *display = sp->displayptr;

  g_assert (d->edge.xed_by_brush.nels == d->edge.n);

  for (k = 0; k < d->edge.n; k++) {
    if (xed_by_brush (k, display, d->gg)) {
      d->edge.nxed_by_brush++;
      d->edge.xed_by_brush.els[k] = true;
    } else {
      d->edge.xed_by_brush.els[k] = false;
    }
  }
}


gboolean
edges_update_paint (splotd * sp, GGobiData * e, ggobid * gg)
{
  displayd *display = sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
    
  update_points_under_brush(e, sp);

  return paint_edges (cpanel, e, gg);
  /*if (gg->linkby_cv) {
    // link by categorical variable: presently unavailable
    return paint_points_by_var (cpanel, e, gg);
  } */
}

gboolean
points_update_paint (splotd * sp, GGobiData * d, ggobid * gg)
{
  displayd *display = sp->displayptr;
  cpaneld *cpanel = &display->cpanel;

  update_points_under_brush(d, sp);

  if (gg->linkby_cv) {
    return build_symbol_vectors_by_var (cpanel, d, gg);
  } else {
    return paint_points (cpanel, d, gg);
  }
}


/*----------------------------------------------------------------------*/

/*-- called by brush_motion, brush_mode_cb, and in the api --*/
gboolean
brush_once (gboolean force, splotd * sp, ggobid * gg)
{
/*
 * Determine which bins the brush is currently sitting in.
 * bin0 is the bin which contains of the upper left corner of the
 * brush; bin1 is the one containing of the lower right corner.
*/
  displayd *display = sp->displayptr;
  GGobiData *d = display->d;
  GGobiData *e = display->e;

  brush_coords *brush_pos = &sp->brush_pos;

  icoords *bin0 = &d->brush.bin0;
  icoords *bin1 = &d->brush.bin1;

  gboolean changed = false;
  cpaneld *cpanel = &display->cpanel;

  if (!cpanel->br.brush_on_p)
    return false;

  if (force) {  /*-- force the bin to be the entire screen --*/
    bin0->x = 0;
    bin0->y = 0;
    bin1->x = d->brush.nbins - 1;
    bin1->y = d->brush.nbins - 1;

  } else {

    gint ulx = MIN (brush_pos->x1, brush_pos->x2);
    gint uly = MIN (brush_pos->y1, brush_pos->y2);
    gint lrx = MAX (brush_pos->x1, brush_pos->x2);
    gint lry = MAX (brush_pos->y1, brush_pos->y2);

    if (!point_in_which_bin (ulx, uly, &bin0->x, &bin0->y, d, sp)) {
      bin0->x = MAX (bin0->x, 0);
      bin0->x = MIN (bin0->x, d->brush.nbins - 1);
      bin0->y = MAX (bin0->y, 0);
      bin0->y = MIN (bin0->y, d->brush.nbins - 1);
    }
    if (!point_in_which_bin (lrx, lry, &bin1->x, &bin1->y, d, sp)) {
      bin1->x = MAX (bin1->x, 0);
      bin1->x = MIN (bin1->x, d->brush.nbins - 1);
      bin1->y = MAX (bin1->y, 0);
      bin1->y = MIN (bin1->y, d->brush.nbins - 1);
    }
  }

  if (cpanel->br.point_targets)
    changed = points_update_paint (sp, d, gg);

  if (cpanel->br.edge_targets && e) 
    changed = edges_update_paint (sp, e, gg) || changed;

  return (changed);
}

gboolean
brush_once_and_redraw (gboolean binningp, splotd * sp, displayd * display,
                       ggobid * gg)
{
  cpaneld *cpanel = &display->cpanel;
  gboolean changed = false;
  
  if (!cpanel->br.brush_on_p || !(changed = brush_once (!binningp, sp, gg))) {
    splot_redraw (sp, QUICK, gg);
    return false;    
  }

  if (binningp && binning_permitted (display, gg)) {
    splot_redraw (sp, BINNED, gg);
  } else {                      /* no binning */
    splot_redraw (sp, FULL, gg);
  }
  if (cpanel->br.updateAlways_p)
    displays_plot (sp, FULL, gg);

  return changed;
}