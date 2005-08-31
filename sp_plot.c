/* sp_plot.c */
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
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"
#include "colorscheme.h"

static void splot_draw_border (splotd *, GdkDrawable *, ggobid *);

static void
splot_check_colors (gushort maxcolorid, gint *ncolors_used,
  gushort *colors_used, datad *d, ggobid *gg)
{
  colorschemed *scheme = gg->activeColorScheme;
  gchar *message;

  if (maxcolorid >= scheme->n) {
    /* force a remapping even if the number of colors is too large */
    if (*ncolors_used > scheme->n) {
      message = g_strdup_printf ("The number of colors in use (%d) is greater than than\nthe number of colors in the current scheme (%d).\nColors are being reassigned.", *ncolors_used, scheme->n);
      quick_message (message, false);
      g_free (message);
    }
    else {
      message = g_strdup_printf ("The largest color id in use (%d) is too large for\nthe number of colors in the current scheme (%d).\nColors are being reassigned.", maxcolorid, scheme->n);
      quick_message (message, false);
      g_free (message);
   }

    colors_remap (scheme, true, gg);
    /* repeat to get the remapped values */
    datad_colors_used_get (ncolors_used, colors_used, d, gg);
  }
}


gboolean
splot_plot_case (gint m, datad *d,
  splotd *sp, displayd *display, ggobid *gg)
{
  gboolean draw_case = true;

  /*-- determine whether case m should be plotted --*/
  /*-- usually checking sampled is redundant because we're looping
       over rows_in_plot, but maybe we're not always --*/
  if (d->excluded.els[m] || !d->sampled.els[m])
    draw_case = false;

  /*-- can prevent drawing of missings for parcoords or scatmat plots --*/
  else if (d->nmissing > 0 && !d->missings_show_p) {
    if(GTK_IS_GGOBI_EXTENDED_SPLOT(sp)) {
      GtkGGobiExtendedSPlotClass *klass;
      klass = GTK_GGOBI_EXTENDED_SPLOT_CLASS(GTK_OBJECT_GET_CLASS(sp));
      if(klass->draw_case_p) {
         draw_case = klass->draw_case_p(sp, m, d, gg);
      }
    }
  }
  return draw_case;
}

/*------------------------------------------------------------------------*/
/*               drawing to pixmap0 when binning can't be used            */
/*------------------------------------------------------------------------*/

void
splot_clear_pixmap0 (splotd *sp, ggobid *gg)
{
  colorschemed *scheme = gg->activeColorScheme;

  if (gg->plot_GC == NULL) {
    init_plot_GC (sp->pixmap0, gg);
  }

  /* clear the pixmap */
  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_bg);
  gdk_draw_rectangle (sp->pixmap0, gg->plot_GC,
                      true, 0, 0,
                      sp->da->allocation.width,
                      sp->da->allocation.height);
}

/* I'm replacing this routine -- dfs */
#if 0
void
splot_draw_to_pixmap0_unbinned (splotd *sp, gboolean draw_hidden, ggobid *gg)
{
  gint k;
  gushort current_color;
  gint ncolors_used;
  gushort colors_used[MAXNCOLORS+2];
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;
  colorschemed *scheme = gg->activeColorScheme;
  gushort maxcolorid;

  gint i, m;
  gboolean (*f)(splotd *, datad*, ggobid*, gboolean) = NULL;

  GtkGGobiExtendedSPlotClass *klass = NULL;
  GtkGGobiExtendedDisplayClass *displayKlass = NULL;

  /*
   * since parcoords and tsplot each have their own weird way
   * of drawing line segments, it's necessary to get the point
   * colors before drawing those lines even if we're not drawing
   * points.
  */
  gboolean loop_over_points;

  g_assert (d->hidden.nels == d->nrows);

  if (GTK_IS_GGOBI_EXTENDED_DISPLAY(display)) {
   displayKlass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT_GET_CLASS(display));
   loop_over_points = display->options.points_show_p &&
                      displayKlass->loop_over_points;
  } else
   loop_over_points = display->options.points_show_p;

  if (display->options.whiskers_show_p) loop_over_points = true;
  /*
   * There's a problem with parallel coordinates -- once 'draw points'
   * is turned off, we don't get whiskers, either.  Somehow we need to
   * have the whiskers treated with the edges rather than with the
   * points, but I don't know how to do that at the moment.
   *
   * The time series display has the same problem -- I thought it
   * had switched to using edges, but it hasn't.  (I should take care
   * of that one of these days, since Nicholas probably won't want to.)
   *
   * loop_over_points has to be always true for both those displays,
   * and then the decision about whether to draw the points needs to
   * be deferred.  This won't slow down the scatterplot of the
   * barchart, apparently, because they have redraw methods.
   */

  if (display->options.edges_undirected_show_p ||
      display->options.edges_arrowheads_show_p ||
      display->options.edges_directed_show_p)
  {
    if (displayKlass && displayKlass->show_edges_p)
      splot_edges_draw (sp, draw_hidden, sp->pixmap0, gg);
  }

/*
 * Draw points
*/
  if (!gg->mono_p && loop_over_points) {

    if (draw_hidden) {  /*-- If drawing hidden points ... --*/

      gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_hidden);

      /*
      */
      if (GTK_IS_GGOBI_EXTENDED_SPLOT(sp)) {
        klass = GTK_GGOBI_EXTENDED_SPLOT_CLASS(GTK_OBJECT_GET_CLASS(sp));
        f = klass->redraw;
        if(f) {
          f(sp, d, gg, false);
        }
      }

      if (!f) {
#ifdef WIN32
        win32_draw_to_pixmap_unbinned (-1, sp, draw_hidden, gg);
#else
        for (i=0; i<d->nrows_in_plot; i++) {
          m = d->rows_in_plot.els[i];
          if (d->hidden_now.els[m] && splot_plot_case (m, d, sp, display, gg)) {
            /* This double-check accommodates the parallel coordinates
	     * and time series displays, because we have to ignore
             * points_show_p in order to draw the whiskers without the points.
	    */
            if (display->options.points_show_p)
              draw_glyph (sp->pixmap0, &d->glyph_now.els[m], sp->screen,
                m, gg);
            if (klass && klass->within_draw_to_unbinned)
              klass->within_draw_to_unbinned(sp, m, sp->pixmap0, gg->plot_GC);
          }
        }
#endif
      }
    } else {  /*-- un-hidden points --*/

      maxcolorid = datad_colors_used_get (&ncolors_used, colors_used, d, gg);
      splot_check_colors (maxcolorid, &ncolors_used, colors_used, d, gg);

      /*
       * Now loop through colors_used[], plotting the points of each
       * color.  This avoids the need to reset the foreground so often.
       * On the other hand, it requires more looping.
      */
      for (k=0; k<ncolors_used; k++) {
        current_color = colors_used[k];
        gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb[current_color]);

        if (GTK_IS_GGOBI_EXTENDED_SPLOT(sp)) {
          klass = GTK_GGOBI_EXTENDED_SPLOT_CLASS(GTK_OBJECT_GET_CLASS(sp));
          f = klass->redraw;
          if(f) {
            f(sp, d, gg, false);
          }
        }

        if(!f) {
#ifdef WIN32
        win32_draw_to_pixmap_unbinned (current_color, sp, draw_hidden, gg);
#else
          for (i=0; i<d->nrows_in_plot; i++) {
            m = d->rows_in_plot.els[i];
            if (d->color_now.els[m] == current_color &&
              !d->hidden_now.els[m] &&
              splot_plot_case (m, d, sp, display, gg))
            {
              /* This double-check accommodates the parallel coordinates
               * and time series displays, because we have to ignore
               * points_show_p in order to draw the whiskers without the points.
              */
              if (display->options.points_show_p)
                draw_glyph (sp->pixmap0, &d->glyph_now.els[m], sp->screen,
                  m, gg);
              if (klass && klass->within_draw_to_unbinned)
                klass->within_draw_to_unbinned(sp, m, sp->pixmap0, gg->plot_GC);
            }
          }
#endif
        }
      }
    }

  }

  return;
}
#endif

void
splot_draw_to_pixmap0_unbinned (splotd *sp, gboolean draw_hidden, ggobid *gg)
{
  gint k;
  gushort current_color;
  gint ncolors_used;
  gushort colors_used[MAXNCOLORS+2];
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;
  colorschemed *scheme = gg->activeColorScheme;
  gushort maxcolorid;
  gboolean loop_over_points;

  gint i, m;
  gboolean (*f)(splotd *, datad*, ggobid*, gboolean) = NULL;  /* redraw */

  GtkGGobiExtendedSPlotClass *klass = NULL;
  GtkGGobiExtendedDisplayClass *displayKlass = NULL;

  g_assert (d->hidden.nels == d->nrows);

  /*
   * There's a problem with parallel coordinates -- once 'draw points'
   * is turned off, we don't get whiskers, either.  Somehow we need to
   * have the whiskers treated with the edges rather than with the
   * points, but I don't know how to do that at the moment.
   *
   * The time series display has the same problem -- I thought it
   * had switched to using edges, but it hasn't.  (I should take care
   * of that one of these days, since Nicholas probably won't want to.)
   *
   * loop_over_points has to be always true for both those displays,
   * and then the decision about whether to draw the points needs to
   * be deferred.    -- dfs
   */

  if (GTK_IS_GGOBI_EXTENDED_DISPLAY(display)) {
    displayKlass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT_GET_CLASS(display)); 
    /* 
     * This has to be true for the whiskered displays if either points
     * or whiskers are to be drawn.
     */
    loop_over_points = (display->options.points_show_p ||
                        display->options.whiskers_show_p) &&
                       displayKlass->loop_over_points;
  } else {
    loop_over_points = display->options.points_show_p;
  }

  if (GTK_IS_GGOBI_EXTENDED_SPLOT(sp)) {
    klass = GTK_GGOBI_EXTENDED_SPLOT_CLASS(GTK_OBJECT_GET_CLASS(sp));
    f = klass->redraw;
  }

  /* Draw edges -- this doesn't include whiskers, alas */
  if (displayKlass && displayKlass->show_edges_p) {
    if (display->options.edges_undirected_show_p ||
        display->options.edges_arrowheads_show_p ||
        display->options.edges_directed_show_p)
    {
      splot_edges_draw (sp, draw_hidden, sp->pixmap0, gg);
    }
  }

  /* Now deal with points -- and whiskers */

  if (displayKlass && displayKlass->loop_over_points && f && 
      display->options.points_show_p)
  {
    /* 
     * Only the barchart has a redraw routine, as far as I can tell,
     * and it does its own handling of hiddens and colors.  I could
     * certainly add redraw routines for the other displays ... dfs
    */
    f(sp, d, gg, false);

  } else {

    if (draw_hidden) {
      gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_hidden);

#ifdef WIN32
      win32_draw_to_pixmap_unbinned (-1, sp, draw_hidden, gg);
#else
      for (i=0; i<d->nrows_in_plot; i++) {
        m = d->rows_in_plot.els[i];
        if (d->hidden_now.els[m] && splot_plot_case (m, d, sp, display, gg)) {
          /*
           * This double-check accommodates the parallel coordinates and
           * time series displays, because we have to ignore points_show_p
           * in order to draw the whiskers but not the points.
          */
          if (display->options.points_show_p)
            draw_glyph (sp->pixmap0, &d->glyph_now.els[m], sp->screen,
              m, gg);
          /* draw the whiskers ... or, potentially, other decorations */
          if (klass && klass->within_draw_to_unbinned)
            klass->within_draw_to_unbinned(sp, m, sp->pixmap0, gg->plot_GC);
        }
      }
#endif
    } else {  /*-- un-hidden points --*/

      maxcolorid = datad_colors_used_get (&ncolors_used, colors_used, d, gg);
      splot_check_colors (maxcolorid, &ncolors_used, colors_used, d, gg);
      /*
       * Now loop through colors_used[], plotting the points of each
       * color.  This avoids the need to reset the foreground so often.
      */
      for (k=0; k<ncolors_used; k++) {
        current_color = colors_used[k];
        gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb[current_color]);
#ifdef WIN32
        win32_draw_to_pixmap_unbinned (current_color, sp, draw_hidden, gg);
#else
        for (i=0; i<d->nrows_in_plot; i++) {
          m = d->rows_in_plot.els[i];
          if (d->color_now.els[m] == current_color &&
            !d->hidden_now.els[m] &&
            splot_plot_case (m, d, sp, display, gg))
          {
            /*
             * As above, this test accommodates the parallel coordinates
             * and time series displays, because we have to ignore points_show_p
             * in order to draw the whiskers but not the points.
            */
            if (display->options.points_show_p)
              draw_glyph (sp->pixmap0, &d->glyph_now.els[m], sp->screen,
                m, gg);
            if (klass && klass->within_draw_to_unbinned)
              klass->within_draw_to_unbinned(sp, m, sp->pixmap0, gg->plot_GC);
          }
        }
#endif
      }
    }
  }

  return;
}

void
splot_clear_pixmap0_binned (splotd *sp, ggobid *gg)
{
  icoords loc_clear0, loc_clear1;
  icoords *bin0 = &gg->plot.bin0;
  icoords *bin1 = &gg->plot.bin1;
  icoords *loc0 = &gg->plot.loc0;
  icoords *loc1 = &gg->plot.loc1;
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;
  colorschemed *scheme = gg->activeColorScheme;

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

  if (gg->plot_GC == NULL) {
    init_plot_GC (sp->pixmap0, gg);
  }

  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_bg);
  gdk_draw_rectangle (sp->pixmap0, gg->plot_GC,
                      true,  /* fill */
                      loc_clear0.x, loc_clear0.y,
                      1 + loc_clear1.x - loc_clear0.x ,
                      1 + loc_clear1.y - loc_clear0.y);
}

void
splot_draw_to_pixmap0_binned (splotd *sp, gboolean draw_hidden, ggobid *gg)
{
#ifndef WIN32
  gint ih, iv;
  gint i, m;
#endif
  gint k;
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  ProjectionMode proj = cpanel->pmode;
  datad *d = display->d;
  colorschemed *scheme = gg->activeColorScheme;
  icoords *bin0 = &gg->plot.bin0;
  icoords *bin1 = &gg->plot.bin1;
  gushort maxcolorid;

  gushort current_color;
  gint ncolors_used;
  gushort colors_used[MAXNCOLORS+2];

  GtkGGobiExtendedSPlotClass *klass = NULL;

  if (gg->plot_GC == NULL)
    init_plot_GC (sp->pixmap0, gg);

    /* Allow the extended plot to take over the entire thing.
       If it wants to take over just a small part, see below.*/
  if(GTK_IS_GGOBI_EXTENDED_SPLOT(sp)) {
    klass = GTK_GGOBI_EXTENDED_SPLOT_CLASS(GTK_OBJECT_GET_CLASS(sp));
    if(klass->redraw) {
      displayd *display = (displayd *) sp->displayptr;
      datad *d = display->d;
/* XXX barcharts, for instance, don't know about this new approach yet */
      if(klass->redraw(sp, d, gg, true)) {
        return;
      }
    }
  }

  if (!gg->mono_p && display->options.points_show_p) {

    if (draw_hidden) {  /* draw only the hidden cases */

      gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_hidden);

#ifdef WIN32
      win32_draw_to_pixmap_binned (bin0, bin1, -1, sp, draw_hidden, gg);
#else
      for (ih=bin0->x; ih<=bin1->x; ih++) {
        for (iv=bin0->y; iv<=bin1->y; iv++) {
          for (m=0; m<d->brush.binarray[ih][iv].nels ; m++) {
            i = d->rows_in_plot.els[d->brush.binarray[ih][iv].els[m]];

            /* if hidden && plottable */
            if (d->hidden_now.els[i] &&
                splot_plot_case (i, d, sp, display, gg))
            {
              draw_glyph (sp->pixmap0, &d->glyph_now.els[i],
                sp->screen, i, gg);

              /* parallel coordinate plot and time series plot whiskers */
              if(klass && klass->within_draw_to_binned) {
                klass->within_draw_to_binned(sp, m,
                  sp->pixmap0, gg->plot_GC);
              }
            }
          }
        }
      }
#endif

    } else {  /* if !draw_hidden */

      maxcolorid = datad_colors_used_get (&ncolors_used, colors_used, d, gg);
      splot_check_colors (maxcolorid, &ncolors_used, colors_used, d, gg);

      /*
       * Now loop through colors_used[], plotting the points of each
       * color in a group.
      */
      for (k=0; k<ncolors_used; k++) {
        current_color = colors_used[k];
        gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb[current_color]);

#ifdef WIN32
        win32_draw_to_pixmap_binned (bin0, bin1, current_color,
          sp, draw_hidden, gg);
#else

        for (ih=bin0->x; ih<=bin1->x; ih++) {
          for (iv=bin0->y; iv<=bin1->y; iv++) {
            for (m=0; m<d->brush.binarray[ih][iv].nels ; m++) {
              i = d->rows_in_plot.els[d->brush.binarray[ih][iv].els[m]];

              if (!d->hidden_now.els[i] &&
                  d->color_now.els[i] == current_color &&
                  splot_plot_case (i, d, sp, display, gg))
              {
                draw_glyph (sp->pixmap0, &d->glyph_now.els[i],
                  sp->screen, i, gg);

                /* parallel coordinate plot whiskers */
                if(klass && klass->within_draw_to_binned) {
                  klass->within_draw_to_binned(sp, m,
                    sp->pixmap0, gg->plot_GC);
                }

              }
            }
          }
        }
#endif
      }
    }
  }

  if (proj == TOUR1D || proj == TOUR2D3 || proj == TOUR2D || proj == COTOUR) {
    splot_draw_tour_axes(sp, sp->pixmap0, gg);
  }

  return;
}


/*------------------------------------------------------------------------*/
/*                   plot labels: variables                               */
/*------------------------------------------------------------------------*/

static void
splot_add_plot_labels (splotd *sp, GdkDrawable *drawable, ggobid *gg) 
{
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  datad *d = display->d;
  colorschemed *scheme = gg->activeColorScheme;

  gboolean proceed = (cpanel->pmode == XYPLOT ||
                      cpanel->pmode == P1PLOT ||
                      /*cpanel->pmode == PCPLOT ||
			cpanel->pmode == SCATMAT ||*/
                      cpanel->pmode == EXTENDED_DISPLAY_PMODE);
  if (!proceed)
    return;

  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);

  if(GTK_IS_GGOBI_EXTENDED_SPLOT(sp)) {
    void (*f)(splotd *, GdkDrawable*, ggobid*);
    f =  GTK_GGOBI_EXTENDED_SPLOT_CLASS(GTK_OBJECT_GET_CLASS(sp))->add_plot_labels;
    if(f) {
      f(sp, drawable, gg);
      return;
    }

  } 
     /* And allow the display to take up the slack if we don't want to have a special splot class
        for the display type but still need to do something special. */
  if(GTK_IS_GGOBI_EXTENDED_DISPLAY(display)) {
    void (*f)(displayd *, splotd *, GdkDrawable*, datad *, ggobid*);
    f =  GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT_GET_CLASS(display))->add_plot_labels;
    if(f)
      f(display, sp, drawable, d, gg);
  }

}



#define DIAMOND_DIM 5

/*-- draw a diamond around the current case --*/
void
splot_add_diamond_cue (gint k, splotd *sp, GdkDrawable *drawable, ggobid *gg)
{
  datad *d = sp->displayptr->d;
  gint diamond_dim = DIAMOND_DIM;
  GdkPoint diamond[5];
  colorschemed *scheme = gg->activeColorScheme;

  if (k < 0 || k >= d->nrows) return;

  diamond[0].x = diamond[4].x = sp->screen[k].x - diamond_dim;
  diamond[0].y = diamond[4].y = sp->screen[k].y;
  diamond[1].x = sp->screen[k].x;
  diamond[1].y = sp->screen[k].y - diamond_dim;
  diamond[2].x = sp->screen[k].x + diamond_dim;
  diamond[2].y = sp->screen[k].y;
  diamond[3].x = sp->screen[k].x;
  diamond[3].y = sp->screen[k].y + diamond_dim;

  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
  gdk_draw_lines (drawable, gg->plot_GC, diamond, 5);
}

/*-- add the label to a case (with or without underlining) --*/
static void
splot_add_point_label (gboolean nearest, gint k, splotd *sp,
  GdkDrawable *drawable, ggobid *gg)
{
  displayd *dsp = sp->displayptr;
  datad *d = dsp->d;
  gint lbearing, rbearing, width, ascent, descent;
  GtkStyle *style = gtk_widget_get_style (sp->da);
  gint diamond_dim = DIAMOND_DIM;
  gchar *lbl = NULL;

  if (k < 0 || k >= d->nrows) return;

  lbl = identify_label_fetch (k, &dsp->cpanel, d, gg);

  /*
   * if displaying 'variable labels' and no variable is selected,
   * lbl can still be NULL here.
  */
  if (lbl) {
    splot_text_extents (lbl, style, 
      &lbearing, &rbearing, &width, &ascent, &descent);

    if (sp->screen[k].x <= sp->max.x/2) {
      splot_draw_string (lbl,
        sp->screen[k].x+diamond_dim,
        sp->screen[k].y-diamond_dim,
        style, drawable, gg);

      /*-- underline the nearest point label?  --*/
      if (nearest)
        gdk_draw_line (drawable, gg->plot_GC,
          sp->screen[k].x+diamond_dim,
          sp->screen[k].y-diamond_dim+1,
          sp->screen[k].x+diamond_dim+width,
          sp->screen[k].y-diamond_dim+1);
      
    } else {
      splot_draw_string (lbl,
        sp->screen[k].x - width - diamond_dim,
        sp->screen[k].y - diamond_dim,
        style, drawable, gg);

      if (nearest)
        gdk_draw_line (drawable, gg->plot_GC,
          sp->screen[k].x - width - diamond_dim,
          sp->screen[k].y - diamond_dim+1,
          sp->screen[k].x - diamond_dim,
          sp->screen[k].y - diamond_dim+1);
    }

    /*-- display the label in the top center of the window as well --*/
    if (nearest) {
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
splot_add_identify_cues (splotd *sp, GdkDrawable *drawable,
  gint k, gboolean nearest, ggobid *gg)
{
  colorschemed *scheme = gg->activeColorScheme;
  gboolean useDefault = false;

  if (nearest) {
    if (GTK_IS_GGOBI_EXTENDED_SPLOT(sp)) {
      GtkGGobiExtendedSPlotClass *klass;
      klass = GTK_GGOBI_EXTENDED_SPLOT_CLASS(GTK_OBJECT_GET_CLASS(sp));
      if (klass->add_identify_cues)
        klass->add_identify_cues(k, sp, drawable, gg);
      else 
        useDefault = true;
    }

    if (useDefault)
      splot_add_diamond_cue (k, sp, drawable, gg);
  }

  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
  splot_add_point_label (nearest, k, sp, drawable, gg);
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
    splot_add_point_label (nearest, k, sp, drawable, gg);
  }
}


static void
splot_add_record_cues (splotd *sp, GdkDrawable *drawable, ggobid *gg) {
  gint id;
  GSList *l;
  displayd *display = (displayd *) sp->displayptr;
  datad *d = display->d;
  datad *e = display->e;
  InteractionMode imode = imode_get (gg);
  cpaneld *cpanel = &display->cpanel;

  /*
     these are the cues added to
       nearest point and sticky guys    in identification
       nearest edge and sticky guys     in identification
       nearest point                    in moving points
       source, maybe dest and edge      in edge editing
  */

  if (imode == IDENT) {
    if (cpanel->id_target_type == identify_points &&
        d->nearest_point != -1)
    {
      splot_add_identify_cues (sp, drawable, d->nearest_point, true, gg);
    } else if (cpanel->id_target_type == identify_edges && e) {
      if (e->nearest_point != -1)
        splot_add_identify_edge_cues (sp, drawable, e->nearest_point, true, gg);
    }
  }
  else if (imode == MOVEPTS)
     splot_add_movepts_cues (sp, drawable, d->nearest_point, true, gg);
  else if (imode == EDGEED)
    /* If I want to draw in the color of nearest_point, I should pass
       it in here -- dfs*/
    splot_add_edgeedit_cues (sp, drawable, d->nearest_point, true, gg);
 

  /*-- and these are the sticky ids, added in all modes --*/
  if (d->sticky_ids != NULL && g_slist_length (d->sticky_ids) > 0) {
    for (l = d->sticky_ids; l; l = l->next) {
      id = GPOINTER_TO_INT (l->data);
      if (!d->hidden_now.els[id])
        /*-- false = !nearest --*/
        splot_add_identify_cues (sp, drawable, id, false, gg);
    }
  }

  if (e && e->sticky_ids != NULL && g_slist_length (e->sticky_ids) > 0) {
    for (l = e->sticky_ids; l; l = l->next) {
      id = GPOINTER_TO_INT (l->data);
      if (!e->hidden_now.els[id])
        /*-- false = !nearest --*/
        splot_add_identify_edge_cues (sp, drawable, id, false, gg);
    }
  }
}

/*------------------------------------------------------------------------*/
/*                 draw the border indicating current splot               */
/*------------------------------------------------------------------------*/

static void
splot_draw_border (splotd *sp, GdkDrawable *drawable, ggobid *gg)
{
  colorschemed *scheme = gg->activeColorScheme;

  if (sp != NULL && sp->da != NULL && sp->da->window != NULL) {
    gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
    gdk_gc_set_line_attributes (gg->plot_GC,
      3, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);

    gdk_draw_rectangle (drawable, gg->plot_GC,
      false, 1, 1, sp->da->allocation.width-3, sp->da->allocation.height-3);

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
  displayd *dsp = (displayd *) sp->displayptr;
  datad *e = dsp->e;
  datad *d = dsp->d;
  cpaneld *cpanel = &dsp->cpanel;
  gint proj = cpanel->pmode;
  GtkGGobiExtendedSPlotClass *splotKlass;

  /*
   * if identification is going on in a plot of points that 
   * correspond to edges in this plot, add markup for the edge
   * corresponding to the nearest point in the other plot.
   * ( What about stickies? )
  */
  /*-- moving this section breaks splot_redraw (QUICK) for adding edges --*/
  if (sp != gg->current_splot && e && e->edge.n) {
    gboolean draw_edge;
    GtkGGobiExtendedDisplayClass *displayKlass = NULL;

    if (GTK_IS_GGOBI_EXTENDED_DISPLAY(dsp)) {
      displayKlass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT_GET_CLASS(dsp));
      draw_edge = displayKlass->supports_edges_p && displayKlass->show_edges_p;
    } else {
      draw_edge = dsp->options.edges_undirected_show_p ||
        dsp->options.edges_arrowheads_show_p ||
        dsp->options.edges_directed_show_p;
    }

    if (draw_edge && e->nearest_point != -1) {
      splot_add_edge_highlight_cue (sp, drawable, e->nearest_point,
        true, gg);
      splot_add_edge_label (sp, drawable, e->nearest_point,
        true, gg);
    }
  }
  
  if (GTK_IS_GGOBI_EXTENDED_SPLOT(sp)) {
    void (*f)(splotd *, GdkDrawable*, ggobid*);
    splotKlass = GTK_GGOBI_EXTENDED_SPLOT_CLASS(GTK_OBJECT_GET_CLASS(sp));
    f = splotKlass->add_markup_cues;
    if(f) {
      f(sp, drawable, gg);
    }
  }
   
  splot_add_plot_labels (sp, drawable, gg);  /*-- axis labels --*/

  /*-- identify points or edges, move points, edge editing --*/
  splot_add_record_cues (sp, drawable, gg);  

  if (sp == gg->current_splot)
    splot_draw_border (sp, drawable, gg);

  /*-- draw these cues whether this is the current display or not --*/
  if (g_list_length (dsp->splots) == 1  /*-- scatterplot --*/
      || sp == dsp->current_splot)  /*-- ... in a multi-plot display --*/
  {
    if (cpanel->imode == BRUSH) {
      brush_draw_brush (sp, drawable, d, gg);
      brush_draw_label (sp, drawable, d, gg);
    } else if (cpanel->imode == SCALE) {
      scaling_visual_cues_draw (sp, drawable, gg);

      if (GTK_IS_GGOBI_EXTENDED_SPLOT(sp)) {
        void (*f)(splotd *, GdkDrawable*, ggobid*);
        splotKlass = GTK_GGOBI_EXTENDED_SPLOT_CLASS(GTK_OBJECT_GET_CLASS(sp));
        f = splotKlass->add_scaling_cues;
        if(f)
          f(sp, drawable, gg);
      }
    }
  }

  if (proj == TOUR1D || proj == TOUR2D3 || proj == TOUR2D || proj == COTOUR) {
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
      splot_clear_pixmap0 (sp, gg);
      splot_draw_to_pixmap0_unbinned (sp, true, gg);  /* true = hiddens */
      splot_draw_to_pixmap0_unbinned (sp, false, gg);
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
      splot_clear_pixmap0_binned (sp, gg);
      splot_draw_to_pixmap0_binned (sp, true, gg); /* true = hiddens */
      splot_draw_to_pixmap0_binned (sp, false, gg);
      splot_pixmap0_to_pixmap1 (sp, true, gg);  /* true = binned */
      splot_add_markup_to_pixmap (sp, sp->pixmap1, gg);
      splot_pixmap_to_window (sp, sp->pixmap1, gg);
    break;

    case FULL_1PIXMAP:  /*-- to optimize motion --*/
      splot_clear_pixmap0 (sp, gg);
      splot_draw_to_pixmap0_unbinned (sp, true, gg);  /* true = hiddens */
      splot_draw_to_pixmap0_unbinned (sp, false, gg);
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
