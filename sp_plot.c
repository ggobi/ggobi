/* sp_plot.c */
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

static void splot_draw_border (splotd *, GdkDrawable *, ggobid *);
static void edges_draw (splotd *, GdkDrawable *, ggobid *gg);
static void splot_nearest_edge_highlight (splotd *, gint, gboolean nearest, ggobid *);

static void splot_draw_tour_axes(splotd *, GdkDrawable *, ggobid *);

#ifdef WIN32
extern void win32_draw_to_pixmap_binned (icoords *, icoords *, gint, splotd *, ggobid *gg);
extern void win32_draw_to_pixmap_unbinned (gint, splotd *, ggobid *gg);
#endif

/* colors_used now contains integers, 0:ncolors-1 */
void
splot_colors_used_get (splotd *sp, gint *ncolors_used,
  gushort *colors_used, datad *d, ggobid *gg) 
{
  gboolean new_color;
  gint i, k, m;

  *ncolors_used = 0;

  if (d == NULL || d->nrows == 0)
/**/return;
          
  /*
   * Loop once through d->color[], collecting the colors currently
   * in use into the colors_used[] vector.
  */
  for (i=0; i<d->nrows_in_plot; i++) {
    m = d->rows_in_plot[i];
    if (d->hidden_now.els[m]) {  /*-- if it's hidden, we don't care --*/
      new_color = false;
    } else {
      new_color = true;
      for (k=0; k<*ncolors_used; k++) {
        if (colors_used[k] == d->color_now.els[m]) {
          new_color = false;
          break;
        }
      }
    }
    if (new_color) {
      colors_used[*ncolors_used] = d->color_now.els[m];
      (*ncolors_used)++;
    }
  }

  /*
   * Make sure that the current brushing color is
   * last in the list, so that it is drawn on top of
   * the pile of points.
  */
  for (k=0; k<(*ncolors_used-1); k++) {
    if (colors_used[k] == gg->color_id) {
      colors_used[k] = colors_used[*ncolors_used-1];
      colors_used[*ncolors_used-1] = gg->color_id;
      break;
    }
  }

  /* insurance -- eg if using mono drawing on a color screen */
  if (*ncolors_used == 0) {
    *ncolors_used = 1;
    colors_used[0] = d->color_now.els[0];
  }
}

gboolean
splot_plot_case (gint m, datad *d, splotd *sp, displayd *display, ggobid *gg)
{
  gboolean draw_case;

  /*-- determine whether case m should be plotted --*/
  draw_case = true;
  if (d->hidden_now.els[m]) {
    draw_case = false;
/*
 *} else if (d->color_now.els[m] >= gg->ncolors) {
 *  draw_case = false;
*/

  /*-- can prevent drawing of missings for parcoords or scatmat plots --*/
  } else if (!display->options.missings_show_p && d->nmissing > 0) {
    switch (display->displaytype) {
      case parcoords:
        if (d->missing.vals[m][sp->p1dvar])
          draw_case = false;
      break;

      case scatmat:
        if (sp->p1dvar != -1) {
          if (d->missing.vals[m][sp->p1dvar])
            draw_case = false;
        } else {
          if (d->missing.vals[m][sp->xyvars.x] ||
              d->missing.vals[m][sp->xyvars.y])
          {
            draw_case = false;
          }
        }
      break;

      case tsplot:
        if (d->missing.vals[m][sp->xyvars.y]|| 
            d->missing.vals[m][sp->xyvars.x])
          draw_case = false;
      break;

      case scatterplot:
      {
        gint proj = projection_get (gg);
        switch (proj) {
          case P1PLOT:
            if (d->missing.vals[m][sp->p1dvar])
              draw_case = false;
          break;
          case XYPLOT:
            if (d->missing.vals[m][sp->xyvars.x])
              draw_case = false;
            else if (d->missing.vals[m][sp->xyvars.y])
              draw_case = false;
          break;
          case TOUR1D:
            if (d->missing.vals[m][display->t1d.vars.els[m]])
              draw_case = false;
          break;

          case TOUR2D:
            if (d->missing.vals[m][display->t2d.vars.els[m]])
              draw_case = false;
          break;

          case COTOUR:
            if (d->missing.vals[m][display->tcorr1.vars.els[m]])
              draw_case = false;
            else if (d->missing.vals[m][display->tcorr2.vars.els[m]])
              draw_case = false;
          break;
        }
      }
      break;
      default:
      break;
    }
  }
  return draw_case;
}

/*------------------------------------------------------------------------*/
/*               drawing to pixmap0 when binning can't be used            */
/*------------------------------------------------------------------------*/

void
splot_draw_to_pixmap0_unbinned (splotd *sp, ggobid *gg)
{
  gint k;
#ifndef WIN32
  gint i, m, n;
#endif
  gushort current_color;
  gint ncolors_used;
  gushort colors_used[MAXNCOLORS+2];
  GtkWidget *da = sp->da;
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;
  gboolean draw_case;
  gint dtype = display->displaytype;

  /*
   * since parcoords and tsplot each have their own weird way
   * of drawing line segments, it's necessary to get the point
   * colors before drawing those lines even if we're not drawing
   * points.
  */
  gboolean loop_over_points =
    display->options.points_show_p || dtype == parcoords || dtype == tsplot;

  if (gg->plot_GC == NULL) {
    init_plot_GC (sp->pixmap0, gg);
  }

  /* clear the pixmap */
  gdk_gc_set_foreground (gg->plot_GC, &gg->bg_color);
  gdk_draw_rectangle (sp->pixmap0, gg->plot_GC,
                      TRUE, 0, 0,
                      da->allocation.width,
                      da->allocation.height);

/*
 * Draw edges under points
*/
  if (display->options.edges_undirected_show_p ||
      display->options.edges_arrowheads_show_p ||
      display->options.edges_directed_show_p)
  {
    if (dtype == scatterplot || dtype == scatmat)
      edges_draw (sp, sp->pixmap0, gg);
  }

  if (!gg->mono_p && loop_over_points) {
    splot_colors_used_get (sp, &ncolors_used, colors_used, d, gg);

    /*
     * Now loop through colors_used[], plotting the points of each
     * color.  This avoids the need to reset the foreground so often.
     * On the other hand, it requires more looping.
    */
    for (k=0; k<ncolors_used; k++) {
      current_color = colors_used[k];
      gdk_gc_set_foreground (gg->plot_GC, &gg->color_table[current_color]);

#ifdef WIN32
      win32_draw_to_pixmap_unbinned (current_color, sp, gg);
#else
      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot[i];
        draw_case = splot_plot_case (m, d, sp, display, gg);

        if (draw_case && d->color_now.els[m] == current_color) {
          if (display->options.points_show_p) {
            draw_glyph (sp->pixmap0, &d->glyph_now.els[m], sp->screen, m, gg);
          }

          /*-- whiskers: parallel coordinate and time series plots --*/
          if (dtype == parcoords || dtype == tsplot)
          {
            if (display->options.whiskers_show_p) {
              if (dtype == parcoords) {
                n = 2*m;
                gdk_draw_line (sp->pixmap0, gg->plot_GC,
                  sp->whiskers[n].x1, sp->whiskers[n].y1,
                  sp->whiskers[n].x2, sp->whiskers[n].y2);
                n++;
                gdk_draw_line (sp->pixmap0, gg->plot_GC,
                  sp->whiskers[n].x1, sp->whiskers[n].y1,
                  sp->whiskers[n].x2, sp->whiskers[n].y2);
              }
              else { /*-- if time series plot --*/
                if (m < d->nrows_in_plot-1)  /*-- there are n-1 whiskers --*/
                  gdk_draw_line (sp->pixmap0, gg->plot_GC,
                    sp->whiskers[m].x1, sp->whiskers[m].y1,
                    sp->whiskers[m].x2, sp->whiskers[m].y2);
              }
            }
          }
        }
      }
#endif
    }  /* deal with mono later */
  }

/*
moving this to the end of the routine that adds markup
  cpaneld *cpanel = &display->cpanel;
  gint proj = cpanel->projection;
  if (proj == TOUR1D || proj == TOUR2D || proj == COTOUR) {
    splot_draw_tour_axes(sp, sp->pixmap0, gg);
  }
*/
  return;
}

void
splot_draw_to_pixmap0_binned (splotd *sp, ggobid *gg)
{
  icoords loc_clear0, loc_clear1;
#ifndef WIN32
  gint ih, iv;
  gint i, m, n;
#endif
  gint k;
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  gint proj = cpanel->projection;
  datad *d = display->d;
  icoords *bin0 = &gg->plot.bin0;
  icoords *bin1 = &gg->plot.bin1;
  icoords *loc0 = &gg->plot.loc0;
  icoords *loc1 = &gg->plot.loc1;

  gushort current_color;
  gint ncolors_used;
  gushort colors_used[MAXNCOLORS+2];

  if (gg->plot_GC == NULL)
    init_plot_GC (sp->pixmap0, gg);

/*
 * Instead of clearing and redrawing the entire pixmap0, only
 * clear what's necessary.
*/

  get_extended_brush_corners (bin0, bin1, d, sp);

/*
 * Determine locations of bin corners: upper left edge of loc0;
 * lower right edge of loc1.
*/
  loc0->x = (gint)
    ((gfloat) bin0->x / (gfloat) d->brush.nbins * (sp->max.x+1.0));
  loc0->y = (gint)
    ((gfloat) bin0->y / (gfloat) d->brush.nbins * (sp->max.y+1.0));
  loc1->x = (gint)
    ((gfloat) (bin1->x+1) / (gfloat) d->brush.nbins * (sp->max.x+1.0));
  loc1->y = (gint)
    ((gfloat) (bin1->y+1) / (gfloat) d->brush.nbins * (sp->max.y+1.0));

/*
 * Clear an area a few pixels inside that region.  Watch out
 * for border effects.
*/
  loc_clear0.x = (bin0->x == 0) ? 0 : loc0->x + BRUSH_MARGIN;
  loc_clear0.y = (bin0->y == 0) ? 0 : loc0->y + BRUSH_MARGIN;
  loc_clear1.x = (bin1->x == d->brush.nbins-1) ? sp->max.x :
                                               loc1->x - BRUSH_MARGIN;
  loc_clear1.y = (bin1->y == d->brush.nbins-1) ? sp->max.y :
                                               loc1->y - BRUSH_MARGIN;

  gdk_gc_set_foreground (gg->plot_GC, &gg->bg_color);
  gdk_draw_rectangle (sp->pixmap0, gg->plot_GC,
                      true,  /* fill */
                      loc_clear0.x, loc_clear0.y,
                      1 + loc_clear1.x - loc_clear0.x ,
                      1 + loc_clear1.y - loc_clear0.y);

  if (display->options.points_show_p) {
    if (!gg->mono_p) {

      splot_colors_used_get (sp, &ncolors_used, colors_used, d, gg); 

      /*
       * Now loop through colors_used[], plotting the points of each
       * color in a group.
      */
      for (k=0; k<ncolors_used; k++) {
        current_color = colors_used[k];
        gdk_gc_set_foreground (gg->plot_GC, &gg->color_table[current_color]);

#ifdef WIN32
        win32_draw_to_pixmap_binned (bin0, bin1, current_color, sp, gg);
#else
        for (ih=bin0->x; ih<=bin1->x; ih++) {
          for (iv=bin0->y; iv<=bin1->y; iv++) {
            for (m=0; m<d->brush.binarray[ih][iv].nels; m++) {
              i = d->rows_in_plot[d->brush.binarray[ih][iv].els[m]];
              if (!d->hidden_now.els[i] &&
                   d->color_now.els[i] == current_color)
              {
                draw_glyph (sp->pixmap0, &d->glyph_now.els[i],
                  sp->screen, i, gg);

                /* parallel coordinate plot whiskers */
                if (display->displaytype == parcoords) {
                  if (display->options.whiskers_show_p) {
                    n = 2*i;
                    gdk_draw_line (sp->pixmap0, gg->plot_GC,
                      sp->whiskers[n].x1, sp->whiskers[n].y1,
                      sp->whiskers[n].x2, sp->whiskers[n].y2);
                    n++;
                    gdk_draw_line (sp->pixmap0, gg->plot_GC,
                      sp->whiskers[n].x1, sp->whiskers[n].y1,
                      sp->whiskers[n].x2, sp->whiskers[n].y2);
                  }
                } else if(display->displaytype == tsplot) {
                  gdk_draw_line (sp->pixmap0, gg->plot_GC,
                    sp->whiskers[m].x1, sp->whiskers[m].y1,
                    sp->whiskers[m].x2, sp->whiskers[m].y2);
                }
              }
            }
          }
        }
#endif
      }
    }
  }

  if (proj == TOUR1D || proj == TOUR2D || proj == COTOUR) {
    splot_draw_tour_axes(sp, sp->pixmap0, gg);
  }

  return;
}


/*------------------------------------------------------------------------*/
/*                   plot labels: variables                               */
/*------------------------------------------------------------------------*/

static void
splot_add_plot_labels (splotd *sp, GdkDrawable *drawable, ggobid *gg) {
  gint lbearing, rbearing, width, ascent, descent;
  GtkStyle *style = gtk_widget_get_style (sp->da);
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  gint dtype = display->displaytype;
  datad *d = display->d;
  vartabled *vt, *vtx, *vty;

  gboolean proceed = (cpanel->projection == XYPLOT ||
                      cpanel->projection == P1PLOT ||
                      cpanel->projection == PCPLOT ||
                      cpanel->projection == TSPLOT ||
                      cpanel->projection == SCATMAT);
  if (!proceed)
    return;

  gdk_gc_set_foreground (gg->plot_GC, &gg->accent_color);

  if (dtype == scatterplot || dtype == scatmat) {
    if ((dtype == scatterplot && cpanel->projection == XYPLOT) ||
        (dtype == scatmat && sp->p1dvar == -1))
    {
      /*-- xyplot: right justify the label --*/
      vtx = vartable_element_get (sp->xyvars.x, d);
      gdk_text_extents (style->font, 
        vtx->collab_tform, strlen (vtx->collab_tform),
        &lbearing, &rbearing, &width, &ascent, &descent);
      gdk_draw_string (drawable, style->font, gg->plot_GC,
        sp->max.x - width - 5,  /*-- right justify --*/
        sp->max.y - 5,
        vtx->collab_tform);

      vty = vartable_element_get (sp->xyvars.y, d);
      gdk_text_extents (style->font, 
        vty->collab_tform, strlen (vty->collab_tform),
        &lbearing, &rbearing, &width, &ascent, &descent);
      gdk_draw_string (drawable, style->font, gg->plot_GC,
        5, 5 + ascent + descent,
        vty->collab_tform);
    }

    /*-- 1dplot: center the label --*/
    if ((dtype == scatterplot && cpanel->projection == P1PLOT) ||
        (dtype == scatmat && sp->p1dvar != -1))
    {
      vt = vartable_element_get (sp->p1dvar, d);
      gdk_text_extents (style->font,
        vt->collab_tform, strlen (vt->collab_tform),
        &lbearing, &rbearing, &width, &ascent, &descent);
      gdk_draw_string (drawable, style->font, gg->plot_GC,
        sp->max.x/2 - width/2,  /*-- center --*/
        sp->max.y - 5,
        vt->collab_tform);
    }

  } else if (dtype == parcoords) {

    vt = vartable_element_get (sp->p1dvar, d);
    gdk_text_extents (style->font,
      vt->collab_tform, strlen (vt->collab_tform),
      &lbearing, &rbearing, &width, &ascent, &descent);
    gdk_draw_string (drawable, style->font, gg->plot_GC,
      sp->max.x/2 - width/2,  /*-- center --*/
      sp->max.y - 5,
      vt->collab_tform);

  } else if (dtype == tsplot) {

    GList *l = display->splots;
    if (l->data == sp) {
      vtx = vartable_element_get (sp->xyvars.x, d);
      gdk_text_extents (style->font, 
        vtx->collab_tform, strlen (vtx->collab_tform),
        &lbearing, &rbearing, &width, &ascent, &descent);
      gdk_draw_string (drawable, style->font, gg->plot_GC,
        sp->max.x - width - 5,
        sp->max.y - 5,
        vtx->collab_tform);
    }
    vty = vartable_element_get (sp->xyvars.y, d);
    gdk_text_extents (style->font, 
      vty->collab_tform, strlen (vty->collab_tform),
      &lbearing, &rbearing, &width, &ascent, &descent);
    gdk_draw_string (drawable, style->font, gg->plot_GC,
      5, 5 + ascent + descent,
      vty->collab_tform);
  }

}

/*------------------------------------------------------------------------*/
/*               case highlighting for points (and edges?)                */
/*------------------------------------------------------------------------*/

/*-- add highlighting for parallel coordinates plot --*/
static void
splot_add_whisker_cues (gint k, splotd *sp, GdkDrawable *drawable, ggobid *gg)
{
  gint n;
  displayd *display = sp->displayptr;
  datad *d = display->d;

  if (k < 0 || k >= d->nrows) return;

  if (display->options.whiskers_show_p) {
    gdk_gc_set_line_attributes (gg->plot_GC,
      3, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
    gdk_gc_set_foreground (gg->plot_GC, &gg->color_table[d->color_now.els[k]]);

    n = 2*k;
    gdk_draw_line (drawable, gg->plot_GC,
      sp->whiskers[n].x1, sp->whiskers[n].y1,
      sp->whiskers[n].x2, sp->whiskers[n].y2);
    n++;
    gdk_draw_line (drawable, gg->plot_GC,
      sp->whiskers[n].x1, sp->whiskers[n].y1,
      sp->whiskers[n].x2, sp->whiskers[n].y2);

    gdk_gc_set_line_attributes (gg->plot_GC,
      0, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
  }
}

#define DIAMOND_DIM 5

/*-- draw a diamond around the current case --*/
static void
splot_add_diamond_cue (gint k, splotd *sp, GdkDrawable *drawable, ggobid *gg)
{
  datad *d = sp->displayptr->d;
  gint diamond_dim = DIAMOND_DIM;
  GdkPoint diamond[5];

  if (k < 0 || k >= d->nrows) return;

  diamond[0].x = diamond[4].x = sp->screen[k].x - diamond_dim;
  diamond[0].y = diamond[4].y = sp->screen[k].y;
  diamond[1].x = sp->screen[k].x;
  diamond[1].y = sp->screen[k].y - diamond_dim;
  diamond[2].x = sp->screen[k].x + diamond_dim;
  diamond[2].y = sp->screen[k].y;
  diamond[3].x = sp->screen[k].x;
  diamond[3].y = sp->screen[k].y + diamond_dim;

  gdk_gc_set_foreground (gg->plot_GC, &gg->accent_color);
  gdk_draw_lines (drawable, gg->plot_GC, diamond, 5);
}

/*-- add the label to a case (with or without underlining) --*/
static void
splot_add_record_label (gboolean nearest, gint k, splotd *sp,
  GdkDrawable *drawable, ggobid *gg)
{
  displayd *dsp = sp->displayptr;
  cpaneld *cpanel = &dsp->cpanel;
  gint proj = cpanel->projection;
  datad *d = dsp->d;
  gint j;
  gint lbearing, rbearing, width, ascent, descent;
  GtkStyle *style = gtk_widget_get_style (sp->da);
  gint diamond_dim = DIAMOND_DIM;
  gchar *lbl;

  if (k < 0 || k >= d->nrows) return;

  /*-- add the label last so it will be in front of other markings --*/
  if (cpanel->identify_display_type == ID_CASE_LABEL)
    lbl = (gchar *) g_array_index (d->rowlab, gchar *, k);
  else {
    switch (proj) {
      case P1PLOT:
        lbl = g_strdup_printf ("%g", d->tform.vals[k][sp->p1dvar]);
      break;
      case XYPLOT:
        lbl = g_strdup_printf ("%g, %g",
          d->tform.vals[k][sp->xyvars.x], d->tform.vals[k][sp->xyvars.y]);
      break;

      case TOUR1D:
        for (j=0; j<dsp->t1d.nvars; j++) {
          if (j == 0)
            lbl = g_strdup_printf ("%g",
              d->tform.vals[k][dsp->t1d.vars.els[j]]);
          else
            lbl = g_strdup_printf ("%s, %g", lbl,
              d->tform.vals[k][dsp->t1d.vars.els[j]]);
        }
      break;

      case TOUR2D:
        for (j=0; j<dsp->t2d.nvars; j++) {
          if (j == 0)
            lbl = g_strdup_printf ("%g",
              d->tform.vals[k][dsp->t2d.vars.els[j]]);
          else
            lbl = g_strdup_printf ("%s, %g", lbl,
              d->tform.vals[k][dsp->t2d.vars.els[j]]);
        }
      break;

      case COTOUR:
        for (j=0; j<dsp->tcorr1.nvars; j++) {
          if (j == 0)
            lbl = g_strdup_printf ("%g",
              d->tform.vals[k][dsp->tcorr1.vars.els[j]]);
          else
            lbl = g_strdup_printf ("%s, %g", lbl,
              d->tform.vals[k][dsp->tcorr1.vars.els[j]]);
        }
        for (j=0; j<dsp->tcorr2.nvars; j++) {
          if (j == 0)
            lbl = g_strdup_printf ("%s; %g", lbl,
              d->tform.vals[k][dsp->tcorr2.vars.els[j]]);
          else
            lbl = g_strdup_printf ("%s, %g", lbl,
              d->tform.vals[k][dsp->tcorr2.vars.els[j]]);
        }
      break;
    }
  }
  gdk_text_extents (style->font, lbl, strlen (lbl),
    &lbearing, &rbearing, &width, &ascent, &descent);

  /*-- underline the nearest point label?  --*/
  if (sp->screen[k].x <= sp->max.x/2) {
    gdk_draw_string (drawable, style->font, gg->plot_GC,
      sp->screen[k].x+diamond_dim,
      sp->screen[k].y-diamond_dim,
      lbl);
    if (nearest)
      gdk_draw_line (drawable, gg->plot_GC,
        sp->screen[k].x+diamond_dim, sp->screen[k].y-diamond_dim+1,
        sp->screen[k].x+diamond_dim+width, sp->screen[k].y-diamond_dim+1);
      
  } else {
    gdk_draw_string (drawable, style->font, gg->plot_GC,
      sp->screen[k].x - width - diamond_dim,
      sp->screen[k].y - diamond_dim,
      lbl);
    if (nearest)
      gdk_draw_line (drawable, gg->plot_GC,
        sp->screen[k].x - width - diamond_dim, sp->screen[k].y - diamond_dim+1,
        sp->screen[k].x - diamond_dim, sp->screen[k].y - diamond_dim+1);
  }

}

/*-- add the nearest_point and sticky labels, plus a diamond for emphasis --*/
void
splot_add_identify_cues (splotd *sp, GdkDrawable *drawable,
  gint k, gboolean nearest, ggobid *gg)
{
  displayd *dsp = (displayd *) sp->displayptr;

  if (nearest) {
    if (dsp->displaytype == parcoords) {
      splot_add_whisker_cues (k, sp, drawable, gg);
    } else {  /*-- for all displays other than the parcoords plot --*/
      splot_add_diamond_cue (k, sp, drawable, gg);
    }
  }

  gdk_gc_set_foreground (gg->plot_GC, &gg->accent_color);
  splot_add_record_label (nearest, k, sp, drawable, gg);
}

void
splot_add_movepts_cues (splotd *sp, GdkDrawable *drawable,
  gint k, gboolean nearest, ggobid *gg)
{
  displayd *dsp = (displayd *) sp->displayptr;
  datad *d = dsp->d;

  if (k < 0 || k >= d->nrows)
    return;

  splot_add_diamond_cue (k, sp, drawable, gg);

  /*-- only add the label if the mouse is up --*/
  if (!gg->buttondown) {
    splot_add_record_label (nearest, k, sp, drawable, gg);
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

static void
splot_add_point_cues (splotd *sp, GdkDrawable *drawable, ggobid *gg) {
  gint id;
  GSList *l;
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;
  PipelineMode mode = viewmode_get (gg);

  /*
     these are the cues added to
       nearest point and sticky guys    in identification
       nearest point                    in moving points
       source, maybe dest and edge      in edge editing
  */
  if (mode == IDENT && d->nearest_point != -1)
    splot_add_identify_cues (sp, drawable, d->nearest_point, true, gg);
  else if (mode == MOVEPTS)
     splot_add_movepts_cues (sp, drawable, d->nearest_point, true, gg);
  else if (mode == EDGEED)
    splot_add_edgeedit_cues (sp, drawable, d->nearest_point, true, gg);
 

  /*-- and these are the sticky ids, added in all modes --*/
  if (d->sticky_ids != NULL &&
      g_slist_length (d->sticky_ids) > 0)
  {
    for (l = d->sticky_ids; l; l = l->next) {
      id = GPOINTER_TO_INT (l->data);
      /*-- false = !nearest --*/
      splot_add_identify_cues (sp, drawable, id, false, gg);
    }
  }
}


/*------------------------------------------------------------------------*/
/*                   line drawing routines                                */
/*------------------------------------------------------------------------*/

/*-- the current color and line type need to be drawn last --*/

void
edges_draw (splotd *sp, GdkDrawable *drawable, ggobid *gg)
{
  gint i, j, m;
  gint k, n, p;
  gint a, b;
  gboolean doit;
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;
  datad *e = display->e;
  endpointsd *endpoints;
  gboolean edges_show_p, arrowheads_show_p;
  gint nels = d->rowid.idv.nels;
  gint ltype, gtype;

  if (e == (datad *) NULL || e->edge.n == 0) {
/**/return;
  }

  if (nels == 0) {  /*-- d has no record ids --*/
/**/return;
  }

  edges_show_p = (display->options.edges_directed_show_p ||
                  display->options.edges_undirected_show_p);
  arrowheads_show_p = (display->options.edges_directed_show_p ||
                       display->options.edges_arrowheads_show_p);
  if (!gg->mono_p && (edges_show_p || arrowheads_show_p)) {

    gint symbols_used[NGLYPHSIZES][NEDGETYPES][MAXNCOLORS];
    gint nl = 0;
    gint ncolors = MIN(MAXNCOLORS, gg->ncolors);
    gint k_prev = -1, n_prev = -1, p_prev = -1;

    endpoints = e->edge.endpoints;

    for (k=0; k<NGLYPHSIZES; k++)
      for (n=0; n<NEDGETYPES; n++)
        for (p=0; p<ncolors; p++)
          symbols_used[k][n][p] = 0;

    /*
     * Use e->color_now[] and e->glyph_now[] to work out which
     * line symbols should be drawn
    */
    for (i=0; i<e->nrows_in_plot; i++) {
      m = e->rows_in_plot[i];
      if (!e->hidden_now.els[m]) {  /*-- if it's hidden, we don't care --*/
        gtype = e->glyph_now.els[m].type;
        if (gtype == FILLED_CIRCLE || gtype == FILLED_RECTANGLE)
          ltype = SOLID;
        else if (gtype == OPEN_CIRCLE || gtype == OPEN_RECTANGLE)
          ltype = WIDE_DASH;
        else ltype = NARROW_DASH;

        symbols_used[e->glyph_now.els[m].size][ltype][e->color_now.els[m]]++;
      }
    }


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
              doit = true;
              if (e->hidden_now.els[j]) {
                doit = false;
              } else if (endpoints[j].a >= nels || endpoints[j].b >= nels) {
                doit = false;
              }

              if (doit) {
                a = d->rowid.idv.els[endpoints[j].a];
                b = d->rowid.idv.els[endpoints[j].b];
                doit = (!d->hidden_now.els[a] && !d->hidden_now.els[b]);

                /* If not plotting imputed values, and one is
                 * missing, skip it */
/*
          if (!plot_imputed_values && plotted_var_missing(from, to, gg))
            doit = false;
*/
              }

              if (doit) {

                gtype = e->glyph_now.els[j].type;
                if (gtype == FILLED_CIRCLE || gtype == FILLED_RECTANGLE)
                  ltype = SOLID;
                else if (gtype == OPEN_CIRCLE || gtype == OPEN_RECTANGLE)
                  ltype = WIDE_DASH;
                else ltype = NARROW_DASH;

                if (e->color_now.els[j] == p &&
                    ltype == n &&
                    e->glyph_now.els[j].size == k)
                {

                  if (edges_show_p) {
                    sp->edges[nl].x1 = sp->screen[a].x;
                    sp->edges[nl].y1 = sp->screen[a].y;
                    sp->edges[nl].x2 = sp->screen[b].x;
                    sp->edges[nl].y2 = sp->screen[b].y;
                  }

                  if (arrowheads_show_p) {
                    /*
                     * Add thick piece of the lines to suggest a
                     * directional arrow
                    */
                    sp->arrowheads[nl].x1 =
                      (gint) (.2*sp->screen[a].x + .8*sp->screen[b].x);
                    sp->arrowheads[nl].y1 =
                      (gint) (.2*sp->screen[a].y + .8*sp->screen[b].y);
                    sp->arrowheads[nl].x2 = sp->screen[b].x;
                    sp->arrowheads[nl].y2 = sp->screen[b].y;
                  }
                  nl++;
                }
              }
            }  /*-- end of looping through edges --*/

            if (p_prev == -1 || p_prev != p) {  /* color */
              gdk_gc_set_foreground (gg->plot_GC, &gg->color_table[p]);
            }

            if (edges_show_p) {
              gint lwidth = (k<3) ? 0 : (k-2)*2;
              gchar dash_list[2];
              gint ltype;

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
                3, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
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
splot_nearest_edge_highlight (splotd *sp, gint k, gboolean nearest, ggobid *gg) {
  displayd *dsp = (displayd *) sp->displayptr;
  datad *d = dsp->d;
  datad *e = dsp->e;
  gint lbearing, rbearing, width, ascent, descent;
  GtkStyle *style = gtk_widget_get_style (sp->da);
  gchar *lbl;
  gint xp, yp;
  gboolean draw_edge;

  if (k >= e->edge.n)
    return;

  /*-- not yet using rowid.idv --*/
  if (e->hidden_now.els[k])
    return;

  draw_edge = (dsp->options.edges_undirected_show_p ||
               dsp->options.edges_directed_show_p);

  /*-- draw a thickened line, and add a label if nearest==true --*/
  if (draw_edge) {
    endpointsd *endpoints = e->edge.endpoints;
    gint a = d->rowid.idv.els[endpoints[k].a];
    gint b = d->rowid.idv.els[endpoints[k].b];

    gdk_gc_set_line_attributes (gg->plot_GC,
      3, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
    gdk_gc_set_foreground (gg->plot_GC, &gg->color_table[e->color_now.els[k]]);

    gdk_draw_line (sp->pixmap1, gg->plot_GC,
      sp->screen[a].x, sp->screen[a].y,
      sp->screen[b].x, sp->screen[b].y);

    gdk_gc_set_line_attributes (gg->plot_GC,
      0, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
    gdk_gc_set_foreground (gg->plot_GC, &gg->accent_color);

    if (nearest) {
      /*-- add the label last so it will be in front of other markings --*/
      lbl = (gchar *) g_array_index (e->rowlab, gchar *, k);
      gdk_text_extents (style->font,
        lbl, strlen (lbl),
        &lbearing, &rbearing, &width, &ascent, &descent);

      if (sp->screen[a].x > sp->screen[b].x) {gint itmp=b; b=a; a=itmp;}
      xp = (sp->screen[b].x - sp->screen[a].x)/2 + sp->screen[a].x;
      if (sp->screen[a].y > sp->screen[b].y) {gint itmp=b; b=a; a=itmp;}
      yp = (sp->screen[b].y - sp->screen[a].y)/2 + sp->screen[a].y;

      /*-- underline the nearest point label?  --*/
      gdk_draw_string (sp->pixmap1, style->font, gg->plot_GC,
        xp, yp, lbl);
      if (nearest)
        gdk_draw_line (sp->pixmap1, gg->plot_GC,
          xp, yp+1,
          xp+width, yp+1);
    }
  }
}

/*------------------------------------------------------------------------*/
/*                 draw the border indicating current splot               */
/*------------------------------------------------------------------------*/

static void
splot_draw_border (splotd *sp, GdkDrawable *drawable, ggobid *gg)
{
  if (sp != NULL && sp->da != NULL && sp->da->window != NULL) {
    gdk_gc_set_foreground (gg->plot_GC, &gg->accent_color);
    gdk_gc_set_line_attributes (gg->plot_GC,
      3, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);

    gdk_draw_rectangle (drawable, gg->plot_GC,
      FALSE, 1, 1, sp->da->allocation.width-3, sp->da->allocation.height-3);

    gdk_gc_set_line_attributes (gg->plot_GC,
      0, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
  }
}


/*------------------------------------------------------------------------*/
/*    getting from pixmap0 to pixmap1, then pixmap1 to the window         */
/*------------------------------------------------------------------------*/

void
splot_pixmap0_to_pixmap1 (splotd *sp, gboolean binned, ggobid *gg) {
  GtkWidget *w = sp->da;
  icoords *loc0 = &gg->plot.loc0;
  icoords *loc1 = &gg->plot.loc1;

  if (!binned) {
    gdk_draw_pixmap (sp->pixmap1, gg->plot_GC, sp->pixmap0,
                     0, 0, 0, 0,
                     w->allocation.width,
                     w->allocation.height);
  }
  else {
    gdk_draw_pixmap (sp->pixmap1, gg->plot_GC, sp->pixmap0,
                      loc0->x, loc0->y,
                      loc0->x, loc0->y,
                      1 + loc1->x - loc0->x, 1 + loc1->y - loc0->y);
  }
}

static void
splot_add_markup_to_pixmap (splotd *sp, GdkDrawable *drawable, ggobid *gg)
{
  displayd *display = (displayd *) sp->displayptr;
  datad *e = display->e;
  datad *d = display->d;
  cpaneld *cpanel = &display->cpanel;
  gint displaytype = display->displaytype;
  gint proj = cpanel->projection;

/*-- moving this section breaks splot_redraw (QUICK) for adding edges --*/
  if (display->options.edges_undirected_show_p ||
      display->options.edges_arrowheads_show_p ||
      display->options.edges_directed_show_p)
  {
    if (displaytype == scatterplot || displaytype == scatmat) {
      if (e->nearest_point != -1)
        splot_nearest_edge_highlight (sp, e->nearest_point, true, gg);
    }
  }
     
  splot_add_plot_labels (sp, drawable, gg);  /*-- axis labels --*/

  /*-- identify, move points, edge editing --*/
  splot_add_point_cues (sp, drawable, gg);  

  if (sp == gg->current_splot)
    splot_draw_border (sp, drawable, gg);

  /*-- draw these cues whether this is the current display or not --*/
  if (g_list_length (display->splots) == 1  /*-- scatterplot --*/
      || sp == display->current_splot)  /*-- ... in a multi-plot display --*/
  {
    if (cpanel->viewmode == BRUSH) {
      brush_draw_brush (sp, drawable, d, gg);
      brush_draw_label (sp, drawable, d, gg);
    } else if (cpanel->viewmode == SCALE) {
      scaling_visual_cues_draw (sp, drawable, gg);
    }
  }

  if (proj == TOUR1D || proj == TOUR2D || proj == COTOUR) {
    splot_draw_tour_axes(sp, drawable, gg);
  }
}


void
splot_pixmap_to_window (splotd *sp, GdkPixmap *pixmap, ggobid *gg) {
  GtkWidget *w = sp->da;
#if 0
  if(gg->plot_GC == NULL) {
    init_plot_GC(w->window, gg);
    fprintf(stderr, "Mmmm");
    return;
  }
#endif
  gdk_draw_pixmap (sp->da->window, gg->plot_GC, pixmap,
                   0, 0, 0, 0,
                   w->allocation.width,
                   w->allocation.height);
}

/*------------------------------------------------------------------------*/
/*                   draw tour axes                                       */
/*------------------------------------------------------------------------*/
static void
splot_draw_tour_axes(splotd *sp, GdkDrawable *drawable, ggobid *gg)
{
  gint j, ix, iy;
  displayd *dsp = (displayd *) sp->displayptr;
  cpaneld *cpanel = &dsp->cpanel;
  gint proj = cpanel->projection;
  gint lbearing, rbearing, width, ascent, descent;
  GtkStyle *style = gtk_widget_get_style (sp->da);
  datad *d = dsp->d;
  gfloat dst;
  gint textheight = 0;
  gchar *varlab;
  gint dawidth = sp->da->allocation.width;
  gint daheight = sp->da->allocation.height;
  vartabled *vt;

  if (!dsp->options.axes_show_p)
    return;
  
  if (sp != NULL && sp->da != NULL && sp->da->window != NULL) {
    gdk_gc_set_foreground (gg->plot_GC, &gg->accent_color);
    switch (proj) {
      case TOUR1D:
        /*-- use string height to place the labels --*/
        gdk_text_extents (style->font, "yA", strlen("yA"),
          &lbearing, &rbearing, &width, &ascent, &descent);
        textheight = ascent + descent;

        /*-- draw vertical lines to mark the min and max positions --*/
        gdk_draw_line(drawable, gg->plot_GC,
          dawidth/4, daheight - textheight*d->ncols - 10,
          dawidth/4, daheight);
        gdk_draw_line(drawable, gg->plot_GC,
          3*dawidth/4, daheight - textheight*d->ncols - 10,
          3*dawidth/4, daheight);

        gdk_gc_set_line_attributes(gg->plot_GC, 2, GDK_LINE_SOLID, 
          GDK_CAP_ROUND, GDK_JOIN_ROUND);

        for (j=0; j<d->ncols; j++) {
          ix = dawidth/2 + (gint) (dsp->t1d.u.vals[0][j]*(gfloat) dawidth/4);
          iy = daheight - 10 - (d->ncols-1-j)*textheight;
          gdk_draw_line(drawable, gg->plot_GC,
            dawidth/2, daheight - 10 - (d->ncols-1-j)*textheight,
            ix, iy);
/*
 * An experiment:  add the labels only for those variables with
 * non-zero multipliers.  Add them on the right if positive, on
 * the left if negative.
*/
          if (ix != dawidth/2) {
            vt = vartable_element_get (j, d);
            gdk_text_extents (style->font, 
              vt->collab_tform, strlen (vt->collab_tform),
              &lbearing, &rbearing, &width, &ascent, &descent);
            gdk_draw_string (drawable, style->font, gg->plot_GC,
              (ix > dawidth/2) ? 3*dawidth/4 + 10 : dawidth/4 - width -10,
              iy, vt->collab_tform);
          }
        }     
        gdk_gc_set_line_attributes(gg->plot_GC, 1, GDK_LINE_SOLID, 
          GDK_CAP_ROUND, GDK_JOIN_ROUND);
      break;
      case TOUR2D:
        gdk_draw_arc(drawable,gg->plot_GC,FALSE,
          10, 3*daheight/4 - 10,
          dawidth/4, daheight/4, 0,360*64);

        gdk_gc_set_line_attributes(gg->plot_GC, 2, GDK_LINE_SOLID, 
          GDK_CAP_ROUND, GDK_JOIN_ROUND);
        for (j=0; j<d->ncols; j++) {
          ix = dawidth/8 + 10 +
            (gint) (dsp->t2d.u.vals[0][j]* (gfloat) dawidth/8);
          iy = daheight - 10 - (daheight/8 + 
            (gint) (dsp->t2d.u.vals[1][j]* (gfloat) daheight/8));
          gdk_draw_line(drawable, gg->plot_GC,
            dawidth/8+10, daheight-daheight/8-10,
            ix, iy);
          if (abs(ix - 10 - dawidth/8) > 5 ||
              abs(iy + 10 - (daheight- daheight/8)) > 5)
          {
            varlab = g_strdup_printf("%d",j+1);
            gdk_text_extents (style->font, 
              varlab, strlen (varlab),
              &lbearing, &rbearing, &width, &ascent, &descent);
            ix = ix - 10 - dawidth/8;
            iy = iy - (daheight - daheight/8 - 10);
            dst = sqrt(ix*ix + iy*iy);
            ix = 10 + dawidth/8 + 
               (gint) ((gfloat) ix / dst * (gfloat) dawidth/8);
            iy = daheight - 10 - 
               daheight/8 + (gint) ((gfloat) iy / dst * (gfloat) daheight/8);
            if (ix < dawidth/8+10)
              ix -= width;
            else
              ix += width;
            if (iy < daheight-daheight/8-10)
              iy -= 3;
            else
              iy += (8);
            gdk_draw_string (drawable, style->font, gg->plot_GC,
              ix, iy,
              varlab);
            g_free (varlab);
          }
        }
        gdk_gc_set_line_attributes(gg->plot_GC, 0, GDK_LINE_SOLID, 
          GDK_CAP_ROUND, GDK_JOIN_ROUND);
      break;
      case COTOUR:
        /*-- use string height to place the labels --*/
        gdk_text_extents (style->font, "yA", strlen("yA"),
          &lbearing, &rbearing, &width, &ascent, &descent);
        textheight = ascent + descent;

        /*-- draw vertical lines to mark the min and max positions --*/
        gdk_draw_line(drawable, gg->plot_GC,
          dawidth/4, daheight - textheight*d->ncols - 10,
          dawidth/4, daheight);
        gdk_draw_line(drawable, gg->plot_GC,
          3*dawidth/4, daheight - textheight*d->ncols - 10,
          3*dawidth/4, daheight);

        /*-- draw horizontal lines to mark the min and max positions --*/
        gdk_draw_line(drawable, gg->plot_GC,
          0,                   daheight/4,
          textheight*d->ncols, daheight/4);
        gdk_draw_line(drawable, gg->plot_GC,
          0,                   3*daheight/4,
          textheight*d->ncols, 3*daheight/4);

        gdk_gc_set_line_attributes(gg->plot_GC, 2, GDK_LINE_SOLID, 
          GDK_CAP_ROUND, GDK_JOIN_ROUND);

        for (j=0; j<d->ncols; j++) {
          vt = vartable_element_get (j, d);

          /* horizontal */
          ix = dawidth/2 + 
            (gint) (dsp->tcorr1.u.vals[0][j]*
            (gfloat) dawidth/4);
          iy = daheight - 10 - (d->ncols-1-j)*textheight;
          gdk_draw_line(drawable, gg->plot_GC,
            dawidth/2,daheight - 10 - 
            (d->ncols-1-j)*textheight, ix, iy);
          gdk_gc_set_line_attributes(gg->plot_GC, 1, GDK_LINE_SOLID, 
            GDK_CAP_ROUND, GDK_JOIN_ROUND);

          gdk_text_extents (style->font, 
            vt->collab_tform, strlen (vt->collab_tform),
            &lbearing, &rbearing, &width, &ascent, &descent);
          gdk_draw_string (drawable, style->font, gg->plot_GC,
            dawidth/2+dawidth/4+10,
            iy, vt->collab_tform);
  
          /* vertical */
          ix = 10 + j*textheight;
          iy = daheight - (daheight/2 + 
            (gint) (dsp->tcorr2.u.vals[0][j]*
            (gfloat) daheight/4));
          gdk_gc_set_line_attributes(gg->plot_GC, 2, GDK_LINE_SOLID, 
            GDK_CAP_ROUND, GDK_JOIN_ROUND);
          gdk_draw_line(drawable, gg->plot_GC,
            10+j*textheight,daheight/2,
            ix, iy);

          /*-- can't add vertical variable labels --*/
        }     
        gdk_gc_set_line_attributes(gg->plot_GC, 0, GDK_LINE_SOLID, 
          GDK_CAP_ROUND, GDK_JOIN_ROUND);
        break;
    }
  }
}

/*------------------------------------------------------------------------*/
/*                   convenience routine                                  */
/*------------------------------------------------------------------------*/

void
splot_redraw (splotd *sp, RedrawStyle redraw_style, ggobid *gg) {

  /*-- sometimes the first draw happens before configure is called --*/
  if (sp == NULL || sp->da == NULL || sp->pixmap0 == NULL) {
    return;
  }

  switch (redraw_style) {
    case FULL:  /*-- FULL_2PIXMAP --*/
      splot_draw_to_pixmap0_unbinned (sp, gg);
      splot_pixmap0_to_pixmap1 (sp, false, gg);  /* false = not binned */
      splot_add_markup_to_pixmap (sp, sp->pixmap1, gg);
      splot_pixmap_to_window (sp, sp->pixmap1, gg);
    break;
    case QUICK:
      splot_pixmap0_to_pixmap1 (sp, false, gg);  /* false = not binned */
      splot_add_markup_to_pixmap (sp, sp->pixmap1, gg);
      splot_pixmap_to_window (sp, sp->pixmap1, gg);
    break;

    case BINNED:
      splot_draw_to_pixmap0_binned (sp, gg);
      splot_pixmap0_to_pixmap1 (sp, true, gg);  /* true = binned */
      splot_add_markup_to_pixmap (sp, sp->pixmap1, gg);
      splot_pixmap_to_window (sp, sp->pixmap1, gg);
    break;

    case FULL_1PIXMAP:  /*-- to optimize motion --*/
      splot_draw_to_pixmap0_unbinned (sp, gg);
      splot_add_markup_to_pixmap (sp, sp->pixmap0, gg);
      splot_pixmap_to_window (sp, sp->pixmap0, gg);
    break;

    case EXPOSE:
      splot_pixmap_to_window (sp, sp->pixmap1, gg);
    break;

    case NONE:
    break;
  }

  /*
   * Somehow the very first window is initially drawn without a border. 
   * I ought to be able to fix that more nicely some day, but in the
   * meantime, what's an extra rectangle?
  */
  if (sp == gg->current_splot && redraw_style != NONE) 
    splot_draw_border (sp, sp->da->window, gg);

  sp->redraw_style = EXPOSE;
}
