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
  gint ulx = MIN (gg.app.brush_pos.x1, gg.app.brush_pos.x2);
  gint uly = MIN (gg.app.brush_pos.y1, gg.app.brush_pos.y2);
  gint lrx = MAX (gg.app.brush_pos.x1, gg.app.brush_pos.x2);
  gint lry = MAX (gg.app.brush_pos.y1, gg.app.brush_pos.y2);
  gboolean changed = false;
  cpaneld *cpanel = &gg.current_display->cpanel;

  if (!point_in_which_bin (ulx, uly, &gg.bin0.x, &gg.bin0.y)) {
    gg.bin0.x = MAX (gg.bin0.x, 0);
    gg.bin0.x = MIN (gg.bin0.x, gg.br_nbins - 1);
    gg.bin0.y = MAX (gg.bin0.y, 0);
    gg.bin0.y = MIN (gg.bin0.y, gg.br_nbins - 1);
  }
  if (!point_in_which_bin (lrx, lry, &gg.bin1.x, &gg.bin1.y)) {
    gg.bin1.x = MAX (gg.bin1.x, 0);
    gg.bin1.x = MIN (gg.bin1.x, gg.br_nbins - 1);
    gg.bin1.y = MAX (gg.bin1.y, 0);
    gg.bin1.y = MIN (gg.bin1.y, gg.br_nbins - 1);
  }

/*
 * Now paint.
*/
  if (cpanel->br_scope == BR_POINTS || cpanel->br_scope == BR_PANDL) {
    changed = active_paint_points ();
  }

/*
  if (gg.is_line_painting)
    active_paint_lines ();
*/

  return (changed);
}

void
brush_prev_vectors_update (void) {
  gint m, i;
  for (m=0; m<gg.nrows_in_plot; m++) {
    i = gg.rows_in_plot[m];
    gg.color_prev[i] = gg.color_ids[i];
    gg.hidden_prev[i] = gg.hidden[i];
    gg.glyph_prev[i].size = gg.glyph_ids[i].size;
    gg.glyph_prev[i].type = gg.glyph_ids[i].type;
  }
}

void
brush_undo (splotd *sp) {
  gint m, i;
  for (m=0; m<gg.nrows_in_plot; m++) {
    i = gg.rows_in_plot[m];
    gg.color_ids[i] = gg.color_now[i] = gg.color_prev[i];
    gg.hidden[i] = gg.hidden_now[i] = gg.hidden_prev[i];
    gg.glyph_ids[i].type = gg.glyph_now[i].type = gg.glyph_prev[i].type;
    gg.glyph_ids[i].size = gg.glyph_now[i].size = gg.glyph_prev[i].size;
  }
  splot_redraw (sp, FULL);
}

void
reinit_transient_brushing (void)
{
/*
 * If a new variable is selected or a variable is transformed
 * during transient brushing, restore all points to the permanent
 * value, and then re-execute brush_once() to brush the points that
 * are now underneath the brush.  For now, don't make the
 * same change for persistent brushing.
*/
  gint i, m;

  for (m=0; m<gg.nrows_in_plot; m++)
  {
    i = gg.rows_in_plot[m];
    gg.color_now[i] = gg.color_ids[i] ;
    gg.glyph_now[i].type = gg.glyph_ids[i].type;
    gg.glyph_now[i].size = gg.glyph_ids[i].size;
    gg.hidden_now[i] = gg.hidden[i];
  }
  (void) brush_once (false);
}

void
brush_set_pos (gint x, gint y) {
  gint xdist = gg.app.brush_pos.x2 - gg.app.brush_pos.x1 ;
  gint ydist = gg.app.brush_pos.y2 - gg.app.brush_pos.y1 ;
  /*
   * (x2,y2) is the corner that's moving.
  */
 gg.app.brush_pos.x1 = x - xdist ;
 gg.app.brush_pos.x2 = x ;
 gg.app.brush_pos.y1 = y - ydist ;
 gg.app.brush_pos.y2 = y ;
}


void
brush_motion (icoords *mouse, gboolean button1_p, gboolean button2_p,
  cpaneld *cpanel)
{
  gboolean changed = false;
  splotd *sp = gg.current_splot;
  displayd *display = (displayd *) sp->displayptr;

  if (button1_p)
    brush_set_pos (mouse->x, mouse->y);

  else if (button2_p) {
    gg.app.brush_pos.x2 = mouse->x ;
    gg.app.brush_pos.y2 = mouse->y ;
  }


  if (cpanel->brush_on_p) {
    changed = brush_once (false);
    if (display->options.segments_undirected_show_p ||
        display->options.segments_directed_show_p ||
        display->options.segments_show_p ||
        gg.nrgroups > 0)      /*-- a full redraw is required --*/
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
  splotd *sp = gg.current_splot;
  gint pt;
  gint x1 = MIN (gg.app.brush_pos.x1, gg.app.brush_pos.x2);
  gint x2 = MAX (gg.app.brush_pos.x1, gg.app.brush_pos.x2);
  gint y1 = MIN (gg.app.brush_pos.y1, gg.app.brush_pos.y2);
  gint y2 = MAX (gg.app.brush_pos.y1, gg.app.brush_pos.y2);

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
    imin->x = MIN (gg.bin0.x, obin0->x);
    imin->y = MIN (gg.bin0.y, obin0->y);
    imax->x = MAX (gg.bin1.x, obin1->x);
    imax->y = MAX (gg.bin1.y, obin1->y);
  }
  else {
    imin->x = gg.bin0.x;
    imin->y = gg.bin0.y;
    imax->x = gg.bin1.x;
    imax->y = gg.bin1.y;
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
    gdk_draw_string (sp->pixmap1, style->font, gg.plot_GC,
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
  cpaneld *cpanel = &gg.current_display->cpanel;
  gboolean point_painting_p =
     (cpanel->br_scope == BR_POINTS || cpanel->br_scope == BR_PANDL);
  gboolean line_painting_p =
     (cpanel->br_scope == BR_LINES || cpanel->br_scope == BR_PANDL);

  gint x1 = MIN (gg.app.brush_pos.x1, gg.app.brush_pos.x2);
  gint x2 = MAX (gg.app.brush_pos.x1, gg.app.brush_pos.x2);
  gint y1 = MIN (gg.app.brush_pos.y1, gg.app.brush_pos.y2);
  gint y2 = MAX (gg.app.brush_pos.y1, gg.app.brush_pos.y2);

  if (!gg.mono_p) {
    if ((gg.default_color_table[gg.color_id].red != gg.bg_color.red) ||
        (gg.default_color_table[gg.color_id].blue != gg.bg_color.blue) ||
        (gg.default_color_table[gg.color_id].green != gg.bg_color.green))
    {
      gdk_gc_set_foreground (gg.plot_GC, &gg.default_color_table[gg.color_id]);
    } else {
      gdk_gc_set_foreground (gg.plot_GC, &gg.accent_color);
    }
  }

  if (point_painting_p)
  {
    gdk_draw_rectangle (sp->pixmap1, gg.plot_GC, false,
      x1, y1, (x2>x1)?(x2-x1):(x1-x2), (y2>y1)?(y2-y1):(y1-y2));
    /* Mark the corner to which the cursor will be attached */
    gdk_draw_rectangle (sp->pixmap1, gg.plot_GC, true,
      gg.app.brush_pos.x2-1, gg.app.brush_pos.y2-1, 2, 2);

    /*
     * highlight brush
    */
    if (cpanel->brush_on_p) {
      gdk_draw_rectangle (sp->pixmap1, gg.plot_GC, false,
        x1-1, y1-1, (x2>x1)?(x2-x1+2):(x1-x2+2), (y2>y1)?(y2-y1+2):(y1-y2+2)); 

      /* Mark the corner to which the cursor will be attached */
      gdk_draw_rectangle (sp->pixmap1, gg.plot_GC, true,
        gg.app.brush_pos.x2-2, gg.app.brush_pos.y2-2, 4, 4);
    }
  }

  if (line_painting_p) {
    gdk_draw_line (sp->pixmap1, gg.plot_GC,
      x1 + (x2 - x1)/2, y1, x1 + (x2 - x1)/2, y2 );
    gdk_draw_line (sp->pixmap1, gg.plot_GC,
      x1, y1 + (y2 - y1)/2, x2, y1 + (y2 - y1)/2 );

    if (cpanel->brush_on_p) {
      gdk_draw_line (sp->pixmap1, gg.plot_GC,
        x1 + (x2 - x1)/2 + 1, y1, x1 + (x2 - x1)/2 + 1, y2 );
      gdk_draw_line (sp->pixmap1, gg.plot_GC,
        x1, y1 + (y2 - y1)/2 + 1, x2, y1 + (y2 - y1)/2 + 1 );
    }
  }
}

/*----------------------------------------------------------------------*/
/*                      Glyph brushing                                  */
/*----------------------------------------------------------------------*/

static gboolean
update_glyph_arrays (gint i, gboolean changed) {
  cpaneld *cpanel = &gg.current_display->cpanel;
  gboolean doit = true;

/* setting the value of changed */
  if (!changed) {
    if (gg.under_new_brush[i]) {

      doit = (gg.glyph_now[i].size != gg.glyph_id.size);
      if (cpanel->br_target != BR_GSIZE)  /*-- ... if not ignoring type --*/
        doit = doit || (gg.glyph_now[i].type != gg.glyph_id.type);

    } else {

      doit = (gg.glyph_now[i].size != gg.glyph_ids[i].size);
      if (cpanel->br_target != BR_GSIZE)  /*-- ... if not ignoring type --*/
        doit = doit || (gg.glyph_now[i].type != gg.glyph_ids[i].type);
    }
  }
/* */

  if (doit) {
    if (gg.under_new_brush[i]) {
      switch (cpanel->br_mode) {

        case BR_PERSISTENT:
          gg.glyph_ids[i].size = gg.glyph_now[i].size = gg.glyph_id.size;
          if (cpanel->br_target != BR_GSIZE)  /*-- ... if not ignoring type --*/
            gg.glyph_ids[i].type = gg.glyph_now[i].type = gg.glyph_id.type;
          break;

        case BR_TRANSIENT:
          gg.glyph_now[i].size = gg.glyph_id.size;
          if (cpanel->br_target != BR_GSIZE)  /*-- ... if not ignoring type --*/
            gg.glyph_now[i].type = gg.glyph_id.type;
          break;
      }
    } else {
      gg.glyph_now[i].size = gg.glyph_ids[i].size;
      if (cpanel->br_target != BR_GSIZE)  /*-- ... if not ignoring type --*/
        gg.glyph_now[i].type = gg.glyph_ids[i].type;
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
  cpaneld *cpanel = &gg.current_display->cpanel;

  brush_boundaries_set (cpanel, &obin0, &obin1, &imin, &imax);

  for (ih=imin.x; ih<=imax.x; ih++) {
    for (iv=imin.y; iv<=imax.y; iv++) {
      for (m=0; m<gg.br_binarray[ih][iv].nels; m++) {
        /*
         * j is the row number; k is the index of rows_in_plot[]
        */
        j = gg.rows_in_plot[ k = gg.br_binarray[ih][iv].els[m] ] ;

        if (j < gg.nlinkable) {

          /* update the glyph arrays for every member of the row group */
          if (gg.nrgroups > 0) {
            gp = gg.rgroup_ids[k];
            for (n=0; n<gg.rgroups[gp].nels; n++) {
              p = gg.rgroups[gp].els[n];
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

  obin0.x = gg.bin0.x;
  obin0.y = gg.bin0.y;
  obin1.x = gg.bin1.x;
  obin1.y = gg.bin1.y;

  return (changed);
}

/*----------------------------------------------------------------------*/
/*                      Color brushing                                  */
/*----------------------------------------------------------------------*/

static gboolean
update_color_arrays (gint i, gboolean changed) {
  cpaneld *cpanel = &gg.current_display->cpanel;
  gboolean doit = true;

/* setting the value of doit */
  if (!changed) {
    if (gg.under_new_brush[i])
      doit = (gg.color_now[i] != gg.color_id);
    else
      doit = (gg.color_now[i] != gg.color_ids[i]);
  }
/* */

  /*
   * If doit is false, it's guaranteed that there will be no change.
  */
  if (doit) {
    if (gg.under_new_brush[i]) {
      switch (cpanel->br_mode) {
        case BR_PERSISTENT:
          gg.color_ids[i] = gg.color_now[i] = gg.color_id;
          break;
        case BR_TRANSIENT:
          gg.color_now[i] = gg.color_id;
          break;
      }
    } else gg.color_now[i] = gg.color_ids[i];
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
  cpaneld *cpanel = &gg.current_display->cpanel;

  brush_boundaries_set (cpanel, &obin0, &obin1, &imin, &imax);

  for (ih=imin.x; ih<=imax.x; ih++) {
    for (iv=imin.y; iv<=imax.y; iv++) {
      for (m=0; m<gg.br_binarray[ih][iv].nels; m++) {
        j = gg.rows_in_plot[ k = gg.br_binarray[ih][iv].els[m] ] ;
        /*
         * k   raw index, based on nrows
         * j   index based on nrows_in_plot
        */
        if (j < gg.nlinkable) {

          /* update the color arrays for every member of the row group */
          if (gg.nrgroups > 0) {
            gp = gg.rgroup_ids[k];
            for (n=0; n<gg.rgroups[gp].nels; n++) {
              p = gg.rgroups[gp].els[n];
              changed = update_color_arrays (p, changed);
            }
          /* */

          } else {  /* update the arrays for this point only */
            changed = update_color_arrays (j, changed);
          }
        }
      }
    }
    obin0.x = gg.bin0.x;
    obin0.y = gg.bin0.y;
    obin1.x = gg.bin1.x;
    obin1.y = gg.bin1.y;
  }

  return (changed);
}

/*----------------------------------------------------------------------*/
/*                      Hide brushing                                   */
/*----------------------------------------------------------------------*/

static gboolean
update_hidden_arrays (gint i, gboolean changed) {
  cpaneld *cpanel = &gg.current_display->cpanel;
  gboolean doit = true;

  /*
   * First find out if this will result in a change; this in
   * order to be able to return that information.
  */
  if (!changed) {
    if (gg.under_new_brush[i])
      doit = (gg.hidden_now[i] != true);
    else
      doit = (gg.hidden_now[i] != gg.hidden[i]);
  }
/* */

/*
 * If doit is false, it's guaranteed that there will be no change.
*/

  if (doit) {
    if (gg.under_new_brush[i]) {
      switch (cpanel->br_mode) {
        case BR_PERSISTENT:
          gg.hidden[i] = gg.hidden_now[i] = true;
          break;
        case BR_TRANSIENT:
          gg.hidden_now[i] = true;
          break;
      }
    } else gg.hidden_now[i] = gg.hidden[i];
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
  cpaneld *cpanel = &gg.current_display->cpanel;

  brush_boundaries_set (cpanel, &obin0, &obin1, &imin, &imax);

  for (ih=imin.x; ih<=imax.x; ih++) {
    for (iv=imin.y; iv<=imax.y; iv++) {
      for (m=0; m<gg.br_binarray[ih][iv].nels; m++) {
        j = gg.rows_in_plot[ k = gg.br_binarray[ih][iv].els[m] ] ;
        /*
         * k   raw index, based on nrows
         * j   index based on nrows_in_plot
        */
        if (j < gg.nlinkable) {

          if (gg.nrgroups > 0) {
            /*-- update the hidden arrays for every member of the row group --*/
            gp = gg.rgroup_ids[k];
            for (n=0; n<gg.rgroups[gp].nels; n++) {
              p = gg.rgroups[gp].els[n];
              changed = update_hidden_arrays (p, changed);
            }
          /* */

          } else {  /* update the arrays for this point only */
            changed = update_hidden_arrays (j, changed);
          }
        }
      }
    }
    obin0.x = gg.bin0.x;
    obin0.y = gg.bin0.y;
    obin1.x = gg.bin1.x;
    obin1.y = gg.bin1.y;
  }

  return (changed);
}

gboolean
active_paint_points (void)
{
  gint ih, iv, j, pt, k, gp;
  gboolean changed;
  cpaneld *cpanel = &gg.current_display->cpanel;
/*
 * Set under_new_brush[j] to 1 if point j is inside the rectangular brush.
*/

  /* Zero out under_new_brush[] before looping */
  npts_under_brush = 0;
  for (j=0; j<gg.nrows_in_plot; j++)
    gg.under_new_brush[gg.rows_in_plot[j]] = 0;
 
  /*
   * br_binarray[][] only represents the
   * rows in rows_in_plot[] so there's no need to test for that.
  */
  for (ih=gg.bin0.x; ih<=gg.bin1.x; ih++) {
    for (iv=gg.bin0.y; iv<=gg.bin1.y; iv++) {
      for (j=0; j<gg.br_binarray[ih][iv].nels; j++) {
        pt = gg.rows_in_plot[gg.br_binarray[ih][iv].els[j]];

        if (pt < gg.nlinkable) {
/*          if (!gg.hidden[pt] && under_brush (pt)) {*/
          if (under_brush (pt)) {

            npts_under_brush++ ;
            gg.under_new_brush[pt] = 1;

            /* brush other members of this row group */
            if (gg.nrgroups > 0) {
              gp = gg.rgroup_ids[pt];
              if (gp < gg.nrgroups) {  /* exclude points without an rgroup */
                for (k=0; k<gg.rgroups[gp].nels; k++) {
/*                  if (!gg.hidden[ gg.rgroups[gp].els[k] ])*/
                    gg.under_new_brush[gg.rgroups[gp].els[k]] = 1;
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
