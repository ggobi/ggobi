/* brush.c */

#include <stdlib.h>
#include <math.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/* */
gboolean active_paint_points ();
/* */

static gulong npts_under_brush = 0;


  /* corner (x1, y1); corner where the cursor goes (x2,y2) */

void
find_glyph_type_and_size(gint gid, glyphv *glyph)
{
  glyph->type = ( (gid-1) / (gint) NGLYPHSIZES ) + 1 ;
  glyph->size = ( (gid-1) % (gint) NGLYPHSIZES ) + 1 ;
}

gboolean
brush_once (gboolean force)
{
/*
 * Determine which bins the brush is currently sitting in.
 * bin0 is the bin which contains of the upper left corner of the
 * brush; bin1 is the one containing of the lower right corner.
*/
  gint ulx = MIN (brush_pos.x1, brush_pos.x2);
  gint uly = MIN (brush_pos.y1, brush_pos.y2);
  gint lrx = MAX (brush_pos.x1, brush_pos.x2);
  gint lry = MAX (brush_pos.y1, brush_pos.y2);
  gboolean changed = false;
  cpaneld *cpanel = &xg.current_display->cpanel;

  if (!point_in_which_bin (ulx, uly, &xg.bin0.x, &xg.bin0.y)) {
    xg.bin0.x = MAX (xg.bin0.x, 0);
    xg.bin0.x = MIN (xg.bin0.x, xg.br_nbins - 1);
    xg.bin0.y = MAX (xg.bin0.y, 0);
    xg.bin0.y = MIN (xg.bin0.y, xg.br_nbins - 1);
  }
  if (!point_in_which_bin (lrx, lry, &xg.bin1.x, &xg.bin1.y)) {
    xg.bin1.x = MAX (xg.bin1.x, 0);
    xg.bin1.x = MIN (xg.bin1.x, xg.br_nbins - 1);
    xg.bin1.y = MAX (xg.bin1.y, 0);
    xg.bin1.y = MIN (xg.bin1.y, xg.br_nbins - 1);
  }

/*
 * Now paint.
*/
  if (cpanel->br_scope == BR_POINTS || cpanel->br_scope == BR_PANDL) {
    changed = active_paint_points ();
  }

/*
  if (xg.is_line_painting)
    active_paint_lines ();
*/

  return (changed);
}

void
brush_prev_vectors_update (void) {
  gint m, i;
  for (m=0; m<xg.nrows_in_plot; m++) {
    i = xg.rows_in_plot[m];
    xg.color_prev[i] = xg.color_ids[i];
    xg.hidden_prev[i] = xg.hidden[i];
    xg.glyph_prev[i].size = xg.glyph_ids[i].size;
    xg.glyph_prev[i].type = xg.glyph_ids[i].type;
  }
}

void
brush_undo (splotd *sp) {
  gint m, i;
  for (m=0; m<xg.nrows_in_plot; m++) {
    i = xg.rows_in_plot[m];
    xg.color_ids[i] = xg.color_now[i] = xg.color_prev[i];
    xg.hidden[i] = xg.hidden_now[i] = xg.hidden_prev[i];
    xg.glyph_ids[i].type = xg.glyph_now[i].type = xg.glyph_prev[i].type;
    xg.glyph_ids[i].size = xg.glyph_now[i].size = xg.glyph_prev[i].size;
  }
  splot_redraw (sp, FULL);
}

void
reinit_transient_brushing (void)
{
/*
 * If a new variable is selected or a variable is transformed
 * during transient brushing,
 * restore all points to the permanent value, and then
 * re-execute brush_once() to brush the points that are
 * now underneath the brush.  For now, don't make the
 * same change for persistent or undo brushing.
*/
  gint i, m;

  for (m=0; m<xg.nrows_in_plot; m++)
  {
    i = xg.rows_in_plot[m];
    xg.color_now[i] = xg.color_ids[i] ;
    xg.glyph_now[i].type = xg.glyph_ids[i].type;
    xg.glyph_now[i].size = xg.glyph_ids[i].size;
    xg.hidden_now[i] = xg.hidden[i];
  }
  (void) brush_once (false);
}

void
brush_set_pos (gint x, gint y) {
  gint xdist = brush_pos.x2 - brush_pos.x1 ;
  gint ydist = brush_pos.y2 - brush_pos.y1 ;
  /*
   * (x2,y2) is the corner that's moving.
  */
  brush_pos.x1 = x - xdist ;
  brush_pos.x2 = x ;
  brush_pos.y1 = y - ydist ;
  brush_pos.y2 = y ;
}


void
brush_motion (icoords *mouse, gboolean button1_p, gboolean button2_p,
  cpaneld *cpanel)
{
  gboolean changed = false;
  splotd *sp = xg.current_splot;
  displayd *display = (displayd *) sp->displayptr;

  if (button1_p)
    brush_set_pos (mouse->x, mouse->y);

  else if (button2_p) {
    brush_pos.x2 = mouse->x ;
    brush_pos.y2 = mouse->y ;
  }


  if (cpanel->brush_on_p) {
    changed = brush_once (false);
    if (display->segments_undirected_show_p ||
        display->segments_directed_show_p ||
        display->segments_show_p ||
        xg.nrgroups > 0)      /*-- a full redraw is required --*/
    {
      splot_redraw (sp, FULL);
      displays_plot (sp);

    } else {  /*-- if we can get away with binning --*/

      if (changed) {
        splot_redraw (sp, BINNED);
        displays_plot (sp);
      } else {  /*-- just redraw the brush --*/
        splot_redraw (sp, QUICK);  
      }
    }

  } else {  /*-- we're not brushing, and we just need to redraw the brush --*/
    splot_redraw (sp, QUICK);
  }
}


gint
under_brush (gint k)
/*
 * Determine if point is under the brush.
*/
{
  splotd *sp = xg.current_splot;
  gint pt;
  gint x1 = MIN (brush_pos.x1, brush_pos.x2);
  gint x2 = MAX (brush_pos.x1, brush_pos.x2);
  gint y1 = MIN (brush_pos.y1, brush_pos.y2);
  gint y2 = MAX (brush_pos.y1, brush_pos.y2);

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
  icoords *imin, icoords *imax)
{
  if (cpanel->br_mode == BR_TRANSIENT) {
    imin->x = MIN (xg.bin0.x, obin0->x);
    imin->y = MIN (xg.bin0.y, obin0->y);
    imax->x = MAX (xg.bin1.x, obin1->x);
    imax->y = MAX (xg.bin1.y, obin1->y);
  }
  else {
    imin->x = xg.bin0.x;
    imin->y = xg.bin0.y;
    imax->x = xg.bin1.x;
    imax->y = xg.bin1.y;
  }
}

void
brush_draw_label (splotd *sp) {
  gint lbearing, rbearing, width, ascent, descent;
  GtkStyle *style = gtk_widget_get_style (sp->da);

  if (npts_under_brush > 0) {
    gchar *str = g_strdup_printf ("%ld", npts_under_brush);
    gdk_text_extents (style->font, 
      str, strlen (str),
      &lbearing, &rbearing, &width, &ascent, &descent);
    gdk_draw_string (sp->pixmap1, style->font, xg.plot_GC,
      sp->max.x - width - 5,
      ascent + descent + 5,
      str);
    g_free (str);
  }
}

void
brush_draw_brush (splotd *sp) {
/*
 * Use brush_pos to draw the brush.
*/
  cpaneld *cpanel = &xg.current_display->cpanel;
  gboolean point_painting_p =
     (cpanel->br_scope == BR_POINTS || cpanel->br_scope == BR_PANDL);
  gboolean line_painting_p =
     (cpanel->br_scope == BR_LINES || cpanel->br_scope == BR_PANDL);

  gint x1 = MIN (brush_pos.x1, brush_pos.x2);
  gint x2 = MAX (brush_pos.x1, brush_pos.x2);
  gint y1 = MIN (brush_pos.y1, brush_pos.y2);
  gint y2 = MAX (brush_pos.y1, brush_pos.y2);

  if (!xg.mono_p) {
    if ((xg.default_color_table[xg.color_id].red != xg.bg_color.red) ||
        (xg.default_color_table[xg.color_id].blue != xg.bg_color.blue) ||
        (xg.default_color_table[xg.color_id].green != xg.bg_color.green))
    {
      gdk_gc_set_foreground (xg.plot_GC, &xg.default_color_table[xg.color_id]);
    } else {
      gdk_gc_set_foreground (xg.plot_GC, &xg.accent_color);
    }
  }

  if (point_painting_p)
  {
    gdk_draw_rectangle (sp->pixmap1, xg.plot_GC, false,
      x1, y1, (x2>x1)?(x2-x1):(x1-x2), (y2>y1)?(y2-y1):(y1-y2));
    /* Mark the corner to which the cursor will be attached */
    gdk_draw_rectangle (sp->pixmap1, xg.plot_GC, true,
      brush_pos.x2-1, brush_pos.y2-1, 2, 2);

    /*
     * highlight brush
    */
    if (cpanel->brush_on_p) {
      gdk_draw_rectangle (sp->pixmap1, xg.plot_GC, false,
        x1-1, y1-1, (x2>x1)?(x2-x1+2):(x1-x2+2), (y2>y1)?(y2-y1+2):(y1-y2+2)); 

      /* Mark the corner to which the cursor will be attached */
      gdk_draw_rectangle (sp->pixmap1, xg.plot_GC, true,
        brush_pos.x2-2, brush_pos.y2-2, 4, 4);
    }
  }

  if (line_painting_p) {
    gdk_draw_line (sp->pixmap1, xg.plot_GC,
      x1 + (x2 - x1)/2, y1, x1 + (x2 - x1)/2, y2 );
    gdk_draw_line (sp->pixmap1, xg.plot_GC,
      x1, y1 + (y2 - y1)/2, x2, y1 + (y2 - y1)/2 );

    if (cpanel->brush_on_p) {
      gdk_draw_line (sp->pixmap1, xg.plot_GC,
        x1 + (x2 - x1)/2 + 1, y1, x1 + (x2 - x1)/2 + 1, y2 );
      gdk_draw_line (sp->pixmap1, xg.plot_GC,
        x1, y1 + (y2 - y1)/2 + 1, x2, y1 + (y2 - y1)/2 + 1 );
    }
  }
}

/*----------------------------------------------------------------------*/
/*                      Glyph brushing                                  */
/*----------------------------------------------------------------------*/

static gboolean
update_glyph_arrays (gint i, gboolean changed) {
  cpaneld *cpanel = &xg.current_display->cpanel;
  gboolean doit = true;

/* setting the value of changed */
  if (!changed) {
    if (xg.under_new_brush[i]) {

      doit = (xg.glyph_now[i].size != xg.glyph_id.size);
      if (cpanel->br_target != BR_GSIZE)  /*-- ... if not ignoring type --*/
        doit = doit || (xg.glyph_now[i].type != xg.glyph_id.type);

    } else {

      doit = (xg.glyph_now[i].size != xg.glyph_ids[i].size);
      if (cpanel->br_target != BR_GSIZE)  /*-- ... if not ignoring type --*/
        doit = doit || (xg.glyph_now[i].type != xg.glyph_ids[i].type);
    }
  }
/* */

  if (doit) {
    if (xg.under_new_brush[i]) {
      switch (cpanel->br_mode) {

        case BR_PERSISTENT:
          xg.glyph_ids[i].size = xg.glyph_now[i].size = xg.glyph_id.size;
          if (cpanel->br_target != BR_GSIZE)  /*-- ... if not ignoring type --*/
            xg.glyph_ids[i].type = xg.glyph_now[i].type = xg.glyph_id.type;
          break;

        case BR_TRANSIENT:
          xg.glyph_now[i].size = xg.glyph_id.size;
          if (cpanel->br_target != BR_GSIZE)  /*-- ... if not ignoring type --*/
            xg.glyph_now[i].type = xg.glyph_id.type;
          break;
      }
    } else {
      xg.glyph_now[i].size = xg.glyph_ids[i].size;
      if (cpanel->br_target != BR_GSIZE)  /*-- ... if not ignoring type --*/
        xg.glyph_now[i].type = xg.glyph_ids[i].type;
    }
  }

  return (changed);
}


static gboolean
build_glyph_vectors ()
{
  gint ih, iv, m, j, k, gp, n, p;
  static icoords obin0 = {BRUSH_NBINS/2, BRUSH_NBINS/2};
  static icoords obin1 = {BRUSH_NBINS/2, BRUSH_NBINS/2};
  icoords imin, imax;
  gboolean changed = false;
  cpaneld *cpanel = &xg.current_display->cpanel;

  brush_boundaries_set (cpanel, &obin0, &obin1, &imin, &imax);

  for (ih=imin.x; ih<=imax.x; ih++) {
    for (iv=imin.y; iv<=imax.y; iv++) {
      for (m=0; m<xg.br_binarray[ih][iv].nels; m++) {
        /*
         * j is the row number; k is the index of rows_in_plot[]
        */
        j = xg.rows_in_plot[ k = xg.br_binarray[ih][iv].els[m] ] ;

        if (j < xg.nlinkable) {

          /* update the glyph arrays for every member of the row group */
          if (xg.nrgroups > 0) {
            gp = xg.rgroup_ids[k];
            for (n=0; n<xg.rgroups[gp].nels; n++) {
              p = xg.rgroups[gp].els[n];
              changed = update_glyph_arrays (p, changed);
            }
          /* */

          } else {  /* update the arrays for this point only */
            changed = update_glyph_arrays (j, changed);
          }
        }
      }
    }
  }

  obin0.x = xg.bin0.x;
  obin0.y = xg.bin0.y;
  obin1.x = xg.bin1.x;
  obin1.y = xg.bin1.y;

  return (changed);
}

/*----------------------------------------------------------------------*/
/*                      Color brushing                                  */
/*----------------------------------------------------------------------*/

static gboolean
update_color_arrays (gint i, gboolean changed) {
  cpaneld *cpanel = &xg.current_display->cpanel;
  gboolean doit = true;

/* setting the value of doit */
  if (!changed) {
    if (xg.under_new_brush[i])
      doit = (xg.color_now[i] != xg.color_id);
    else
      doit = (xg.color_now[i] != xg.color_ids[i]);
  }
/* */

  /*
   * If doit is false, it's guaranteed that there will be no change.
  */
  if (doit) {
    if (xg.under_new_brush[i]) {
      switch (cpanel->br_mode) {
        case BR_PERSISTENT:
          xg.color_ids[i] = xg.color_now[i] = xg.color_id;
          break;
        case BR_TRANSIENT:
          xg.color_now[i] = xg.color_id;
          break;
      }
    } else xg.color_now[i] = xg.color_ids[i];
  }

  return (doit);
}

static gboolean
build_color_vectors (void)
{
  gint ih, iv, m, j, k, gp, n, p;
  static icoords obin0 = {BRUSH_NBINS/2, BRUSH_NBINS/2};
  static icoords obin1 = {BRUSH_NBINS/2, BRUSH_NBINS/2};
  icoords imin, imax;
  gboolean changed = false;
  cpaneld *cpanel = &xg.current_display->cpanel;

  brush_boundaries_set (cpanel, &obin0, &obin1, &imin, &imax);

  for (ih=imin.x; ih<=imax.x; ih++) {
    for (iv=imin.y; iv<=imax.y; iv++) {
      for (m=0; m<xg.br_binarray[ih][iv].nels; m++) {
        j = xg.rows_in_plot[ k = xg.br_binarray[ih][iv].els[m] ] ;
        /*
         * k   raw index, based on nrows
         * j   index based on nrows_in_plot
        */
        if (j < xg.nlinkable) {

          /* update the color arrays for every member of the row group */
          if (xg.nrgroups > 0) {
            gp = xg.rgroup_ids[k];
            for (n=0; n<xg.rgroups[gp].nels; n++) {
              p = xg.rgroups[gp].els[n];
              changed = update_color_arrays (p, changed);
            }
          /* */

          } else {  /* update the arrays for this point only */
            changed = update_color_arrays (j, changed);
          }
        }
      }
    }
    obin0.x = xg.bin0.x;
    obin0.y = xg.bin0.y;
    obin1.x = xg.bin1.x;
    obin1.y = xg.bin1.y;
  }

  return (changed);
}

/*----------------------------------------------------------------------*/
/*                      Hide brushing                                   */
/*----------------------------------------------------------------------*/

static gboolean
update_hidden_arrays (gint i, gboolean changed) {
  cpaneld *cpanel = &xg.current_display->cpanel;
  gboolean doit = true;

  /*
   * First find out if this will result in a change; this in
   * order to be able to return that information.
  */
  if (!changed) {
    if (xg.under_new_brush[i])
      doit = (xg.hidden_now[i] != true);
    else
      doit = (xg.hidden_now[i] != xg.hidden[i]);
  }
/* */

/*
 * If doit is false, it's guaranteed that there will be no change.
*/

  if (doit) {
    if (xg.under_new_brush[i]) {
      switch (cpanel->br_mode) {
        case BR_PERSISTENT:
          xg.hidden[i] = xg.hidden_now[i] = true;
          break;
        case BR_TRANSIENT:
          xg.hidden_now[i] = true;
          break;
      }
    } else xg.hidden_now[i] = xg.hidden[i];
  }

  return (doit);
}

static gboolean
build_hidden_vectors (void)
{
  gint ih, iv, m, j, k, gp, n, p;
  static icoords obin0 = {BRUSH_NBINS/2, BRUSH_NBINS/2};
  static icoords obin1 = {BRUSH_NBINS/2, BRUSH_NBINS/2};
  icoords imin, imax;
  gboolean changed = false;
  cpaneld *cpanel = &xg.current_display->cpanel;

  brush_boundaries_set (cpanel, &obin0, &obin1, &imin, &imax);

  for (ih=imin.x; ih<=imax.x; ih++) {
    for (iv=imin.y; iv<=imax.y; iv++) {
      for (m=0; m<xg.br_binarray[ih][iv].nels; m++) {
        j = xg.rows_in_plot[ k = xg.br_binarray[ih][iv].els[m] ] ;
        /*
         * k   raw index, based on nrows
         * j   index based on nrows_in_plot
        */
        if (j < xg.nlinkable) {

          if (xg.nrgroups > 0) {
            /*-- update the hidden arrays for every member of the row group --*/
            gp = xg.rgroup_ids[k];
            for (n=0; n<xg.rgroups[gp].nels; n++) {
              p = xg.rgroups[gp].els[n];
              changed = update_hidden_arrays (p, changed);
            }
          /* */

          } else {  /* update the arrays for this point only */
            changed = update_hidden_arrays (j, changed);
          }
        }
      }
    }
    obin0.x = xg.bin0.x;
    obin0.y = xg.bin0.y;
    obin1.x = xg.bin1.x;
    obin1.y = xg.bin1.y;
  }

  return (changed);
}

gboolean
active_paint_points (void)
{
  gint ih, iv, j, pt, k, gp;
  gboolean changed;
  cpaneld *cpanel = &xg.current_display->cpanel;
/*
 * Set under_new_brush[j] to 1 if point j is inside the rectangular brush.
*/

  /* Zero out under_new_brush[] before looping */
  npts_under_brush = 0;
  for (j=0; j<xg.nrows_in_plot; j++)
    xg.under_new_brush[xg.rows_in_plot[j]] = 0;
 
  /*
   * br_binarray[][] only represents the
   * rows in rows_in_plot[] so there's no need to test for that.
  */
  for (ih=xg.bin0.x; ih<=xg.bin1.x; ih++) {
    for (iv=xg.bin0.y; iv<=xg.bin1.y; iv++) {
      for (j=0; j<xg.br_binarray[ih][iv].nels; j++) {
        pt = xg.rows_in_plot[xg.br_binarray[ih][iv].els[j]];

        if (pt < xg.nlinkable) {
/*          if (!xg.hidden[pt] && under_brush (pt)) {*/
          if (under_brush (pt)) {

            npts_under_brush++ ;
            xg.under_new_brush[pt] = 1;

            /* brush other members of this row group */
            if (xg.nrgroups > 0) {
              gp = xg.rgroup_ids[pt];
              if (gp < xg.nrgroups) {  /* exclude points without an rgroup */
                for (k=0; k<xg.rgroups[gp].nels; k++) {
/*                  if (!xg.hidden[ xg.rgroups[gp].els[k] ])*/
                    xg.under_new_brush[xg.rgroups[gp].els[k]] = 1;
                }
              }
            }
            /* */

          }
        }
      }
    }
  }

  changed = false;

  if (cpanel->brush_on_p) {
    switch (cpanel->br_target) {
      case BR_CANDG:  /*-- color and glyph --*/
        if (build_color_vectors ()) changed = true;
        if (build_glyph_vectors ()) changed = true;
        break;
      case BR_COLOR:
        if (build_color_vectors ()) changed = true;
        break;
      case BR_GLYPH:  /*-- glyph type and size --*/
        if (build_glyph_vectors ()) changed = true;
        break;
      case BR_GSIZE:  /*-- glyph size only --*/
        if (build_glyph_vectors ()) changed = true;
        break;
      case BR_HIDE:  /*-- hidden --*/
        if (build_hidden_vectors ()) changed = true;
        break;
    }
  }

  return (changed);
}
