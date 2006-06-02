/* sp_plot_axes.c */
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

/*------------------------------------------------------------------------*/
/*                   draw tour axes                                       */
/*------------------------------------------------------------------------*/
void
splot_draw_tour_axes (splotd * sp, GdkDrawable * drawable, ggobid * gg)
{
  gint j, k, ix, iy, nc;
  displayd *dsp = (displayd *) sp->displayptr;
  cpaneld *cpanel = &dsp->cpanel;
  gint proj = cpanel->pmode;
  GGobiStage *d = dsp->d;
  gfloat dst, val;
  gint textheight = 0, textheight2;
  gchar *varlab, *varval;
  gint dawidth = sp->da->allocation.width;
  gint daheight = sp->da->allocation.height;
  gint axindent = 20;
  GGobiVariable *var;
  colorschemed *scheme = gg->activeColorScheme;
  PangoContext *ctx;
  PangoFontMetrics *metrics;
  PangoLayout *layout = gtk_widget_create_pango_layout (sp->da, NULL);
  PangoRectangle rect;
#ifdef ENABLE_CAIRO
  cairo_t *cr;
#endif

  if (!dsp->options.axes_show_p)
    return;

#ifdef ENABLE_CAIRO
  //cr = create_cairo_glitz(drawable);
  cr = gdk_cairo_create (drawable);
#endif

  ctx = gtk_widget_get_pango_context (sp->da);
  metrics = pango_context_get_metrics (ctx,
                                       pango_context_get_font_description
                                       (ctx), NULL);
  if (sp != NULL && sp->da != NULL && sp->da->window != NULL) {
    gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
    switch (proj) {
    case TOUR1D:
        /*-- use string height to place the labels --*/

      textheight = PANGO_PIXELS (pango_font_metrics_get_ascent (metrics) +
                                 pango_font_metrics_get_descent (metrics));

        /*-- draw vertical lines to mark the min and max positions --*/
      gdk_draw_line (drawable, gg->plot_GC,
                     dawidth / 4, daheight - textheight * d->n_cols - 10,
                     dawidth / 4, daheight);
      gdk_draw_line (drawable, gg->plot_GC,
                     3 * dawidth / 4, daheight - textheight * d->n_cols - 10,
                     3 * dawidth / 4, daheight);

      gdk_gc_set_line_attributes (gg->plot_GC, 2, GDK_LINE_SOLID,
                                  GDK_CAP_ROUND, GDK_JOIN_ROUND);

      for (k = 0; k < dsp->t1d.nsubset; k++) {
        j = dsp->t1d.subset_vars.els[k];
        ix =
          dawidth / 2 + (gint) (dsp->t1d.F.vals[0][j] * (gfloat) dawidth / 4);
        iy = daheight - 10 - (dsp->t1d.nsubset - 1 - k) * textheight;
        if (j == dsp->t1d_manip_var)
#ifdef ENABLE_CAIRO
          gdk_cairo_set_source_color (cr, &gg->vcirc_manip_color);
#else
          gdk_gc_set_foreground (gg->plot_GC, &gg->vcirc_manip_color);
#endif
        else
#ifdef ENABLE_CAIRO
          gdk_cairo_set_source_color (cr, &scheme->rgb_accent);
#else
          gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
#endif
#ifdef ENABLE_CAIRO
        cairo_move_to (cr, dawidth / 2,
                       daheight - 10 - textheight / 2 - (dsp->t1d.nsubset -
                                                         1 - k) * textheight);
        cairo_line_to (cr, ix, iy - textheight / 2);
        cairo_stroke (cr);
#else
        gdk_draw_line (drawable, gg->plot_GC,
                       dawidth / 2,
                       daheight - 10 - textheight / 2 - (dsp->t1d.nsubset -
                                                         1 - k) * textheight,
                       ix, iy - textheight / 2);
#endif
/*
 * An experiment:  add the labels only for those variables with
 * non-zero multipliers.  Add them on the right if positive, on
 * the left if negative.
*/
        if (ix != dawidth / 2) {
          var = ggobi_stage_get_variable(d, j);
          varlab = g_strdup_printf ("%s:%4.3f(%.2f)", ggobi_stage_get_transformed_col_name(d, j),
                                    dsp->t1d.F.vals[0][j],
                                    ggobi_variable_get_range(var));
          layout_text (layout, varlab, &rect);
          gdk_draw_layout (drawable, gg->plot_GC,
                           (ix >
                            dawidth / 2) ? 3 * dawidth / 4 +
                           10 : dawidth / 4 - rect.width - 10,
                           iy - rect.height, layout);
          g_free (varlab);
        }
      }
      gdk_gc_set_line_attributes (gg->plot_GC, 1, GDK_LINE_SOLID,
                                  GDK_CAP_ROUND, GDK_JOIN_ROUND);
      break;

    case TOUR2D3:
      /* draws circle */
#ifdef ENABLE_CAIRO
      cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
      cairo_arc (cr, axindent + daheight / 8, 7 * daheight / 8 - axindent,
                 daheight / 8, 0, 2 * M_PI);
      cairo_stroke (cr);
#else
      gdk_draw_arc (drawable, gg->plot_GC, FALSE,
                    axindent, 3 * daheight / 4 - axindent,
                    dawidth / 4, daheight / 4, 0, 360 * 64);
#endif
      /* draw the axes and labels */
      for (k = 0; k < dsp->t2d3.nsubset; k++) {
        j = dsp->t2d3.subset_vars.els[k];
        ix = dawidth / 8 + axindent +
          (gint) (dsp->t2d3.F.vals[0][j] * (gfloat) dawidth / 8);
        iy = daheight - axindent - (daheight / 8 +
                                    (gint) (dsp->t2d3.F.vals[1][j] *
                                            (gfloat) daheight / 8));
        gdk_gc_set_line_attributes (gg->plot_GC, 2, GDK_LINE_SOLID,
                                    GDK_CAP_ROUND, GDK_JOIN_ROUND);
        if (j == dsp->t2d_manip_var)
#ifdef ENABLE_CAIRO
          gdk_cairo_set_source_color (cr, &gg->vcirc_manip_color);
#else
          gdk_gc_set_foreground (gg->plot_GC, &gg->vcirc_manip_color);
#endif
        else
#ifdef ENABLE_CAIRO
          gdk_cairo_set_source_color (cr, &scheme->rgb_accent);
#else
          gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
#endif
#ifdef ENABLE_CAIRO
        cairo_move_to (cr, daheight / 8 + axindent,
                       daheight - daheight / 8 - axindent);
        cairo_line_to (cr, ix, iy);
        cairo_stroke (cr);
#else
        gdk_draw_line (drawable, gg->plot_GC,
                       dawidth / 8 + axindent,
                       daheight - daheight / 8 - axindent, ix, iy);
#endif

        if (abs (ix - axindent - dawidth / 8) > 5 ||
            abs (iy + axindent - (daheight - daheight / 8)) > 5) {
          if (dsp->options.axes_label_p) {
            varlab = ggobi_stage_get_col_nickname(d, j);
          }
          else {
            varlab = g_strdup_printf ("%d", j + 1);
          }
          layout_text (layout, varlab, &rect);

          ix = ix - axindent - dawidth / 8;
          iy = iy - (daheight - daheight / 8 - axindent);
          dst = sqrt (ix * ix + iy * iy);
          ix = axindent + dawidth / 8 +
            (gint) ((gfloat) ix / dst * (gfloat) dawidth / 8);
          iy = daheight - axindent -
            daheight / 8 + (gint) ((gfloat) iy / dst * (gfloat) daheight / 8);
          if (ix < dawidth / 8 + axindent)
            ix -= rect.width;
          else
            ix += (rect.width / 2);
          if (iy < daheight - daheight / 8 - axindent)
            iy -= (rect.height);
          //else
          //  iy += (rect.height);

          gdk_draw_layout (drawable, gg->plot_GC, ix, iy, layout);
          g_free (varlab);
        }

        /* Drawing the axes values now */
        if (dsp->options.axes_values_p) {
          varval = g_strdup_printf ("%d:%4.3f,%4.3f", j + 1,
                                    dsp->t2d3.F.vals[0][j],
                                    dsp->t2d3.F.vals[1][j]);
          //if (k == 0) {
          layout_text (layout, varval, &rect);
          textheight2 = rect.height;
          //}

          ix = dawidth - rect.width - axindent;
          iy = daheight - (dsp->t2d3.nsubset - k) * textheight2 - axindent;
          gdk_draw_layout (drawable, gg->plot_GC, ix, iy, layout);
          g_free (varval);
        }
      }
      gdk_gc_set_line_attributes (gg->plot_GC, 0, GDK_LINE_SOLID,
                                  GDK_CAP_ROUND, GDK_JOIN_ROUND);

      break;

    case TOUR2D:
      /* draws circle */
#ifdef ENABLE_CAIRO
      cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
      cairo_arc (cr, axindent + daheight / 8, 7 * daheight / 8 - axindent,
                 daheight / 8, 0, 2 * M_PI);
      cairo_stroke (cr);
#else
      gdk_draw_arc (drawable, gg->plot_GC, FALSE,
                    axindent, 3 * daheight / 4 - axindent,
                    daheight / 4, daheight / 4, 0, 360 * 64);
#endif

      /* draw the axes and labels */
      for (k = 0; k < dsp->t2d.nsubset; k++) {
        j = dsp->t2d.subset_vars.els[k];
        ix = daheight / 8 + axindent +
          (gint) (dsp->t2d.F.vals[0][j] * (gfloat) daheight / 8);
        iy = daheight - axindent - (daheight / 8 +
                                    (gint) (dsp->t2d.F.vals[1][j] *
                                            (gfloat) daheight / 8));
        gdk_gc_set_line_attributes (gg->plot_GC, 2, GDK_LINE_SOLID,
                                    GDK_CAP_ROUND, GDK_JOIN_ROUND);
        if (j == dsp->t2d_manip_var)
#ifdef ENABLE_CAIRO
          gdk_cairo_set_source_color (cr, &gg->vcirc_manip_color);
#else
          gdk_gc_set_foreground (gg->plot_GC, &gg->vcirc_manip_color);
#endif
        else
#ifdef ENABLE_CAIRO
          gdk_cairo_set_source_color (cr, &scheme->rgb_accent);
#else
          gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
#endif
#ifdef ENABLE_CAIRO
        cairo_move_to (cr, daheight / 8 + axindent,
                       daheight - daheight / 8 - axindent);
        cairo_line_to (cr, ix, iy);
        cairo_stroke (cr);
#else
        gdk_draw_line (drawable, gg->plot_GC,
                       daheight / 8 + axindent,
                       daheight - daheight / 8 - axindent, ix, iy);
#endif

        if (abs (ix - axindent - daheight / 8) > 5 ||
            abs (iy + axindent - (daheight - daheight / 8)) > 5) {
          if (dsp->options.axes_label_p) {
            varlab = ggobi_stage_get_col_nickname(d, j);
          }
          else {
            varlab = g_strdup_printf ("%d", j + 1);
          }

          layout_text (layout, varlab, &rect);

          ix = ix - axindent - daheight / 8;
          iy = iy - (daheight - daheight / 8 - axindent);
          dst = sqrt (ix * ix + iy * iy);
          ix = axindent + daheight / 8 +
            (gint) ((gfloat) ix / dst * (gfloat) daheight / 8);
          iy = daheight - axindent -
            daheight / 8 + (gint) ((gfloat) iy / dst * (gfloat) daheight / 8);
          if (ix < daheight / 8 + axindent)
            ix -= rect.width;
          else
            ix += (rect.width / 2);
          if (iy < daheight - daheight / 8 - axindent)
            iy -= (rect.height);
          //else
          //  iy += (rect.height);

          gdk_draw_layout (drawable, gg->plot_GC, ix, iy, layout);
          g_free (varlab);
        }

        /* Drawing the axes values now */
        if (dsp->options.axes_values_p) {
          var = ggobi_stage_get_variable(d, j);
          varval = g_strdup_printf ("%d:%4.3f,%4.3f(%.2f)", j + 1,
                                    dsp->t2d.F.vals[0][j],
                                    dsp->t2d.F.vals[1][j],
                                    ggobi_variable_get_range(var));
          //if (k == 0) {
          layout_text (layout, varval, &rect);
          textheight2 = rect.height;
          //}

          ix = dawidth - rect.width - axindent;
          iy = daheight - (dsp->t2d.nsubset - k) * textheight2 - axindent;
          gdk_draw_layout (drawable, gg->plot_GC, ix, iy, layout);
          g_free (varval);
        }

      }
      gdk_gc_set_line_attributes (gg->plot_GC, 0, GDK_LINE_SOLID,
                                  GDK_CAP_ROUND, GDK_JOIN_ROUND);

      break;
    case COTOUR:
      if (d->n_cols < MIN_NVARS_FOR_COTOUR)
        break;

      nc = dsp->tcorr1.nsubset + dsp->tcorr2.nsubset;

        /*-- use string height to place the labels --*/
      textheight = PANGO_PIXELS (pango_font_metrics_get_ascent (metrics) +
                                 pango_font_metrics_get_descent (metrics));

        /*-- draw vertical lines to mark the min and max positions --*/
      gdk_draw_line (drawable, gg->plot_GC,
                     dawidth / 4, daheight - textheight * nc - 10,
                     dawidth / 4, daheight);
      gdk_draw_line (drawable, gg->plot_GC,
                     3 * dawidth / 4, daheight - textheight * nc - 10,
                     3 * dawidth / 4, daheight);

        /*-- draw horizontal lines to mark the min and max positions --*/
      gdk_draw_line (drawable, gg->plot_GC,
                     0, daheight / 4, textheight * nc, daheight / 4);
      gdk_draw_line (drawable, gg->plot_GC,
                     0, 3 * daheight / 4, textheight * nc, 3 * daheight / 4);

      gdk_gc_set_line_attributes (gg->plot_GC, 2, GDK_LINE_SOLID,
                                  GDK_CAP_ROUND, GDK_JOIN_ROUND);

      k = 0;
      for (j = 0; j < d->n_cols; j++) {
        if (!dsp->tcorr1.subset_vars_p.els[j] &&
            !dsp->tcorr2.subset_vars_p.els[j])
          continue;

        //var = ggobi_stage_get_variable(d, j);
        if (dsp->tcorr1.subset_vars_p.els[j]) {
          val = (ABS (dsp->tcorr1.F.vals[0][j]) > .004) ?
            dsp->tcorr1.F.vals[0][j] : 0.0;
          varlab = g_strdup_printf ("%s:%3.2f,0", ggobi_stage_get_transformed_col_name(d, j), val);
        }
        else {
          val = (ABS (dsp->tcorr2.F.vals[0][j]) > .004) ?
            dsp->tcorr2.F.vals[0][j] : 0.0;
          varlab = g_strdup_printf ("%s:0,%3.2f", ggobi_stage_get_transformed_col_name(d, j), val);
        }

        /* horizontal */
        ix = dawidth / 2 +
          (gint) (dsp->tcorr1.F.vals[0][j] * (gfloat) dawidth / 4);
        iy = daheight - 10 - (nc - k) * textheight;
        if (j == dsp->tc1_manip_var)
#ifdef ENABLE_CAIRO
          gdk_cairo_set_source_color (cr, &gg->vcirc_manip_color);
#else
          gdk_gc_set_foreground (gg->plot_GC, &gg->vcirc_manip_color);
#endif
        else
#ifdef ENABLE_CAIRO
          gdk_cairo_set_source_color (cr, &scheme->rgb_accent);
#else
          gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
#endif
#ifdef ENABLE_CAIRO
        cairo_move_to (cr, dawidth / 2, iy + rect.height / 2);
        cairo_line_to (cr, ix, iy + rect.height / 2);
        cairo_stroke (cr);
#else
        gdk_draw_line (drawable, gg->plot_GC,
                       dawidth / 2, iy + rect.height / 2,
                       ix, iy + rect.height / 2);
#endif
        gdk_gc_set_line_attributes (gg->plot_GC, 1, GDK_LINE_SOLID,
                                    GDK_CAP_ROUND, GDK_JOIN_ROUND);

        layout_text (layout, varlab, &rect);
        gdk_draw_layout (drawable, gg->plot_GC,
                         dawidth / 2 + dawidth / 4 + 10, iy, layout);

        /* vertical */
        ix = 10 + k * textheight;
        iy = daheight - (daheight / 2 +
                         (gint) (dsp->tcorr2.F.vals[0][j] *
                                 (gfloat) daheight / 4));
        gdk_gc_set_line_attributes (gg->plot_GC, 2, GDK_LINE_SOLID,
                                    GDK_CAP_ROUND, GDK_JOIN_ROUND);
        if (j == dsp->tc2_manip_var)
#ifdef ENABLE_CAIRO
          gdk_cairo_set_source_color (cr, &gg->vcirc_manip_color);
#else
          gdk_gc_set_foreground (gg->plot_GC, &gg->vcirc_manip_color);
#endif
        else
#ifdef ENABLE_CAIRO
          gdk_cairo_set_source_color (cr, &scheme->rgb_accent);
#else
          gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
#endif
#ifdef ENABLE_CAIRO
        cairo_move_to (cr, ix, daheight / 2);
        cairo_line_to (cr, ix, iy);
        cairo_stroke (cr);
#else
        gdk_draw_line (drawable, gg->plot_GC, ix, daheight / 2, ix, iy);
#endif

        g_free (varlab);
          /*-- can't add vertical variable labels --*/
        k++;
      }
      gdk_gc_set_line_attributes (gg->plot_GC, 0, GDK_LINE_SOLID,
                                  GDK_CAP_ROUND, GDK_JOIN_ROUND);
      break;
    }
  }
#ifdef ENABLE_CAIRO
  cairo_destroy (cr);
#endif
  pango_font_metrics_unref (metrics);
  g_object_unref (layout);
}
