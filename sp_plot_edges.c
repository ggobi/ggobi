/* sp_plot_edges.c: routines for drawing edges */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
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
splot_plot_edge (gint m, gboolean ignore_hidden, datad *d, datad *e,
  splotd *sp, displayd *display, ggobid *gg)
{
  gint a, b;
  gboolean draw_edge;
  endpointsd *endpoints;

  endpoints = resolveEdgePoints(e, d);
  draw_edge = edge_endpoints_get (m, &a, &b, d, endpoints, e);

  if(!draw_edge)
    return(false);

  /*-- determine whether edge m should be plotted --*/
  /*-- usually checking sampled is redundant because we're looping
       over rows_in_plot, but maybe we're not always --*/

  if (!e->sampled.els[m])
    draw_edge = false;
  else if (ignore_hidden) {
    if (e->hidden_now.els[m])
      draw_edge = false;
    else if (!splot_plot_case (a, ignore_hidden, d, sp, display, gg) ||
             !splot_plot_case (b, ignore_hidden, d, sp, display, gg))
      draw_edge = false;
  
  /*-- can prevent drawing of missings for parcoords or scatmat plots --*/
  } else if (e->nmissing > 0 && !e->missings_show_p) {
    if (GTK_IS_GGOBI_EXTENDED_SPLOT(sp)) {
      GtkGGobiExtendedSPlotClass *klass;
      klass = GTK_GGOBI_EXTENDED_SPLOT_CLASS(GTK_OBJECT(sp)->klass);
      if (klass->draw_edge_p) {
        draw_edge = klass->draw_edge_p(sp, m, d, e, gg);
      }
    }
  }
  return draw_edge;
}

gboolean
splot_hidden_edge (gint m, datad *d, datad *e,
  splotd *sp, displayd *display, ggobid *gg)
{
  gint a, b;
  gboolean hiddenp = false;
  endpointsd *endpoints;

  endpoints = resolveEdgePoints(e, d);
  if (edge_endpoints_get (m, &a, &b, d, endpoints, e))
    if (e->hidden_now.els[m] || d->hidden_now.els[a] || d->hidden_now.els[b])
      hiddenp = true;
  
  /*-- can prevent drawing of missings for parcoords or scatmat plots --*/
/*
  } else if (e->nmissing > 0 && !e->missings_show_p) {
    if (GTK_IS_GGOBI_EXTENDED_SPLOT(sp)) {
      GtkGGobiExtendedSPlotClass *klass;
      klass = GTK_GGOBI_EXTENDED_SPLOT_CLASS(GTK_OBJECT(sp)->klass);
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
splot_edges_draw (splotd *sp, gboolean draw_hidden, GdkDrawable *drawable,
  ggobid *gg)
{
  gint i, j, m;
  gint k, n, p;
  gint a, b;
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;
  datad *e = display->e;
  endpointsd *endpoints;
  gboolean edges_show_p, arrowheads_show_p;
  gint lwidth, ltype;
  GlyphType gtype;
  colorschemed *scheme = gg->activeColorScheme;

  if (e == (datad *) NULL || e->edge.n == 0) {
/**/return;
  }
 
  if (d->idTable == NULL) {  /*-- d has no record ids --*/
/**/return;
  }

  edges_show_p = (display->options.edges_directed_show_p ||
                  display->options.edges_undirected_show_p);
  arrowheads_show_p = (display->options.edges_directed_show_p ||
                       display->options.edges_arrowheads_show_p);
  if (!gg->mono_p && (edges_show_p || arrowheads_show_p)) {

    gint symbols_used[NGLYPHSIZES][NEDGETYPES][MAXNCOLORS];
    gint nl = 0;
    gint ncolors = MIN(MAXNCOLORS, scheme->n);
    gint k_prev = -1, n_prev = -1, p_prev = -1;

    endpoints = resolveEdgePoints(e, d);

    for (k=0; k<NGLYPHSIZES; k++)
      for (n=0; n<NEDGETYPES; n++)
        for (p=0; p<ncolors; p++)
          symbols_used[k][n][p] = 0;

    /*
     * Use e->color_now[] and e->glyph_now[] to work out which
     * line symbols should be drawn
    */
    for (i=0; i<e->nrows_in_plot; i++) {
      m = e->rows_in_plot.els[i];
      /* If we're drawing hiddens and this is hidden and plottable ... */
      if (((draw_hidden && splot_hidden_edge (m, d, e, sp, display, gg)) ||
      /* Or if we're not drawing hiddens and this isn't hidden ... */
          (!draw_hidden && !e->hidden_now.els[m])))
      {
        gtype = e->glyph_now.els[m].type;
        if (gtype == FC || gtype == FR)
          ltype = SOLID;
        else if (gtype == OC || gtype == OR)
          ltype = WIDE_DASH;
        else ltype = NARROW_DASH;

        symbols_used[e->glyph_now.els[m].size][ltype][e->color_now.els[m]]++;
      }
    }


/*
 * It would be nice to draw the edges using the current color and
 * glyph last, but it's not obvious how to set that up in this scheme.
 * I could skip them on the way through and then draw them at the end,
 * I think.
*/
    if (draw_hidden)
      gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_hidden);

    for (k=0; k<NGLYPHSIZES; k++) {
      for (n=0; n<NEDGETYPES; n++) {
        for (p=0; p<ncolors; p++) {
          if (symbols_used[k][n][p]) {

            /*
             * Now work through through symbols_used[], plotting the edges
             * of each color and type in a group.
            */
            nl = 0;

            for (j=0; j<e->edge.n; j++) {

              if (draw_hidden) {
                if (!splot_plot_edge (j, false, d, e, sp, display, gg)) {
                  continue;
                }
              } else {
                if (!splot_plot_edge (j, true, d, e, sp, display, gg)) {
                  continue;
                }
              }

              edge_endpoints_get (j, &a, &b, d, endpoints, e);

              gtype = e->glyph_now.els[j].type;
              if (gtype == FC || gtype == FR)
                ltype = SOLID;
              else if (gtype == OC || gtype == OR)
                ltype = WIDE_DASH;
              else ltype = NARROW_DASH;

              if (e->color_now.els[j] == p &&
                  ltype == n &&
                  e->glyph_now.els[j].size == k)
              {
                if (edges_show_p) {
                  if (endpoints[j].jpartner == -1) {
                    sp->edges[nl].x1 = sp->screen[a].x;
                    sp->edges[nl].y1 = sp->screen[a].y;
                    sp->edges[nl].x2 = sp->screen[b].x;
                    sp->edges[nl].y2 = sp->screen[b].y;
                  } else {
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
                  if (endpoints[j].jpartner == -1) { /* not bidirectional */
                    sp->arrowheads[nl].x1 =
                      (gint) (.2*sp->screen[a].x + .8*sp->screen[b].x);
                    sp->arrowheads[nl].y1 =
                      (gint) (.2*sp->screen[a].y + .8*sp->screen[b].y);
                    sp->arrowheads[nl].x2 = sp->screen[b].x;
                    sp->arrowheads[nl].y2 = sp->screen[b].y;
                  } else {  /*-- draw the partner's arrowhead --*/
                    gint ja, jb, jp = endpoints[j].jpartner;
                    edge_endpoints_get (jp, &ja, &jb, d, endpoints, e);

                    sp->arrowheads[nl].x1 =
                      (gint) (.2*sp->screen[ja].x + .8*sp->screen[jb].x);
                    sp->arrowheads[nl].y1 =
                      (gint) (.2*sp->screen[ja].y + .8*sp->screen[jb].y);
                    sp->arrowheads[nl].x2 = sp->screen[jb].x;
                    sp->arrowheads[nl].y2 = sp->screen[jb].y;
                  }
                }
                nl++;
              }
            }  /*-- end of looping through edges --*/

            if (!draw_hidden) {
              if (p_prev == -1 || p_prev != p) {  /* color */
                gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb[p]);
              }
            }

            lwidth = (k<3) ? 0 : (k-2)*2;
            if (edges_show_p) {
              gchar dash_list[2];

              if (n_prev == -1 || n_prev != n) {  /* type */
                switch (n) {
                  case SOLID:
                    ltype = GDK_LINE_SOLID;
                  break;
                  case WIDE_DASH:
                    ltype = GDK_LINE_ON_OFF_DASH;
                    dash_list[0] = 8;
                    dash_list[1] = 2;
                    gdk_gc_set_dashes (gg->plot_GC, 0, dash_list, 2);
                  break;
                  case NARROW_DASH:
                    ltype = GDK_LINE_ON_OFF_DASH;
                    dash_list[0] = 4;
                    dash_list[1] = 2;
                    gdk_gc_set_dashes (gg->plot_GC, 0, dash_list, 2);
                  break;
                }
              }
              if (k_prev == -1 || k_prev != i || n_prev == -1 || n_prev != n) {
                gdk_gc_set_line_attributes (gg->plot_GC, lwidth,
                  (gint) ltype, GDK_CAP_BUTT, GDK_JOIN_ROUND);
              }

              gdk_draw_segments (drawable, gg->plot_GC, sp->edges, nl);
            }

            if (arrowheads_show_p) {
              gdk_gc_set_line_attributes (gg->plot_GC,
                lwidth+2, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
              gdk_draw_segments (drawable, gg->plot_GC, sp->arrowheads, nl);
              gdk_gc_set_line_attributes (gg->plot_GC,
                0, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
            }

            k_prev = k; n_prev = n; p_prev = p;
          }
        }
      }
    }
  }
  gdk_gc_set_line_attributes (gg->plot_GC,
    0, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
}

void
splot_add_edge_highlight_cue (splotd *sp, GdkDrawable *drawable, gint k,
  gboolean nearest, ggobid *gg)
{
  displayd *dsp = (displayd *) sp->displayptr;
  datad *d = dsp->d;
  datad *e = dsp->e;
  gint a, b;
  endpointsd *endpoints;
  colorschemed *scheme = gg->activeColorScheme;
  gboolean draw_edge = (dsp->options.edges_undirected_show_p ||
                        dsp->options.edges_directed_show_p);

  endpoints = resolveEdgePoints(e, d);
  draw_edge = draw_edge && edge_endpoints_get (k, &a, &b, d, endpoints, e);

/*
 * How to distinguish between sticky and nearest edges?
*/
  /*-- draw a thickened line --*/
  if (draw_edge) {
    gdk_gc_set_line_attributes (gg->plot_GC,
      3, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
    gdk_gc_set_foreground (gg->plot_GC,
      &scheme->rgb[ e->color_now.els[k] ]);

    gdk_draw_line (drawable, gg->plot_GC,
      sp->screen[a].x, sp->screen[a].y,
      sp->screen[b].x, sp->screen[b].y);

    gdk_gc_set_line_attributes (gg->plot_GC,
      0, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
    gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
  }
}

void
splot_add_edge_label (splotd *sp, GdkDrawable *drawable, gint k,
  gboolean nearest, ggobid *gg)
{
  gchar *lbl;
  displayd *dsp = (displayd *) sp->displayptr;
  datad *d = dsp->d;
  datad *e = dsp->e;
  gint lbearing, rbearing, width, ascent, descent;
  GtkStyle *style = gtk_widget_get_style (sp->da);
  gint xp, yp;
  gint a, b;
  endpointsd *endpoints;


  gboolean draw_edge = (dsp->options.edges_undirected_show_p ||
                        dsp->options.edges_directed_show_p);

  endpoints = resolveEdgePoints(e, d);
  draw_edge = draw_edge && edge_endpoints_get (k, &a, &b, d, endpoints, e);

/*
 * Do we distinguish between nearest and sticky edge labels?
*/
  if (draw_edge) {

    /*-- add the label last so it will be in front of other markings --*/
    lbl = identify_label_fetch (k, &dsp->cpanel, e, gg);
    splot_text_extents (lbl, style, 
      &lbearing, &rbearing, &width, &ascent, &descent);

    if (sp->screen[a].x > sp->screen[b].x) {gint itmp=b; b=a; a=itmp;}
    xp = (sp->screen[b].x - sp->screen[a].x)/2 + sp->screen[a].x;
    if (sp->screen[a].y > sp->screen[b].y) {gint itmp=b; b=a; a=itmp;}
    yp = (sp->screen[b].y - sp->screen[a].y)/2 + sp->screen[a].y;

    splot_draw_string (lbl, xp, yp, style, drawable, gg);

    if (nearest) {
      /*-- underline the label --*/
      gdk_draw_line (drawable, gg->plot_GC,
        xp, yp+1, xp+width, yp+1);

      /*-- display it in the top center of the window --*/
      splot_draw_string (lbl,
        (sp->max.x - width)/2,
        ascent + descent + 5,  /*-- the border is of width 3 --*/
        style, drawable, gg);
      /*-- underline it there, too, for consistency --*/
      gdk_draw_line (drawable, gg->plot_GC,
        (sp->max.x - width)/2, ascent + descent + 5 + 1,
        (sp->max.x - width)/2 + width, ascent + descent + 5 + 1);
    }
  }
}

void
splot_add_identify_edge_cues (splotd *sp, GdkDrawable *drawable, gint k,
  gboolean nearest, ggobid *gg)
{
  displayd *dsp = (displayd *) sp->displayptr;
  datad *e = dsp->e;
  gboolean useDefault = false;

  if (k >= e->edge.n) return;

  /*-- not yet using rowid.idv  -- huh? --*/
  if (e->hidden_now.els[k]) return;

  if (GTK_IS_GGOBI_EXTENDED_SPLOT(sp)) {
    GtkGGobiExtendedSPlotClass *klass;
    klass = GTK_GGOBI_EXTENDED_SPLOT_CLASS(GTK_OBJECT(sp)->klass);
    if(klass->add_identify_edge_cues)
      klass->add_identify_edge_cues(k, sp, drawable, nearest, gg);
    else 
      useDefault = true;
  }

  if (useDefault) {
    splot_add_edge_highlight_cue (sp, drawable, k, nearest, gg);
    splot_add_edge_label (sp, drawable, k, nearest, gg);
  }
}

void
splot_add_edgeedit_cues (splotd *sp, GdkDrawable *drawable,
  gint k, gboolean nearest, ggobid *gg)
{
  displayd *display = sp->displayptr;
  cpaneld *cpanel = &display->cpanel;

  if (cpanel->ee_adding_p) {
    if (k != -1)
      splot_add_diamond_cue (k, sp, drawable, gg);

    /*-- when the button is up, we're choosing the starting point --*/

    if (gg->buttondown) {   /*-- button is down: choosing end point --*/
      if (k != -1 && k != gg->edgeedit.a) {
        gdk_draw_line (drawable, gg->plot_GC,
          sp->screen[ gg->edgeedit.a ].x, sp->screen[ gg->edgeedit.a ].y,
          sp->screen[ k ].x, sp->screen[ k ].y);
      } else {
        gdk_draw_line (drawable, gg->plot_GC,
          sp->screen[ gg->edgeedit.a ].x, sp->screen[ gg->edgeedit.a ].y,
          sp->mousepos.x, sp->mousepos.y);
      }
    }
  } else if (cpanel->ee_deleting_p) {
  }
}
