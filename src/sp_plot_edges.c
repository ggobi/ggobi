/* sp_plot_edges.c: routines for drawing edges */
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Eclipse Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"
#include "colorscheme.h"

/*------------------------------------------------------------------------*/
/*                   line drawing routines                                */
/*------------------------------------------------------------------------*/

gboolean
splot_plot_edge (gint m, GGobiData * d, GGobiData * e,
                 splotd * sp, displayd * display, ggobid * gg)
{
  gint a, b;
  gboolean draw_edge;
  endpointsd *endpoints;

  endpoints = resolveEdgePoints (e, d);
  if (!endpoints)
    return (false);
  draw_edge = edge_endpoints_get (m, &a, &b, d, endpoints, e);

  if (!draw_edge)
    return (false);

  /*-- determine whether edge m should be plotted --*/
  /*-- usually checking sampled is redundant because we're looping
       over rows_in_plot, but maybe we're not always --*/

  if (e->excluded.els[m] || !e->sampled.els[m])
    draw_edge = false;

  else if (!splot_plot_case (a, d, sp, display, gg) ||
           !splot_plot_case (b, d, sp, display, gg))
    draw_edge = false;

  /*-- can prevent drawing of missings for parcoords or scatmat plots --*/
  else if (ggobi_data_has_missings(e) && !e->missings_show_p) {
    if (GGOBI_IS_EXTENDED_SPLOT (sp)) {
      GGobiExtendedSPlotClass *klass;
      klass = GGOBI_EXTENDED_SPLOT_GET_CLASS (sp);
      if (klass->draw_edge_p) {
        draw_edge = klass->draw_edge_p (sp, m, d, e, gg);
      }
    }
  }
  return draw_edge;
}

gboolean
splot_hidden_edge (gint m, GGobiData * d, GGobiData * e,
                   splotd * sp, displayd * display, ggobid * gg)
{
  gint a, b;
  gboolean hiddenp = false;
  endpointsd *endpoints;

  endpoints = resolveEdgePoints (e, d);
  if (endpoints && edge_endpoints_get (m, &a, &b, d, endpoints, e))
    if (e->hidden_now.els[m] || d->hidden_now.els[a] || d->hidden_now.els[b])
      hiddenp = true;

  /*-- can prevent drawing of missings for parcoords or scatmat plots --*/
/*
  } else if (ggobi_data_has_missings(e) && !e->missings_show_p) {
    if (GGOBI_IS_EXTENDED_SPLOT(sp)) {
      GGobiExtendedSPlotClass *klass;
      klass = GGOBI_EXTENDED_SPLOT_CLASS(GTK_OBJECT(sp)->klass);
      if (klass->draw_edge_p) {
        draw_edge = klass->draw_edge_p(sp, m, d, e, gg);
      }
    }
  }
*/
  return hiddenp;
}

/*-- the current color and line type need to be drawn last --*/
void
splot_edges_draw (splotd * sp, gboolean draw_hidden, cairo_t * cr, ggobid * gg)
{
  gint i, j, m;
  gint k, n, p, pp;
  gint a, b;
  displayd *display = (displayd *) sp->displayptr;
  GGobiData *d = display->d;
  GGobiData *e = display->e;
  endpointsd *endpoints;
  gboolean edges_show_p, arrowheads_show_p;
  gint lwidth, ltype;
  GlyphType gtype;
  colorschemed *scheme = gg->activeColorScheme;

  if (e == (GGobiData *) NULL || e->edge.n == 0) {
     /**/ return;
  }

  if (d->idTable == NULL) {  /*-- d has no record ids --*/
     /**/ return;
  }

  edges_show_p = (display->options.edges_directed_show_p ||
                  display->options.edges_undirected_show_p);
  arrowheads_show_p = (display->options.edges_directed_show_p ||
                       display->options.edges_arrowheads_show_p);
  if (!gg->mono_p && (edges_show_p || arrowheads_show_p)) {

    gint symbols_used[NGLYPHSIZES][NEDGETYPES][MAXNCOLORS];
    gint nl = 0;
    gint ncolors = MIN (MAXNCOLORS, scheme->n);
    gint k_prev = -1, n_prev = -1, p_prev = -1;

    g_assert (e->color.nels == e->nrows);

    endpoints = resolveEdgePoints (e, d);
    if (!endpoints)
      return;

    for (k = 0; k < NGLYPHSIZES; k++)
      for (n = 0; n < NEDGETYPES; n++)
        for (p = 0; p < ncolors; p++)
          symbols_used[k][n][p] = 0;

    /*
     * Use e->color_now[] and e->glyph_now[] to work out which
     * line symbols should be drawn
     */
    for (i = 0; i < e->nrows_in_plot; i++) {
      m = e->rows_in_plot.els[i];
      /* If we're drawing hiddens and this is hidden and plottable ... */
      if (((draw_hidden && splot_hidden_edge (m, d, e, sp, display, gg)) ||
           /* Or if we're not drawing hiddens and this isn't hidden ... */
           (!draw_hidden && !e->hidden_now.els[m]))) {

        gtype = e->glyph_now.els[m].type;
        ltype = ltype_from_gtype (gtype);
        /*
           if (gtype == FC || gtype == FR)
           ltype = SOLID;
           else if (gtype == OC || gtype == OR)
           ltype = WIDE_DASH;
           else ltype = NARROW_DASH;
         */
        symbols_used[e->glyph_now.els[m].size][ltype][e->color_now.els[m]]++;
      }
    }


    if (draw_hidden)
      cairo_set_source (cr, scheme->rgb_hidden);

    for (k = 0; k < NGLYPHSIZES; k++) {
      for (n = 0; n < NEDGETYPES; n++) {
        for (pp = 0; pp < ncolors + 1; pp++) {
          /* 
           * This is a little trick to draw the edges using the
           * current color last.  I skip over them the first time
           * through, and pick them up at the end.  It shouldn't
           * cost me anything, I don't think.  Should I do
           * the same for glyphs?     -- dfs, 4/2004
           */
          if (pp < ncolors) {
            if (pp == gg->color_id)
              continue;
            else
              p = pp;
          }
          else
            p = gg->color_id;

          if (symbols_used[k][n][p]) {

            /*
             * Now work through through symbols_used[], plotting the edges
             * of each color and type in a group.
             */
            nl = 0;

            for (j = 0; j < e->edge.n; j++) {

              if (draw_hidden
                  && !splot_hidden_edge (j, d, e, sp, display, gg))
                continue;
              if (!draw_hidden
                  && splot_hidden_edge (j, d, e, sp, display, gg))
                continue;
              if (!splot_plot_edge (j, d, e, sp, display, gg))
                continue;

              edge_endpoints_get (j, &a, &b, d, endpoints, e);

              gtype = e->glyph_now.els[j].type;
              ltype = ltype_from_gtype (gtype);
              /*
                 if (gtype == FC || gtype == FR)
                 ltype = SOLID;
                 else if (gtype == OC || gtype == OR)
                 ltype = WIDE_DASH;
                 else ltype = NARROW_DASH;
               */

              if (e->color_now.els[j] == p &&
                  ltype == n && e->glyph_now.els[j].size == k) {
                if (edges_show_p) {
                  if (endpoints[j].jpartner == -1) {
                    sp->edges[nl].x1 = sp->screen[a].x;
                    sp->edges[nl].y1 = sp->screen[a].y;
                    sp->edges[nl].x2 = sp->screen[b].x;
                    sp->edges[nl].y2 = sp->screen[b].y;
                  }
                  else {
                    sp->edges[nl].x1 = sp->screen[a].x;
                    sp->edges[nl].y1 = sp->screen[a].y;
                    sp->edges[nl].x2 = sp->screen[a].x +
                      (sp->screen[b].x - sp->screen[a].x) / 2;
                    sp->edges[nl].y2 = sp->screen[a].y +
                      (sp->screen[b].y - sp->screen[a].y) / 2;
                  }
                }

                if (arrowheads_show_p) {
                  /*
                   * Add thick piece of the lines to suggest a
                   * directional arrow.  How thick should it be
                   * compared to the current line thickness?
                   */
                  if (endpoints[j].jpartner == -1) {  /* not bidirectional */
                    sp->arrowheads[nl].x1 =
                      (gint) (.2 * sp->screen[a].x + .8 * sp->screen[b].x);
                    sp->arrowheads[nl].y1 =
                      (gint) (.2 * sp->screen[a].y + .8 * sp->screen[b].y);
                    sp->arrowheads[nl].x2 = sp->screen[b].x;
                    sp->arrowheads[nl].y2 = sp->screen[b].y;
                  }
                  else {    /*-- draw the partner's arrowhead --*/
                    gint ja, jb, jp = endpoints[j].jpartner;
                    edge_endpoints_get (jp, &ja, &jb, d, endpoints, e);

                    sp->arrowheads[nl].x1 =
                      (gint) (.2 * sp->screen[ja].x + .8 * sp->screen[jb].x);
                    sp->arrowheads[nl].y1 =
                      (gint) (.2 * sp->screen[ja].y + .8 * sp->screen[jb].y);
                    sp->arrowheads[nl].x2 = sp->screen[jb].x;
                    sp->arrowheads[nl].y2 = sp->screen[jb].y;
                  }
                }
                nl++;
              }
            }  /*-- end of looping through edges --*/

            if (!draw_hidden) {
              if (p_prev == -1 || p_prev != p) {  /* color */
                cairo_set_source (cr, scheme->rgb[p]);
              }
            }
            
            lwidth = lwidth_from_gsize (k);
            if (edges_show_p) {
              cairo_save (cr);
              ltype = set_dash_from_ltype (n, cr);

              if (k_prev == -1 || k_prev != i || n_prev == -1 || n_prev != n) {
                cairo_set_line_width(cr, lwidth);
                cairo_set_line_cap(cr, CAIRO_CAP_BUTT);
              }

              draw_segments (cr, sp->edges, nl);
              cairo_stroke (cr);
              cairo_restore (cr);
            }

            if (arrowheads_show_p) {
              cairo_set_line_width (cr, lwidth + 2);
              draw_segments (cr, sp->arrowheads, nl);
              cairo_set_line_width (cr, 0);
            }
          }

          /* 
           * I thought this should be one bracket earlier, but I'm
           * having some kind of intermittent problem with brushing
           * edges after I have added them using the GUI.  This may
           * fix the problem, which may have been a result of my
           * skipping over the current points --- though I don't see
           * why.  dfs 5/2004
           */
          k_prev = k;
          n_prev = n;
          p_prev = p;
        }
      }
    }
  }
}

void
splot_add_edge_highlight_cue (splotd * sp, cairo_t * cr, gint k,
                              gboolean nearest, ggobid * gg)
{
  displayd *dsp = (displayd *) sp->displayptr;
  GGobiData *d = dsp->d;
  GGobiData *e = dsp->e;
  gint a, b;
  endpointsd *endpoints;
  colorschemed *scheme = gg->activeColorScheme;
  gboolean draw_edge = (dsp->options.edges_undirected_show_p ||
                        dsp->options.edges_directed_show_p);

  endpoints = resolveEdgePoints (e, d);
  if (!endpoints)
    return;
  draw_edge = draw_edge && edge_endpoints_get (k, &a, &b, d, endpoints, e);

/*
 * How to distinguish between sticky and nearest edges?
*/
  /*-- draw a thickened line only for nearest --*/
  if (nearest && draw_edge) {
    cairo_save (cr);
    cairo_set_line_width (cr, 3);
    cairo_set_source (cr, scheme->rgb[e->color_now.els[k]]);

    cairo_move_to (cr, sp->screen[a].x, sp->screen[a].y);
    if (endpoints[k].jpartner == -1) {
      cairo_line_to (cr, sp->screen[b].x, sp->screen[b].y);
    }
    else {                      /* thicken only half the line */
      cairo_line_to (cr,
                     sp->screen[a].x + (sp->screen[b].x -
                                        sp->screen[a].x) / 2,
                     sp->screen[a].y + (sp->screen[b].y -
                                        sp->screen[a].y) / 2);
    }

    cairo_restore (cr);
  }
}

void
splot_add_edge_label (splotd * sp, cairo_t * cr, gint k,
                      gboolean nearest, ggobid * gg)
{
  gchar *lbl;
  displayd *dsp = (displayd *) sp->displayptr;
  GGobiData *d = dsp->d;
  GGobiData *e = dsp->e;
  gint xp, yp;
  gint a, b;
  endpointsd *endpoints;
  PangoLayout *layout = gtk_widget_create_pango_layout (sp->da, NULL);
  PangoRectangle rect;

  gboolean draw_edge = (dsp->options.edges_undirected_show_p ||
                        dsp->options.edges_directed_show_p);

  endpoints = resolveEdgePoints (e, d);
  if (!endpoints)
    return;
  draw_edge = draw_edge && edge_endpoints_get (k, &a, &b, d, endpoints, e);

  /*-- If the edge is bidirectional, use both labels --*/

  if (draw_edge) {

    /*-- add the label last so it will be in front of other markings --*/

    /* for edge labels, this is not the appropriate cpanel ... */
    lbl = identify_label_fetch (k, &dsp->cpanel, e, gg);
    layout_text (layout, lbl, &rect);

    if (sp->screen[a].x > sp->screen[b].x) {
      gint itmp = b;
      b = a;
      a = itmp;
    }
    xp = (sp->screen[b].x - sp->screen[a].x) / 2 + sp->screen[a].x;
    if (sp->screen[a].y > sp->screen[b].y) {
      gint itmp = b;
      b = a;
      a = itmp;
    }
    yp =
      (sp->screen[b].y - sp->screen[a].y) / 2 + sp->screen[a].y - rect.height;

    if (nearest) {
      underline_text (layout);
      cairo_move_to (cr, (sp->max.x - rect.width) / 2, 5);
      pango_cairo_show_layout (cr, layout);
    }
    cairo_move_to (cr, xp, yp);
    pango_cairo_show_layout (cr, layout);
  }
}

void
splot_add_identify_edge_cues (splotd * sp, cairo_t * cr, gint k,
                              gboolean nearest, ggobid * gg)
{
  displayd *dsp = (displayd *) sp->displayptr;
  GGobiData *e = dsp->e;
  gboolean useDefault = false;

  if (k >= e->edge.n)
    return;

  if (e->hidden_now.els[k])
    return;

  if (GGOBI_IS_EXTENDED_SPLOT (sp)) {
    GGobiExtendedSPlotClass *klass;
    klass = GGOBI_EXTENDED_SPLOT_GET_CLASS (sp);
    if (klass->add_identify_edge_cues)
      klass->add_identify_edge_cues (k, sp, cr, nearest, gg);
    else
      useDefault = true;
  }

  if (useDefault) {
    splot_add_edge_highlight_cue (sp, cr, k, nearest, gg);
    splot_add_edge_label (sp, cr, k, nearest, gg);
  }
}

void
splot_add_edgeedit_cues (splotd * sp, cairo_t * cr,
                         gint k, gboolean nearest, ggobid * gg)
{
  displayd *display = sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  colorschemed *scheme = gg->activeColorScheme;
  gint lwidth;
  gint size = gg->glyph_id.size;

  /*-- just rely on the cursor for adding points; no other markup --*/

  if (cpanel->ee_mode == ADDING_EDGES) {
    if (k != -1)
      splot_add_diamond_cue (k, sp, drawable, gg);

    if (gg->edgeedit.a != -1)
      splot_add_diamond_cue (gg->edgeedit.a, sp, drawable, gg);

    if (gg->buttondown && gg->edgeedit.a != -1 &&
        k != -1 && k != gg->edgeedit.a) {

      cairo_save (cr);
      lwidth = lwidth_from_gsize (size);  // Not sure gsize is relevant.
      cairo_set_line_width (cr, lwidth);
      cairo_set_line_cap (cr, CAIRO_CAP_BUTT);

      /* This isn't really the color I want to use, but I don't know
         how to get at the color of the endpoints here. */
      cairo_set_source (cr, scheme->rgb[gg->color_id]);
      cairo_move_to (cr,
                     sp->screen[gg->edgeedit.a].x,
                     sp->screen[gg->edgeedit.a].y);
      cairo_line_to (cr, sp->screen[k].x, sp->screen[k].y);

      cairo_restore (cr);
    }
/*  not ready to support deleting
  else if (cpanel->ee_deleting_p)
*/
  }
}
