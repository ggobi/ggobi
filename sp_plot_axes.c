/* sp_plot_axes.c */
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
/*                   draw tour axes                                       */
/*------------------------------------------------------------------------*/
void
splot_draw_tour_axes(splotd *sp, GdkDrawable *drawable, ggobid *gg)
{
  gint j, k, ix, iy;
  displayd *dsp = (displayd *) sp->displayptr;
  cpaneld *cpanel = &dsp->cpanel;
  gint proj = cpanel->projection;
  gint lbearing, rbearing, width, width2, ascent, descent;
  GtkStyle *style = gtk_widget_get_style (sp->da);
  datad *d = dsp->d;
  gfloat dst;
  gint textheight = 0, textheight2;
  gchar *varlab, *varval;
  gint dawidth = sp->da->allocation.width;
  gint daheight = sp->da->allocation.height;
  gint axindent = 20;
  vartabled *vt;
  colorschemed *scheme = gg->activeColorScheme;

  if (!dsp->options.axes_show_p)
    return;
  
  if (sp != NULL && sp->da != NULL && sp->da->window != NULL) {
    gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
    switch (proj) {
      case TOUR1D:
        /*-- use string height to place the labels --*/
        splot_text_extents ("yA", style, 
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

        for (k=0; k<dsp->t1d.nsubset; k++) {
          j = dsp->t1d.subset_vars.els[k];
          ix = dawidth/2 + (gint) (dsp->t1d.F.vals[0][j]*(gfloat) dawidth/4);
          iy = daheight - 10 - (dsp->t1d.nsubset-1-k)*textheight;
          if (j == dsp->t1d_manip_var)
            gdk_gc_set_foreground(gg->plot_GC, &gg->vcirc_manip_color);
          else
            gdk_gc_set_foreground(gg->plot_GC, &scheme->rgb_accent);
          gdk_draw_line(drawable, gg->plot_GC,
            dawidth/2, daheight - 10 - (dsp->t1d.nsubset-1-k)*textheight,
            ix, iy);
/*
 * An experiment:  add the labels only for those variables with
 * non-zero multipliers.  Add them on the right if positive, on
 * the left if negative.
*/
          if (ix != dawidth/2) {
            vt = vartable_element_get (j, d);
            varlab = g_strdup_printf("%s:%4.3f(%.2f)",vt->collab_tform,
              dsp->t1d.F.vals[0][j],vt->lim.max-vt->lim.min);
            splot_text_extents (varlab, style, 
              &lbearing, &rbearing, &width, &ascent, &descent);

            splot_draw_string (varlab,
              (ix > dawidth/2) ? 3*dawidth/4 + 10 : dawidth/4 - width -10,
              iy,
              style, drawable, gg);
            g_free (varlab);
          }
        }     
        gdk_gc_set_line_attributes(gg->plot_GC, 1, GDK_LINE_SOLID, 
          GDK_CAP_ROUND, GDK_JOIN_ROUND);
      break;

      case TOUR2D3:
        /* draws circle */
        gdk_draw_arc(drawable,gg->plot_GC,FALSE,
          axindent, 3*daheight/4 - axindent,
          dawidth/4, daheight/4, 0,360*64);

        /* draw the axes and labels */
        for (k=0; k<dsp->t2d3.nsubset; k++) {
          j = dsp->t2d3.subset_vars.els[k];
          ix = dawidth/8 + axindent +
            (gint) (dsp->t2d3.F.vals[0][j]* (gfloat) dawidth/8);
          iy = daheight - axindent - (daheight/8 + 
            (gint) (dsp->t2d3.F.vals[1][j]* (gfloat) daheight/8));
          gdk_gc_set_line_attributes(gg->plot_GC, 2, GDK_LINE_SOLID, 
            GDK_CAP_ROUND, GDK_JOIN_ROUND);
          if (j == dsp->t2d_manip_var)
            gdk_gc_set_foreground(gg->plot_GC, &gg->vcirc_manip_color);
          else
            gdk_gc_set_foreground(gg->plot_GC, &scheme->rgb_accent);
          gdk_draw_line(drawable, gg->plot_GC,
            dawidth/8+axindent, daheight-daheight/8-axindent,
            ix, iy);

          if (abs(ix - axindent - dawidth/8) > 5 ||
              abs(iy + axindent - (daheight- daheight/8)) > 5)
          {
            if (dsp->options.axes_label_p) {
              vt = vartable_element_get (j, d);
              varlab = g_strdup (vt->nickname);
            } else {
              varlab = g_strdup_printf ("%d",j+1);
            }

            splot_text_extents (varlab, style, 
              &lbearing, &rbearing, &width, &ascent, &descent);

            textheight = ascent+descent;
            ix = ix - axindent - dawidth/8;
            iy = iy - (daheight - daheight/8 - axindent);
            dst = sqrt(ix*ix + iy*iy);
            ix = axindent + dawidth/8 + 
               (gint) ((gfloat) ix / dst * (gfloat) dawidth/8);
            iy = daheight - axindent - 
               daheight/8 + (gint) ((gfloat) iy / dst * (gfloat) daheight/8);
            if (ix < dawidth/8+axindent)
              ix -= width;
            else
              ix += (width/2);
            if (iy < daheight-daheight/8-axindent)
              iy -= (textheight/2);
            else
              iy += (textheight);

            splot_draw_string (varlab, ix, iy, style, drawable, gg);
            g_free (varlab);
          }

          /* Drawing the axes values now */
          if (dsp->options.axes_values_p) {
            varval = g_strdup_printf ("%d:%4.3f,%4.3f",j+1,
              dsp->t2d3.F.vals[0][j],dsp->t2d3.F.vals[1][j]);
            if (k == 0) {
              splot_text_extents (varval, style, 
                &lbearing, &rbearing, &width2, &ascent, &descent);
              textheight2 = ascent+descent+5;
            }

            ix = dawidth - width2 - axindent;
            iy = daheight - (dsp->t2d3.nsubset-k-1)*textheight2 - axindent;
            splot_draw_string (varval, ix, iy, style, drawable, gg);
            g_free (varval);
          }
        }
        gdk_gc_set_line_attributes(gg->plot_GC, 0, GDK_LINE_SOLID, 
          GDK_CAP_ROUND, GDK_JOIN_ROUND);

      break;

      case TOUR2D:
        /* draws circle */
        gdk_draw_arc(drawable,gg->plot_GC,FALSE,
          axindent, 3*daheight/4 - axindent,
          daheight/4, daheight/4, 0,360*64);

        /* draw the axes and labels */
        for (k=0; k<dsp->t2d.nsubset; k++) {
          j = dsp->t2d.subset_vars.els[k];
          ix = daheight/8 + axindent +
            (gint) (dsp->t2d.F.vals[0][j]* (gfloat) daheight/8);
          iy = daheight - axindent - (daheight/8 + 
            (gint) (dsp->t2d.F.vals[1][j]* (gfloat) daheight/8));
          gdk_gc_set_line_attributes(gg->plot_GC, 2, GDK_LINE_SOLID, 
            GDK_CAP_ROUND, GDK_JOIN_ROUND);
          if (j == dsp->t2d_manip_var)
            gdk_gc_set_foreground(gg->plot_GC, &gg->vcirc_manip_color);
          else
            gdk_gc_set_foreground(gg->plot_GC, &scheme->rgb_accent);
          gdk_draw_line(drawable, gg->plot_GC,
            daheight/8+axindent, daheight-daheight/8-axindent,
            ix, iy);

          if (abs(ix - axindent - daheight/8) > 5 ||
              abs(iy + axindent - (daheight- daheight/8)) > 5)
          {
            if (dsp->options.axes_label_p) {
              vt = vartable_element_get (j, d);
              varlab = g_strdup (vt->nickname);
            } else {
              varlab = g_strdup_printf ("%d",j+1);
            }

            splot_text_extents (varlab, style, 
              &lbearing, &rbearing, &width, &ascent, &descent);

            textheight = ascent+descent;
            ix = ix - axindent - daheight/8;
            iy = iy - (daheight - daheight/8 - axindent);
            dst = sqrt(ix*ix + iy*iy);
            ix = axindent + daheight/8 + 
               (gint) ((gfloat) ix / dst * (gfloat) daheight/8);
            iy = daheight - axindent - 
               daheight/8 + (gint) ((gfloat) iy / dst * (gfloat) daheight/8);
            if (ix < daheight/8+axindent)
              ix -= width;
            else
              ix += (width/2);
            if (iy < daheight-daheight/8-axindent)
              iy -= (textheight/2);
            else
              iy += (textheight);

            splot_draw_string (varlab, ix, iy, style, drawable, gg);
            g_free (varlab);
          }

          /* Drawing the axes values now */
          if (dsp->options.axes_values_p) {
            vt = vartable_element_get (j, d);
            varval = g_strdup_printf ("%d:%4.3f,%4.3f(%.2f)",j+1,
              dsp->t2d.F.vals[0][j],dsp->t2d.F.vals[1][j],
              vt->lim.max-vt->lim.min);
            if (k == 0) {
              splot_text_extents (varval, style, 
                &lbearing, &rbearing, &width2, &ascent, &descent);
              textheight2 = ascent+descent+5;
            }

            ix = dawidth - width2 - axindent;
            iy = daheight - (dsp->t2d.nsubset-k-1)*textheight2 - axindent;
            splot_draw_string (varval, ix, iy, style, drawable, gg);
            g_free (varval);
          }

        }
        gdk_gc_set_line_attributes(gg->plot_GC, 0, GDK_LINE_SOLID, 
          GDK_CAP_ROUND, GDK_JOIN_ROUND);

      break;
      case COTOUR:
        if (d->ncols < MIN_NVARS_FOR_COTOUR)
          break;

        /*-- use string height to place the labels --*/
        splot_text_extents ("yA", style, 
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
          varlab = g_strdup_printf("%s:%3.2f,%3.2f",vt->collab_tform,
            dsp->tcorr1.F.vals[0][j],dsp->tcorr2.F.vals[0][j]);

          /* horizontal */
          ix = dawidth/2 + 
            (gint) (dsp->tcorr1.F.vals[0][j]*
            (gfloat) dawidth/4);
          iy = daheight - 10 - (d->ncols-1-j)*textheight;
          if (j == dsp->tc1_manip_var)
            gdk_gc_set_foreground(gg->plot_GC, &gg->vcirc_manip_color);
          else
            gdk_gc_set_foreground(gg->plot_GC, &scheme->rgb_accent);
          gdk_draw_line(drawable, gg->plot_GC,
            dawidth/2,daheight - 10 - 
            (d->ncols-1-j)*textheight, ix, iy);
          gdk_gc_set_line_attributes(gg->plot_GC, 1, GDK_LINE_SOLID, 
            GDK_CAP_ROUND, GDK_JOIN_ROUND);

          splot_text_extents (varlab, style, 
            &lbearing, &rbearing, &width, &ascent, &descent);

          splot_draw_string (varlab, dawidth/2+dawidth/4+10, iy,
            style, drawable, gg);
  
          /* vertical */
          ix = 10 + j*textheight;
          iy = daheight - (daheight/2 + 
            (gint) (dsp->tcorr2.F.vals[0][j]*
            (gfloat) daheight/4));
          gdk_gc_set_line_attributes(gg->plot_GC, 2, GDK_LINE_SOLID, 
            GDK_CAP_ROUND, GDK_JOIN_ROUND);
          if (j == dsp->tc2_manip_var)
            gdk_gc_set_foreground(gg->plot_GC, &gg->vcirc_manip_color);
          else
            gdk_gc_set_foreground(gg->plot_GC, &scheme->rgb_accent);
          gdk_draw_line(drawable, gg->plot_GC,
            10+j*textheight,daheight/2,
            ix, iy);

          g_free (varlab);
          /*-- can't add vertical variable labels --*/
        }     
        gdk_gc_set_line_attributes(gg->plot_GC, 0, GDK_LINE_SOLID, 
          GDK_CAP_ROUND, GDK_JOIN_ROUND);
        break;
    }
  }
}
