/* brush.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <stdlib.h>
#include <math.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/* */
gboolean active_paint_points (datad *d, ggobid *gg);
gboolean active_paint_lines (datad *d, ggobid *gg);
/* */

  /* corner (x1, y1); corner where the cursor goes (x2,y2) */

void
find_glyph_type_and_size(gint gid, glyphv *glyph)
{
  glyph->type = ( (gid-1) / (gint) NGLYPHSIZES ) + 1 ;
  glyph->size = ( (gid-1) % (gint) NGLYPHSIZES ) + 1 ;
}

gboolean
brush_once (gboolean force, datad *d, ggobid *gg)
{
/*
 * Determine which bins the brush is currently sitting in.
 * bin0 is the bin which contains of the upper left corner of the
 * brush; bin1 is the one containing of the lower right corner.
*/
  brush_coords *brush_pos = &d->brush_pos;
  gint ulx = MIN (brush_pos->x1, brush_pos->x2);
  gint uly = MIN (brush_pos->y1, brush_pos->y2);
  gint lrx = MAX (brush_pos->x1, brush_pos->x2);
  gint lry = MAX (brush_pos->y1, brush_pos->y2);
  gboolean changed = false;
  cpaneld *cpanel = &gg->current_display->cpanel;
  icoords *bin0 = &d->brush.bin0;
  icoords *bin1 = &d->brush.bin1;

  if (!point_in_which_bin (ulx, uly, &bin0->x, &bin0->y, d, gg)) {
    bin0->x = MAX (bin0->x, 0);
    bin0->x = MIN (bin0->x, d->brush.nbins - 1);
    bin0->y = MAX (bin0->y, 0);
    bin0->y = MIN (bin0->y, d->brush.nbins - 1);
  }
  if (!point_in_which_bin (lrx, lry, &bin1->x, &bin1->y, d, gg)) {
    bin1->x = MAX (bin1->x, 0);
    bin1->x = MIN (bin1->x, d->brush.nbins - 1);
    bin1->y = MAX (bin1->y, 0);
    bin1->y = MIN (bin1->y, d->brush.nbins - 1);
  }

/*
 * Now paint.
*/
  if (cpanel->br_scope == BR_POINTS || cpanel->br_scope == BR_PANDL) {
    changed = active_paint_points (d, gg);
  }

  if (cpanel->br_scope == BR_LINES || cpanel->br_scope == BR_PANDL) {
    changed = active_paint_lines (d, gg);
  }

  return (changed);
}

void
point_brush_prev_vectors_update (datad *d, ggobid *gg) {
  gint m, i;
  for (m=0; m<d->nrows_in_plot; m++) {
    i = d->rows_in_plot[m];
    d->color_prev.els[i] = d->color_ids.els[i];
    d->hidden_prev.els[i] = d->hidden.els[i];
    d->glyph_prev[i].size = d->glyph_ids[i].size;
    d->glyph_prev[i].type = d->glyph_ids[i].type;
  }
}

void
point_brush_undo (splotd *sp, datad *d, ggobid *gg) {
  gint m, i;
  for (m=0; m<d->nrows_in_plot; m++) {
    i = d->rows_in_plot[m];
    d->color_ids.els[i] = d->color_now.els[i] = d->color_prev.els[i];
    d->hidden.els[i] = d->hidden_now.els[i] = d->hidden_prev.els[i];
    d->glyph_ids[i].type = d->glyph_now[i].type = d->glyph_prev[i].type;
    d->glyph_ids[i].size = d->glyph_now[i].size = d->glyph_prev[i].size;
  }
  splot_redraw (sp, FULL, gg);
}

void
reinit_transient_brushing (datad *d, ggobid *gg)
{
/*
 * If a new variable is selected or a variable is transformed
 * during transient brushing, restore all points to the permanent
 * value, and then re-execute brush_once() to brush the points that
 * are now underneath the brush.  For now, don't make the
 * same change for persistent brushing.
*/
  gint i, m, k;
  displayd *dsp = gg->current_display;
  cpaneld *cpanel = &dsp->cpanel;
  gboolean point_painting_p =
     (cpanel->br_scope == BR_POINTS || cpanel->br_scope == BR_PANDL);
  gboolean line_painting_p =
     (cpanel->br_scope == BR_LINES || cpanel->br_scope == BR_PANDL);

  if (point_painting_p) {
    for (m=0; m<d->nrows_in_plot; m++) {
      i = d->rows_in_plot[m];
      d->color_now.els[i] = d->color_ids.els[i] ;
      d->glyph_now[i].type = d->glyph_ids[i].type;
      d->glyph_now[i].size = d->glyph_ids[i].size;
      d->hidden_now.els[i] = d->hidden.els[i];
    }
  }
  if (line_painting_p) {
    for (k=0; k<gg->nedges; m++) {
      gg->line.color_now.els[k] =
        gg->line.color_prev.els[k] =
        gg->line.color.els[k];
      gg->line.hidden_now.els[k] =
        gg->line.hidden_prev.els[k] =
        gg->line.hidden.els[k];
    }
  }

  brush_once (false, d, gg);
}

void
brush_set_pos (gint x, gint y, datad *d, ggobid *gg) {
  brush_coords *brush_pos = &d->brush_pos;
  gint xdist = brush_pos->x2 - brush_pos->x1 ;
  gint ydist = brush_pos->y2 - brush_pos->y1 ;
  /*
   * (x2,y2) is the corner that's moving.
  */
  brush_pos->x1 = x - xdist ;
  brush_pos->x2 = x ;
  brush_pos->y1 = y - ydist ;
  brush_pos->y2 = y ;
}


void
brush_motion (icoords *mouse, gboolean button1_p, gboolean button2_p,
  cpaneld *cpanel, datad *d, ggobid *gg)
{
  gboolean changed = false;
  splotd *sp = gg->current_splot;
  displayd *display = (displayd *) sp->displayptr;
  brush_coords *brush_pos = &d->brush_pos;

  if (button1_p) {
    if (display->displaytype == parcoords) {
/*
      if (mouse->x > sp->da->allocation.width || mouse->x < 0) {
        gint indx = g_list_index (display->splots, sp);
        gint nplots = g_list_length (display->splots);
        if (mouse->x > sp->da->allocation.width) {
          if (indx != nplots-1) {
            g_printerr ("slid off to the right\n");
          }
        } else if (mouse->x < 0) {
          if (indx > 0) {
            g_printerr ("slid off to the left\n");
          }
        }
      }
*/
    }

    brush_set_pos (mouse->x, mouse->y, d, gg);
  }

  else if (button2_p) {
    brush_pos->x2 = mouse->x ;
    brush_pos->y2 = mouse->y ;
  }


  if (cpanel->brush_on_p) {
    changed = brush_once (false, d, gg);
    if (display->options.edges_undirected_show_p ||
        display->options.edges_directed_show_p ||
        display->options.edges_show_p ||
        d->nrgroups > 0)      /*-- a full redraw is required --*/
    {
      splot_redraw (sp, FULL, gg);
      displays_plot (sp, FULL, gg);

    } else {  /*-- if we can get away with binning --*/

      if (changed) {
        splot_redraw (sp, BINNED, gg);
        displays_plot (sp, FULL, gg);
      } else {  /*-- just redraw the brush --*/
        splot_redraw (sp, QUICK, gg);  
      }
    }

  } else {  /*-- we're not brushing, and we just need to redraw the brush --*/
    splot_redraw (sp, QUICK, gg);
  }
}


gboolean
under_brush (gint k, ggobid *gg)
/*
 * Determine whether point k is under the brush.
*/
{
  splotd *sp = gg->current_splot;
  datad *d = gg->current_display->d;
  brush_coords *brush_pos = &d->brush_pos;
  gint pt;
  gint x1 = MIN (brush_pos->x1, brush_pos->x2);
  gint x2 = MAX (brush_pos->x1, brush_pos->x2);
  gint y1 = MIN (brush_pos->y1, brush_pos->y2);
  gint y2 = MAX (brush_pos->y1, brush_pos->y2);

  pt = (sp->screen[k].x <= x2 && sp->screen[k].y <= y2 &&
        sp->screen[k].x >= x1 && sp->screen[k].y >= y1) ? 1 : 0;
  return (pt);
}


/*----------------------------------------------------------------------*/
/*                      Dealing with the brush                          */
/*----------------------------------------------------------------------*/

static void
brush_boundaries_set (cpaneld *cpanel,
  icoords *obin0, icoords *obin1,
  icoords *imin, icoords *imax, datad *d, ggobid *gg)
{
  icoords *bin0 = &d->brush.bin0;
  icoords *bin1 = &d->brush.bin1;

  if (cpanel->br_mode == BR_TRANSIENT) {
    imin->x = MIN (bin0->x, obin0->x);
    imin->y = MIN (bin0->y, obin0->y);
    imax->x = MAX (bin1->x, obin1->x);
    imax->y = MAX (bin1->y, obin1->y);
  }
  else {
    imin->x = bin0->x;
    imin->y = bin0->y;
    imax->x = bin1->x;
    imax->y = bin1->y;
  }
}

void
brush_draw_label (splotd *sp, datad *d, ggobid *gg) {
  gint lbearing, rbearing, width, ascent, descent;
  GtkStyle *style = gtk_widget_get_style (sp->da);

  if (d->npts_under_brush > 0) {
    gchar *str = g_strdup_printf ("%d", d->npts_under_brush);
    gdk_text_extents (style->font, 
      str, strlen (str),
      &lbearing, &rbearing, &width, &ascent, &descent);
    gdk_draw_string (sp->pixmap1, style->font, gg->plot_GC,
      sp->max.x - width - 5,
      ascent + descent + 5,
      str);
    g_free (str);
  }
}

void
brush_draw_brush (splotd *sp, datad *d, ggobid *gg) {
/*
 * Use brush_pos to draw the brush.
*/
  cpaneld *cpanel = &gg->current_display->cpanel;
  gboolean point_painting_p =
     (cpanel->br_scope == BR_POINTS || cpanel->br_scope == BR_PANDL);
  gboolean line_painting_p =
     (cpanel->br_scope == BR_LINES || cpanel->br_scope == BR_PANDL);

  brush_coords *brush_pos = &d->brush_pos;
  gint x1 = MIN (brush_pos->x1, brush_pos->x2);
  gint x2 = MAX (brush_pos->x1, brush_pos->x2);
  gint y1 = MIN (brush_pos->y1, brush_pos->y2);
  gint y2 = MAX (brush_pos->y1, brush_pos->y2);

  if (!gg->mono_p) {
    if ((gg->default_color_table[gg->color_id].red != gg->bg_color.red) ||
        (gg->default_color_table[gg->color_id].blue != gg->bg_color.blue) ||
        (gg->default_color_table[gg->color_id].green != gg->bg_color.green))
    {
      gdk_gc_set_foreground (gg->plot_GC,
                             &gg->default_color_table[gg->color_id]);
    } else {
      gdk_gc_set_foreground (gg->plot_GC,
                             &gg->accent_color);
    }
  }

  if (point_painting_p)
  {
    gdk_draw_rectangle (sp->pixmap1, gg->plot_GC, false,
      x1, y1, (x2>x1)?(x2-x1):(x1-x2), (y2>y1)?(y2-y1):(y1-y2));
    /* Mark the corner to which the cursor will be attached */
    gdk_draw_rectangle (sp->pixmap1, gg->plot_GC, true,
      brush_pos->x2-1, brush_pos->y2-1, 2, 2);

    /*
     * highlight brush
    */
    if (cpanel->brush_on_p) {
      gdk_draw_rectangle (sp->pixmap1, gg->plot_GC, false,
        x1-1, y1-1, (x2>x1)?(x2-x1+2):(x1-x2+2), (y2>y1)?(y2-y1+2):(y1-y2+2)); 

      /* Mark the corner to which the cursor will be attached */
      gdk_draw_rectangle (sp->pixmap1, gg->plot_GC, true,
        brush_pos->x2-2, brush_pos->y2-2, 4, 4);
    }
  }

  if (line_painting_p) {
    gdk_draw_line (sp->pixmap1, gg->plot_GC,
      x1 + (x2 - x1)/2, y1, x1 + (x2 - x1)/2, y2 );
    gdk_draw_line (sp->pixmap1, gg->plot_GC,
      x1, y1 + (y2 - y1)/2, x2, y1 + (y2 - y1)/2 );

    if (cpanel->brush_on_p) {
      gdk_draw_line (sp->pixmap1, gg->plot_GC,
        x1 + (x2 - x1)/2 + 1, y1, x1 + (x2 - x1)/2 + 1, y2 );
      gdk_draw_line (sp->pixmap1, gg->plot_GC,
        x1, y1 + (y2 - y1)/2 + 1, x2, y1 + (y2 - y1)/2 + 1 );
    }
  }
}

/*----------------------------------------------------------------------*/
/*                      Glyph brushing                                  */
/*----------------------------------------------------------------------*/

static gboolean
update_glyph_vectors (gint i, gboolean changed, datad *d, ggobid *gg) {
  cpaneld *cpanel = &gg->current_display->cpanel;
  gboolean doit = true;

/* setting the value of changed */
  if (!changed) {
    if (d->pts_under_brush.els[i]) {

      doit = (d->glyph_now[i].size != gg->glyph_id.size);
      if (cpanel->br_target != BR_GSIZE)  /*-- ... if not ignoring type --*/
        doit = doit || (d->glyph_now[i].type != gg->glyph_id.type);

    } else {

      doit = (d->glyph_now[i].size != d->glyph_ids[i].size);
      if (cpanel->br_target != BR_GSIZE)  /*-- ... if not ignoring type --*/
        doit = doit || (d->glyph_now[i].type != d->glyph_ids[i].type);
    }
  }
/* */

  if (doit) {
    if (d->pts_under_brush.els[i]) {
      switch (cpanel->br_mode) {

        case BR_PERSISTENT:
          d->glyph_ids[i].size = d->glyph_now[i].size = gg->glyph_id.size;
          if (cpanel->br_target != BR_GSIZE)  /*-- ... if not ignoring type --*/
            d->glyph_ids[i].type = d->glyph_now[i].type = gg->glyph_id.type;
          break;

        case BR_TRANSIENT:
          d->glyph_now[i].size = gg->glyph_id.size;
          if (cpanel->br_target != BR_GSIZE)  /*-- ... if not ignoring type --*/
            d->glyph_now[i].type = gg->glyph_id.type;
          break;
      }
    } else {
      d->glyph_now[i].size = d->glyph_ids[i].size;
      if (cpanel->br_target != BR_GSIZE)  /*-- ... if not ignoring type --*/
        d->glyph_now[i].type = d->glyph_ids[i].type;
    }
  }

  return (changed);
}


static gboolean
build_glyph_vectors (datad *d, ggobid *gg)
{
  gint ih, iv, m, j, k, gp, n, p;
  static icoords obin0 = {BRUSH_NBINS/2, BRUSH_NBINS/2};
  static icoords obin1 = {BRUSH_NBINS/2, BRUSH_NBINS/2};
  icoords imin, imax;
  gboolean changed = false;
  cpaneld *cpanel = &gg->current_display->cpanel;

  brush_boundaries_set (cpanel, &obin0, &obin1, &imin, &imax, d, gg);

  for (ih=imin.x; ih<=imax.x; ih++) {
    for (iv=imin.y; iv<=imax.y; iv++) {
      for (m=0; m<d->brush.binarray[ih][iv].nels; m++) {
        /*
         * j is the row number; k is the index of rows_in_plot[]
        */
        j = d->rows_in_plot[ k = d->brush.binarray[ih][iv].els[m] ] ;


        /* update the glyph vectors for every member of the row group */
        if (d->nrgroups > 0) {
          gp = d->rgroup_ids[k];
          for (n=0; n<d->rgroups[gp].nels; n++) {
            p = d->rgroups[gp].els[n];
            changed = update_glyph_vectors (p, changed, d, gg);
          }
        /* */

        } else {  /* update the vectors for this point only */
          changed = update_glyph_vectors (j, changed, d, gg);
        }
      }
    }
  }

  obin0.x = d->brush.bin0.x;
  obin0.y = d->brush.bin0.y;
  obin1.x = d->brush.bin1.x;
  obin1.y = d->brush.bin1.y;

  return (changed);
}

/*----------------------------------------------------------------------*/
/*                      Color brushing                                  */
/*----------------------------------------------------------------------*/

static gboolean
update_color_vectors (gint i, gboolean changed, datad *d, ggobid *gg) {
  cpaneld *cpanel = &gg->current_display->cpanel;
  gboolean doit = true;

/* setting the value of doit */
  if (!changed) {
    if (d->pts_under_brush.els[i])
      doit = (d->color_now.els[i] != gg->color_id);
    else
      doit = (d->color_now.els[i] != d->color_ids.els[i]);
  }
/* */

  /*
   * If doit is false, it's guaranteed that there will be no change.
  */
  if (doit) {
    if (d->pts_under_brush.els[i]) {
      switch (cpanel->br_mode) {
        case BR_PERSISTENT:
          d->color_ids.els[i] = d->color_now.els[i] = gg->color_id;
          break;
        case BR_TRANSIENT:
          d->color_now.els[i] = gg->color_id;
          break;
      }
    } else d->color_now.els[i] = d->color_ids.els[i];
  }

  return (doit);
}

static gboolean
build_color_vectors (datad *d, ggobid *gg)
{
  gint ih, iv, m, j, k, gp, n, p;
  static icoords obin0 = {BRUSH_NBINS/2, BRUSH_NBINS/2};
  static icoords obin1 = {BRUSH_NBINS/2, BRUSH_NBINS/2};
  icoords imin, imax;
  gboolean changed = false;
  cpaneld *cpanel = &gg->current_display->cpanel;

  brush_boundaries_set (cpanel, &obin0, &obin1, &imin, &imax, d, gg);

  for (ih=imin.x; ih<=imax.x; ih++) {
    for (iv=imin.y; iv<=imax.y; iv++) {
      for (m=0; m<d->brush.binarray[ih][iv].nels; m++) {
        j = d->rows_in_plot[ k = d->brush.binarray[ih][iv].els[m] ] ;
        /*
         * k   raw index, based on nrows
         * j   index based on nrows_in_plot
        */


        /* update the color vectors for every member of the row group */
        if (d->nrgroups > 0) {
          gp = d->rgroup_ids[k];
          for (n=0; n<d->rgroups[gp].nels; n++) {
            p = d->rgroups[gp].els[n];
            changed = update_color_vectors (p, changed, d, gg);
          }
        /* */

        } else {  /* update the vectors for this point only */
          changed = update_color_vectors (j, changed, d, gg);
        }
      }
    }
    obin0.x = d->brush.bin0.x;
    obin0.y = d->brush.bin0.y;
    obin1.x = d->brush.bin1.x;
    obin1.y = d->brush.bin1.y;
  }

  return (changed);
}

/*----------------------------------------------------------------------*/
/*                      Hide brushing                                   */
/*----------------------------------------------------------------------*/

static gboolean
update_hidden_vectors (gint i, gboolean changed, datad *d, ggobid *gg) {
  cpaneld *cpanel = &gg->current_display->cpanel;
  gboolean doit = true;

  /*
   * First find out if this will result in a change; this in
   * order to be able to return that information.
  */
  if (!changed) {
    if (d->pts_under_brush.els[i])
      doit = (d->hidden_now.els[i] != true);
    else
      doit = (d->hidden_now.els[i] != d->hidden.els[i]);
  }
/* */

/*
 * If doit is false, it's guaranteed that there will be no change.
*/

  if (doit) {
    if (d->pts_under_brush.els[i]) {
      switch (cpanel->br_mode) {
        case BR_PERSISTENT:
          d->hidden.els[i] = d->hidden_now.els[i] = true;
          break;
        case BR_TRANSIENT:
          d->hidden_now.els[i] = true;
          break;
      }
    } else d->hidden_now.els[i] = d->hidden.els[i];
  }

  return (doit);
}

static gboolean
build_hidden_vectors (datad *d, ggobid *gg)
{
  gint ih, iv, m, j, k, gp, n, p;
  static icoords obin0 = {BRUSH_NBINS/2, BRUSH_NBINS/2};
  static icoords obin1 = {BRUSH_NBINS/2, BRUSH_NBINS/2};
  icoords imin, imax;
  gboolean changed = false;
  cpaneld *cpanel = &gg->current_display->cpanel;

  brush_boundaries_set (cpanel, &obin0, &obin1, &imin, &imax, d, gg);

  for (ih=imin.x; ih<=imax.x; ih++) {
    for (iv=imin.y; iv<=imax.y; iv++) {
      for (m=0; m<d->brush.binarray[ih][iv].nels; m++) {
        j = d->rows_in_plot[ k = d->brush.binarray[ih][iv].els[m] ] ;
        /*
         * k   raw index, based on nrows
         * j   index based on nrows_in_plot
        */

        if (d->nrgroups > 0) {
          /*-- update the hidden vectors for every member of the row group --*/
          gp = d->rgroup_ids[k];
          for (n=0; n<d->rgroups[gp].nels; n++) {
            p = d->rgroups[gp].els[n];
            changed = update_hidden_vectors (p, changed, d, gg);
          }
        /* */

        } else {  /* update the vectors for this point only */
          changed = update_hidden_vectors (j, changed, d, gg);
        }
      }
    }
    obin0.x = d->brush.bin0.x;
    obin0.y = d->brush.bin0.y;
    obin1.x = d->brush.bin1.x;
    obin1.y = d->brush.bin1.y;
  }

  return (changed);
}

gboolean
active_paint_points (datad *d, ggobid *gg)
{
  gint ih, iv, j, pt, k, gp;
  gboolean changed;
  cpaneld *cpanel = &gg->current_display->cpanel;
/*
 * Set pts_under_brush[j] to 1 if point j is inside the rectangular brush.
*/

  /* Zero out pts_under_brush[] before looping */
  d->npts_under_brush = 0;
  for (j=0; j<d->nrows_in_plot; j++)
    d->pts_under_brush.els[d->rows_in_plot[j]] = 0;
 
  /*
   * brush.binarray[][] only represents the
   * rows in rows_in_plot[] so there's no need to test for that.
  */

  for (ih=d->brush.bin0.x; ih<=d->brush.bin1.x; ih++) {
    for (iv=d->brush.bin0.y; iv<=d->brush.bin1.y; iv++) {
      for (j=0; j<d->brush.binarray[ih][iv].nels; j++) {
        pt = d->rows_in_plot[d->brush.binarray[ih][iv].els[j]];

        if (under_brush (pt, gg)) {

          d->npts_under_brush++ ;
          d->pts_under_brush.els[pt] = 1;

          /* brush other members of this row group */
          if (d->nrgroups > 0) {
            gp = d->rgroup_ids[pt];
            if (gp < d->nrgroups) {  /* exclude points without an rgroup */
              for (k=0; k<d->rgroups[gp].nels; k++) {
                  d->pts_under_brush.els[d->rgroups[gp].els[k]] = 1;
              }
            }
          }
          /* */
        }
      }
    }
  }

  changed = false;

  if (cpanel->brush_on_p) {
    switch (cpanel->br_target) {
      case BR_CANDG:  /*-- color and glyph --*/
        if (build_color_vectors (d, gg)) changed = true;
        if (build_glyph_vectors (d, gg)) changed = true;
        break;
      case BR_COLOR:
        if (build_color_vectors (d, gg)) changed = true;
        break;
      case BR_GLYPH:  /*-- glyph type and size --*/
        if (build_glyph_vectors (d, gg)) changed = true;
        break;
      case BR_GSIZE:  /*-- glyph size only --*/
        if (build_glyph_vectors (d, gg)) changed = true;
        break;
      case BR_HIDE:  /*-- hidden --*/
        if (build_hidden_vectors (d, gg)) changed = true;
        break;
    }
  }

  return (changed);
}

/*----------------------------------------------------------------------*/
/*                      Line brushing                                   */
/*----------------------------------------------------------------------*/

void
line_brush_prev_vectors_update (ggobid *gg) {
  vectors_copy (&gg->line.color, &gg->line.color_prev);
  vectorb_copy (&gg->line.hidden, &gg->line.hidden_prev);
}

void
line_brush_undo (splotd *sp, ggobid *gg) {
  gint k;
  for (k=0; k<gg->nedges; k++) {
    gg->line.color.els[k] =
      gg->line.color_now.els[k] =
      gg->line.color_prev.els[k];
    gg->line.hidden.els[k] =
      gg->line.hidden_now.els[k] =
      gg->line.hidden_prev.els[k];
  }
  splot_redraw (sp, FULL, gg);
}

gboolean
xed_by_brush (gint k, datad *d, ggobid *gg)
/*
 * Determine whether line k intersects the brush
*/
{
  splotd *sp = gg->current_splot;
  gboolean intersect;
  glong x1 = d->brush_pos.x1;
  glong y1 = d->brush_pos.y1;
  glong x2 = d->brush_pos.x2;
  glong y2 = d->brush_pos.y2;

  glong ax = sp->screen[gg->edge_endpoints[k].a - 1].x;
  glong ay = sp->screen[gg->edge_endpoints[k].a - 1].y;
  glong bx = sp->screen[gg->edge_endpoints[k].b - 1].x;
  glong by = sp->screen[gg->edge_endpoints[k].b - 1].y;

  glong x, y;
  extern gint lines_intersect (glong, glong, glong, glong, 
    glong, glong, glong, glong, glong *, glong *);

  /*-- test for intersection with the vertical line --*/
  intersect = lines_intersect (x1 + (x2 - x1)/2, y1, x1 + (x2 - x1)/2, y2,
     ax, ay, bx, by, &x, &y);

  if (!intersect)
    intersect = lines_intersect (x1, y1 + (y2 - y1)/2, x2, y1 + (y2 - y1)/2,
      ax, ay, bx, by, &x, &y);

  return (intersect);
}

static gboolean
update_line_color_vectors (gint k, gboolean changed, ggobid *gg) {
  cpaneld *cpanel = &gg->current_display->cpanel;
  gboolean doit = true;

/* setting the value of doit */
  if (!changed) {
    if (gg->line.xed_by_brush.els[k])
      doit = (gg->line.color_now.els[k] != gg->color_id);
    else
      doit = (gg->line.color_now.els[k] != gg->line.color.els[k]);
  }
/* */

  /*
   * If doit is false, it's guaranteed that there will be no change.
  */
  if (doit) {
    if (gg->line.xed_by_brush.els[k]) {
      switch (cpanel->br_mode) {
        case BR_PERSISTENT:
          gg->line.color.els[k] = gg->line.color_now.els[k] = gg->color_id;
          break;
        case BR_TRANSIENT:
          gg->line.color_now.els[k] = gg->color_id;
          break;
      }
    } else gg->line.color_now.els[k] = gg->line.color.els[k];
  }

  return (doit);
}

static gboolean
build_line_color_vectors (ggobid *gg)
{
  gint k;
  gboolean changed = false;

  for (k=0; k<gg->nedges; k++) {

    /* update the line color vectors for every member of the line group */
    if (gg->nlgroups > 0) {
/*
      gint n, p, gp;
      gp = gg->lgroup_ids[k];
      for (n=0; n<gg->lgroups[gp].nels; n++) {
        p = gg->lgroups[gp].els[n];
        changed = update_line_color_vectors (p, changed, gg);
      }
*/
    /* */

    } else {  /* update the vectors for this point only */
      changed = update_line_color_vectors (k, changed, gg);
    }
  }

  return (changed);
}

static gboolean
update_line_hidden_vectors (gint k, gboolean changed, ggobid *gg) {
  cpaneld *cpanel = &gg->current_display->cpanel;
  gboolean doit = true;

/* setting the value of doit */
  if (!changed) {
    if (gg->line.xed_by_brush.els[k])
      doit = (gg->line.hidden_now.els[k] != true);
    else
      doit = (gg->line.hidden_now.els[k] != gg->line.hidden.els[k]);
  }
/* */

  /*
   * If doit is false, it's guaranteed that there will be no change.
  */
  if (doit) {
    if (gg->line.xed_by_brush.els[k]) {
      switch (cpanel->br_mode) {
        case BR_PERSISTENT:
          gg->line.hidden.els[k] = gg->line.hidden_now.els[k] = true;
          break;
        case BR_TRANSIENT:
          gg->line.hidden_now.els[k] = true;
          break;
      }
    } else gg->line.hidden_now.els[k] = gg->line.hidden.els[k];
  }

  return (doit);
}

static gboolean
build_line_hidden_vectors (ggobid *gg)
{
  gint k;
  gboolean changed = false;

  for (k=0; k<gg->nedges; k++) {

    /* update the line hidden vectors for every member of the line group */
    if (gg->nlgroups > 0) {
/*
      gint n, p, gp;
      gp = gg->lgroup_ids[k];
      for (n=0; n<gg->lgroups[gp].nels; n++) {
        p = gg->lgroups[gp].els[n];
        changed = update_line_hidden_vectors (p, changed, gg);
      }
*/
    /* */

    } else {  /* update the vectors for this point only */
      changed = update_line_hidden_vectors (k, changed, gg);
    }
  }

  return changed;
}

gboolean
active_paint_lines (datad *d, ggobid *gg)
{
  gint k;
  gboolean changed;
  cpaneld *cpanel = &gg->current_display->cpanel;

  /* Zero out xed_by_brush[] before looping */
  gg->line.nxed_by_brush = 0;
  for (k=0; k<gg->nedges; k++)
    gg->line.xed_by_brush.els[k] = false;
 
  for (k=0; k<gg->nedges; k++) {

    if (xed_by_brush (k, d, gg)) {

      gg->line.nxed_by_brush++ ;
      gg->line.xed_by_brush.els[k] = true;

      /* brush other members of this line group */
      if (gg->nlgroups > 0) {
/*
        gint p, gp;
        gp = gg->lgroup_ids[k];
        if (gp < gg->nlgroups) {
          for (p=0; p<gg->rgroups[gp].nels; p++) {
              gg->xed_by_brush.els[gg->lgroups[gp].els[p]] = 1;
          }
        }
*/
      }
      /* */
    }
  }

  changed = false;

  if (cpanel->brush_on_p) {
    switch (cpanel->br_target) {
      case BR_CANDG:  /*-- color and glyph --*/
      case BR_COLOR:  /*-- color --*/
        if (build_line_color_vectors (gg)) changed = true;
        break;
      case BR_HIDE:  /*-- hidden --*/
        if (build_line_hidden_vectors (gg)) changed = true;
        break;
    }
  }

  return (changed);
}
