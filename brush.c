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

#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/* */
static gboolean active_paint_points (datad *d, ggobid *gg);
static gboolean active_paint_edges (displayd *display, ggobid *gg);
static gboolean build_symbol_vectors (cpaneld *, datad *, ggobid *);
/* */

/*----------------------------------------------------------------------*/
/*             Glyph utility: called in read_data                       */
/*----------------------------------------------------------------------*/

void
find_glyph_type_and_size (gint gid, glyphd *glyph)
{
/*  gid ranges from 0:42, type from 0 to 6, size from 0 to 7 */
  if (gid == 0) {
    glyph->type = glyph->size = 0;  /*-- single-pixel point --*/
  } else {
    glyph->type = ( (gid-1) / (gint) (NGLYPHSIZES) ) + 1 ;
    glyph->size = ( (gid-1) % (gint) (NGLYPHSIZES) ) ;
  }
}

/*----------------------------------------------------------------------*/

/*-- called by brush_motion, brush_mode_cb, and in the api --*/
gboolean
brush_once (gboolean force, splotd *sp, ggobid *gg)
{
/*
 * Determine which bins the brush is currently sitting in.
 * bin0 is the bin which contains of the upper left corner of the
 * brush; bin1 is the one containing of the lower right corner.
*/
  displayd *display = sp->displayptr;
  datad *d = display->d;
  datad *e = display->e;

  brush_coords *brush_pos = &sp->brush_pos;

  icoords *bin0 = &d->brush.bin0;
  icoords *bin1 = &d->brush.bin1;

  gboolean changed = false;
  cpaneld *cpanel = &display->cpanel;

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

/*
 * Now paint.
*/
  if (cpanel->br_point_targets) {
    changed = active_paint_points (d, gg);
  }

  if (cpanel->br_edge_targets) {
    if (e != NULL)
      changed = active_paint_edges (display, gg);
  }

  return (changed);
}

void
brush_prev_vectors_update (datad *d, ggobid *gg) {
  gint m, i;

  if (d->color_prev.nels < d->nrows) {
    vectors_realloc (&d->color_prev, d->nrows);
    vectorb_realloc (&d->hidden_prev, d->nrows);
    vectorg_realloc (&d->glyph_prev, d->nrows);
  }

  for (m=0; m<d->nrows_in_plot; m++) {
    i = d->rows_in_plot[m];
    d->color_prev.els[i] = d->color.els[i];
    d->hidden_prev.els[i] = d->hidden.els[i];
    d->glyph_prev.els[i].size = d->glyph.els[i].size;
    d->glyph_prev.els[i].type = d->glyph.els[i].type;
  }
}

void
brush_undo (splotd *sp, datad *d, ggobid *gg) {
  gint m, i;
  if(!d)
    return;

  for (m=0; m<d->nrows_in_plot; m++) {
    i = d->rows_in_plot[m];
    d->color.els[i] = d->color_now.els[i] = d->color_prev.els[i];
    d->hidden.els[i] = d->hidden_now.els[i] = d->hidden_prev.els[i];
    d->glyph.els[i].type = d->glyph_now.els[i].type = d->glyph_prev.els[i].type;
    d->glyph.els[i].size = d->glyph_now.els[i].size = d->glyph_prev.els[i].size;
  }
}

void
reinit_transient_brushing (displayd *dsp, ggobid *gg)
{
/*
 * If a new variable is selected or a variable is transformed
 * during transient brushing, we may want to restore all points to
 * the permanent value.  After calling this routine, re-execute
 * brush_once() to brush the points that are now underneath the brush. 
 * For now, don't make the same change for persistent brushing.
*/
  gint i, m, k;
  datad *d = dsp->d;
  datad *e = dsp->e;
  cpaneld *cpanel = &dsp->cpanel;
  gboolean point_painting_p = (cpanel->br_point_targets != br_off);
  gboolean edge_painting_p = (cpanel->br_edge_targets != br_off);

  if (point_painting_p) {
    for (m=0; m<d->nrows_in_plot; m++) {
      i = d->rows_in_plot[m];
      d->color_now.els[i] = d->color.els[i] ;
      d->glyph_now.els[i].type = d->glyph.els[i].type;
      d->glyph_now.els[i].size = d->glyph.els[i].size;
      d->hidden_now.els[i] = d->hidden.els[i];
    }
  }
  if (edge_painting_p && e != NULL) {
    for (k=0; k<e->edge.n; k++) {
      e->color_now.els[k] = e->color.els[k];
      e->glyph_now.els[k].type = e->glyph.els[k].type;
      e->glyph_now.els[k].size = e->glyph.els[k].size;
      e->hidden_now.els[k] = e->hidden.els[k];
    }
  }
}

void
brush_set_pos (gint x, gint y, splotd *sp) {
  brush_coords *brush = &sp->brush_pos;
  brush_coords *obrush = &sp->brush_pos_o;
  gint xdist = brush->x2 - brush->x1 ;
  gint ydist = brush->y2 - brush->y1 ;

  /*-- copy the current coordinates to the backup brush structure --*/
  obrush->x1 = brush->x1;
  obrush->y1 = brush->y1;
  obrush->x2 = brush->x2;
  obrush->y2 = brush->y2;

  /*
   * (x2,y2) is the corner that's moving.
  */
  brush->x1 = x - xdist ;
  brush->x2 = x ;
  brush->y1 = y - ydist ;
  brush->y2 = y ;
}

static gboolean
binning_permitted (displayd *display, ggobid *gg)
{
  datad *e = display->e;
  gboolean permitted = true;

  if (gg->linkby_cv)
    return(false);

  if(GTK_IS_GGOBI_EXTENDED_DISPLAY(display)) {
     /* If there is a function to determine this, call it. Otherwise just 
        get the value of binning_ok in the class. */
    GtkGGobiExtendedDisplayClass *klass;
    klass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(display)->klass);
    if(klass->binningPermitted)
       return(klass->binningPermitted(display));
    return(klass->binning_ok);
  }


  /*-- if we're drawing edges --*/
  if (e != NULL && e->edge.n > 0) {
    if (display->options.edges_undirected_show_p ||
        display->options.edges_directed_show_p ||
        display->options.whiskers_show_p)
    {
      permitted = false;
    }
  }

  return permitted;
}

void
brush_once_and_redraw (gboolean binningp, splotd *sp, displayd *display,
  ggobid *gg) 
{
  cpaneld *cpanel = &display->cpanel;
  gboolean changed = false;

  if (cpanel->brush_on_p) {
    changed = brush_once (!binningp, sp, gg);

    if (binningp && binning_permitted (display, gg)) {
      if (changed) {
        splot_redraw (sp, BINNED, gg);
        if (gg->brush.updateAlways_p) {
          displays_plot (sp, FULL, gg);
        }

      } else {  /*-- just redraw the brush --*/
        splot_redraw (sp, QUICK, gg);  
      }

    } else {  /* no binning */
      splot_redraw (sp, FULL, gg);  
      if (gg->brush.updateAlways_p)
        displays_plot (sp, FULL, gg);  
    }

  } else {  /*-- we're not brushing, and we just need to redraw the brush --*/
    splot_redraw (sp, QUICK, gg);
  }
}

void
brush_motion (icoords *mouse, gboolean button1_p, gboolean button2_p,
  cpaneld *cpanel, splotd *sp, ggobid *gg)
{
  displayd *display = sp->displayptr;
  brush_coords *brush_pos = &sp->brush_pos;

  if (button1_p) {
/*
    if (display->displaytype == parcoords) {
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
    }
*/

    brush_set_pos (mouse->x, mouse->y, sp);
  }

  else if (button2_p) {
    brush_pos->x2 = mouse->x ;
    brush_pos->y2 = mouse->y ;
  }

  brush_once_and_redraw (true, sp, display, gg);  /* binning permitted */
}


static gboolean
under_brush (gint k, splotd *sp)
/*
 * Determine whether point k is under the brush.
*/
{
  brush_coords *brush_pos = &sp->brush_pos;
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
brush_draw_label (splotd *sp, GdkDrawable *drawable, datad *d, ggobid *gg)
{
  gint lbearing, rbearing, width, ascent, descent;
  GtkStyle *style = gtk_widget_get_style (sp->da);

  if (d->npts_under_brush > 0) {
    gchar *str = g_strdup_printf ("%d", d->npts_under_brush);
    gdk_text_extents (
#if GTK_MAJOR_VERSION == 2
      gtk_style_get_font (style),
#else
      style->font,
#endif
      str, strlen (str),
      &lbearing, &rbearing, &width, &ascent, &descent);
    gdk_draw_string (drawable,
#if GTK_MAJOR_VERSION == 2
      gtk_style_get_font (style),
#else
      style->font,
#endif
      gg->plot_GC,
      sp->max.x - width - 5,
      ascent + descent + 5,
      str);
    g_free (str);
  }
}

void
brush_draw_brush (splotd *sp, GdkDrawable *drawable, datad *d, ggobid *gg) {
/*
 * Use brush_pos to draw the brush.
*/
  displayd *display = sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  gboolean point_painting_p = (cpanel->br_point_targets != br_off);
  gboolean edge_painting_p = (cpanel->br_edge_targets != br_off);
  colorschemed *scheme = gg->activeColorScheme;

  brush_coords *brush_pos = &sp->brush_pos;
  gint x1 = MIN (brush_pos->x1, brush_pos->x2);
  gint x2 = MAX (brush_pos->x1, brush_pos->x2);
  gint y1 = MIN (brush_pos->y1, brush_pos->y2);
  gint y2 = MAX (brush_pos->y1, brush_pos->y2);

  if (!gg->mono_p) {
    if ((scheme->rgb[gg->color_id].red != scheme->rgb_bg.red) ||
        (scheme->rgb[gg->color_id].blue != scheme->rgb_bg.blue) ||
        (scheme->rgb[gg->color_id].green != scheme->rgb_bg.green))
    {
      gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb[gg->color_id]);
    } else {
      gdk_gc_set_foreground (gg->plot_GC,
                             &scheme->rgb_accent);
    }
  }

  if (point_painting_p)
  {
    gdk_draw_rectangle (drawable, gg->plot_GC, false,
      x1, y1, (x2>x1)?(x2-x1):(x1-x2), (y2>y1)?(y2-y1):(y1-y2));
    /* Mark the corner to which the cursor will be attached */
    gdk_draw_rectangle (drawable, gg->plot_GC, true,
      brush_pos->x2-1, brush_pos->y2-1, 2, 2);

    /*
     * highlight brush
    */
    if (cpanel->brush_on_p) {
      gdk_draw_rectangle (drawable, gg->plot_GC, false,
        x1-1, y1-1, (x2>x1)?(x2-x1+2):(x1-x2+2), (y2>y1)?(y2-y1+2):(y1-y2+2)); 

      /* Mark the corner to which the cursor will be attached */
      gdk_draw_rectangle (drawable, gg->plot_GC, true,
        brush_pos->x2-2, brush_pos->y2-2, 4, 4);
    }
  }

  if (edge_painting_p) {
    gdk_draw_line (drawable, gg->plot_GC,
      x1 + (x2 - x1)/2, y1, x1 + (x2 - x1)/2, y2 );
    gdk_draw_line (drawable, gg->plot_GC,
      x1, y1 + (y2 - y1)/2, x2, y1 + (y2 - y1)/2 );

    if (cpanel->brush_on_p) {
      gdk_draw_line (drawable, gg->plot_GC,
        x1 + (x2 - x1)/2 + 1, y1, x1 + (x2 - x1)/2 + 1, y2 );
      gdk_draw_line (drawable, gg->plot_GC,
        x1, y1 + (y2 - y1)/2 + 1, x2, y1 + (y2 - y1)/2 + 1 );
    }
  }
}

/*----------------------------------------------------------------------*/
/*                      Glyph brushing                                  */
/*----------------------------------------------------------------------*/

/*
 * This currently isn't using the same arguments as its
 * _color_ and _hidden_ cousins because we can't brush on line
 * type yet.
*/
gboolean
update_glyph_vectors (gint i, gboolean changed, gboolean *hit_by_brush,
  datad *d, ggobid *gg)
{
  cpaneld *cpanel = &gg->current_display->cpanel;
  gboolean doit = true;

/* setting the value of changed */
  if (!changed) {
    if (hit_by_brush[i]) {

      doit = (d->glyph_now.els[i].size != gg->glyph_id.size);

      /*-- ... and if not ignoring type --*/
      if (!doit && cpanel->br_point_targets != br_gsize) 
        doit = doit || (d->glyph_now.els[i].type != gg->glyph_id.type);

    } else {

      doit = (d->glyph_now.els[i].size != d->glyph.els[i].size);
      /*-- ... if not ignoring type --*/
      if (cpanel->br_point_targets != br_gsize) 
        doit = doit || (d->glyph_now.els[i].type != d->glyph.els[i].type);
    }
  }
/* */

  if (doit) {
    if (hit_by_brush[i]) {
      switch (cpanel->br_mode) {

        case BR_PERSISTENT:
          d->glyph.els[i].size = d->glyph_now.els[i].size = gg->glyph_id.size;
          /*-- ... if not ignoring type --*/
          if (cpanel->br_point_targets != br_gsize) 
            d->glyph.els[i].type = d->glyph_now.els[i].type = gg->glyph_id.type;
        break;

        case BR_TRANSIENT:
          d->glyph_now.els[i].size = gg->glyph_id.size;
          /*-- ... if not ignoring type --*/
          if (cpanel->br_point_targets != br_gsize) 
            d->glyph_now.els[i].type = gg->glyph_id.type;
        break;
      }
    } else {
      d->glyph_now.els[i].size = d->glyph.els[i].size;
      /*-- ... if not ignoring type --*/
      if (cpanel->br_point_targets != br_gsize) 
        d->glyph_now.els[i].type = d->glyph.els[i].type;
    }
  }

  return (doit);
}

/*----------------------------------------------------------------------*/
/*                      Color brushing                                  */
/*----------------------------------------------------------------------*/

gboolean
update_color_vectors (gint i, gboolean changed, gboolean *hit_by_brush,
  datad *d, ggobid *gg)
{
  cpaneld *cpanel = &gg->current_display->cpanel;
  gboolean doit = true;

/* setting the value of doit */
  if (!changed) {
    if (hit_by_brush[i])
      /*-- if persistent, compare against color instead of color_now --*/
      doit = (cpanel->br_mode == BR_TRANSIENT) ?
               (d->color_now.els[i] != gg->color_id) :
               (d->color.els[i] != gg->color_id);
    else
      doit = (d->color_now.els[i] != d->color.els[i]);  /*-- ?? --*/
  }
/* */

  /*
   * If doit is false, it's guaranteed that there will be no change.
  */
  if (doit) {
    if (hit_by_brush[i]) {
      switch (cpanel->br_mode) {
        case BR_PERSISTENT:
          d->color.els[i] = d->color_now.els[i] = gg->color_id;
        break;
        case BR_TRANSIENT:
          d->color_now.els[i] = gg->color_id;
        break;
      }
    } else d->color_now.els[i] = d->color.els[i];
  }

  return (doit);
}

/*----------------------------------------------------------------------*/
/*                      Hide brushing                                   */
/*----------------------------------------------------------------------*/

static gboolean
update_hidden_vectors (gint i, gboolean changed, gboolean *hit_by_brush,
  datad *d, ggobid *gg)
{
  cpaneld *cpanel = &gg->current_display->cpanel;
  gboolean doit = true;

  /*
   * First find out if this will result in a change; this in
   * order to be able to return that information.
  */
  if (!changed) {
    if (hit_by_brush[i])
      doit = (d->hidden_now.els[i] != true);
    else
      doit = (d->hidden_now.els[i] != d->hidden.els[i]);
  }
  /* */

/*
 * If doit is false, it's guaranteed that there will be no change.
*/

  if (doit) {
    if (hit_by_brush[i]) {
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

/*-- the opposite of hide --*/
static gboolean
update_selected_vectors (gint i, gboolean changed, gboolean *hit_by_brush,
  datad *d, ggobid *gg)
{
  cpaneld *cpanel = &gg->current_display->cpanel;
  gboolean doit = true;

  /*
   * First find out if this will result in a change; this in
   * order to be able to return that information.
  */
  if (!changed) {
    if (hit_by_brush[i])
      doit = (d->hidden_now.els[i] == true);
    else
      doit = (d->hidden_now.els[i] == d->hidden.els[i]);
  }
  /* */

/*
 * If doit is false, it's guaranteed that there will be no change.
*/

  if (doit) {
    if (hit_by_brush[i]) {
      switch (cpanel->br_mode) {
        case BR_PERSISTENT:
          d->hidden.els[i] = d->hidden_now.els[i] = false;
        break;
        case BR_TRANSIENT:
          d->hidden_now.els[i] = false;
        break;
      }
    /*} else d->hidden_now.els[i] = d->hidden.els[i];*/
    } else d->hidden_now.els[i] = true;
  }

  return (doit);
}


/*----------------------------------------------------------------------*/
/*         Handle all symbols in one loop through a bin                 */
/*----------------------------------------------------------------------*/

static gboolean
build_symbol_vectors (cpaneld *cpanel, datad *d, ggobid *gg)
{
  gint ih, iv, m, j, k;
  /*-- these look suspicious -- dfs --*/
  static icoords obin0 = {BRUSH_NBINS/2, BRUSH_NBINS/2};
  static icoords obin1 = {BRUSH_NBINS/2, BRUSH_NBINS/2};
  icoords imin, imax;
  gboolean changed = false;
  gint nd = g_slist_length (gg->d);

  /* These two are needed for the extended display.
     Should the method be on the extended splot or the display (as it is now).
     The choice is somewhat arbitrary since the method for the barchart doesn't
     seem to do much with the display or plot. It is just used to check if we
     have a barchart. */
  splotd *sp = gg->current_splot;
  displayd *display = (displayd *) sp->displayptr;

  if (GTK_IS_GGOBI_EXTENDED_DISPLAY(display)) {
    gboolean (*f)(datad *, ggobid *);
    f = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(display)->klass)->build_symbol_vectors; 
    if(f)
      changed = f(d, gg);
  }

  brush_boundaries_set (cpanel, &obin0, &obin1, &imin, &imax, d, gg);

  for (ih=imin.x; ih<=imax.x; ih++) {
    for (iv=imin.y; iv<=imax.y; iv++) {
      for (m=0; m<d->brush.binarray[ih][iv].nels; m++) {
        /*
         * j is the row number; k is the index of rows_in_plot[]
        */
        j = d->rows_in_plot[ k = d->brush.binarray[ih][iv].els[m] ] ;

        switch (cpanel->br_point_targets) {
          case br_candg:  /*-- color and glyph --*/
            changed = update_color_vectors (j, changed,
              d->pts_under_brush.els, d, gg);
            changed = update_glyph_vectors (j, changed,
              d->pts_under_brush.els, d, gg);
          break;
          case br_color:
            changed = update_color_vectors (j, changed,
              d->pts_under_brush.els, d, gg);
          break;
          case br_glyph:  /*-- glyph type and size --*/
          case br_gsize:  /*-- glyph size only --*/
            changed = update_glyph_vectors (j, changed,
              d->pts_under_brush.els, d, gg);
          break;
          case br_hide:  /*-- hidden --*/
            changed = update_hidden_vectors (j, changed,
              d->pts_under_brush.els, d, gg);
          break;
          case br_select:
            changed = update_selected_vectors (j, changed,
              d->pts_under_brush.els, d, gg);
          break;
          case br_off:
            ;
          break;
        }

        /*-- link by id --*/
        if (!gg->linkby_cv && nd > 1) symbol_link_by_id (j, d, gg);
        /*-- --*/
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
/*                   active_paint_points                                */
/*----------------------------------------------------------------------*/

/*
 * Set pts_under_brush[j] to 1 if point j is inside the rectangular brush.
*/
gboolean
active_paint_points (datad *d, ggobid *gg)
{
  gint ih, iv, j, pt;
  gboolean changed;
  cpaneld *cpanel = &gg->current_display->cpanel;
  splotd *sp = gg->current_splot;
  displayd *display = (displayd *) sp->displayptr;
  gint (*f)(splotd *sp, datad *) = NULL;

  if (GTK_IS_GGOBI_EXTENDED_SPLOT(sp)) {
    f = GTK_GGOBI_EXTENDED_SPLOT_CLASS(GTK_OBJECT(sp)->klass)->active_paint_points;
    if(f)
       d->npts_under_brush = f(sp, d);
  }

  if (!f) {
    /* Zero out pts_under_brush[] before looping */
    d->npts_under_brush = 0;
    for (j=0; j<d->nrows_in_plot; j++)
      d->pts_under_brush.els[d->rows_in_plot[j]] = 0;
 
    /*
     * d->brush.binarray[][] only represents the
     * cases in rows_in_plot[] so there's no need to test for that.
    */

    for (ih=d->brush.bin0.x; ih<=d->brush.bin1.x; ih++) {
      for (iv=d->brush.bin0.y; iv<=d->brush.bin1.y; iv++) {
        for (j=0; j<d->brush.binarray[ih][iv].nels; j++) {
          pt = d->rows_in_plot[d->brush.binarray[ih][iv].els[j]];

          /*
           * if a case is hidden, or it's missing and we aren't
           * displaying missings, don't count it as being under the brush.
           * Caution:
           * If we're doing hide or un-hide brushing, we can't ignore hidden
           * cases; otherwise it's ok (I think).
          */
          if (splot_plot_case (pt,
            cpanel->br_point_targets != br_hide &&
            cpanel->br_point_targets != br_select,
            d, sp, display, gg))
          {
            if (under_brush (pt, sp)) {
              d->npts_under_brush++ ;
              d->pts_under_brush.els[pt] = 1;
            }
          }
        }
      }
    }
  }

  changed = false;

  if (cpanel->brush_on_p) {
    if (gg->linkby_cv) {
      /*-- link by categorical variable --*/
      changed = build_symbol_vectors_by_var (cpanel, d, gg);
    } else {
      /*-- link by id --*/
      changed = build_symbol_vectors (cpanel, d, gg); 
    }
  }

  return (changed);
}

/*----------------------------------------------------------------------*/
/*                      Edge brushing                                   */
/*----------------------------------------------------------------------*/


static gboolean
xed_by_brush (gint k, displayd *display, ggobid *gg)
/*
 * Determine whether edge k intersects the brush
*/
{
  datad *d = display->d;
  datad *e = display->e;
  splotd *sp = gg->current_splot;
  gboolean intersect;
  glong x1 = sp->brush_pos.x1;
  glong y1 = sp->brush_pos.y1;
  glong x2 = sp->brush_pos.x2;
  glong y2 = sp->brush_pos.y2;

  endpointsd *endpoints = e->edge.endpoints;
  glong a = d->rowid.idv.els[endpoints[k].a];
  glong b = d->rowid.idv.els[endpoints[k].b];

  /*-- test for intersection with the vertical edge --*/
  intersect = lines_intersect (x1 + (x2 - x1)/2, y1, x1 + (x2 - x1)/2, y2,
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
    intersect = lines_intersect (x1, y1 + (y2 - y1)/2, x2, y1 + (y2 - y1)/2,
      sp->screen[a].x, sp->screen[a].y,
      sp->screen[b].x, sp->screen[b].y);
/*
    intersect = isCrossed (x1, y1 + (y2 - y1)/2, x2, y1 + (y2 - y1)/2,
      (gdouble) sp->screen[a].x, (gdouble) sp->screen[a].y,
      (gdouble) sp->screen[b].x, (gdouble) sp->screen[b].y);
*/
  }

  return (intersect == 1);
}

/*-- link by id? --*/
static gboolean
build_edge_symbol_vectors (cpaneld *cpanel, datad *e, ggobid *gg)
{
  gint i;
  gboolean changed = false;
  gint nd = g_slist_length (gg->d);

/*
 * I'm not doing any checking here to verify that the edges
 * are displayed.
*/
  for (i=0; i<e->edge.n; i++) {

    switch (cpanel->br_edge_targets) {
      case br_candg:  /*-- color and glyph --*/
        changed = update_color_vectors (i, changed,
          e->edge.xed_by_brush.els, e, gg);
        changed = update_glyph_vectors (i, changed,
          e->edge.xed_by_brush.els, e, gg);
      break;
      case br_color:  /*-- color --*/
        changed = update_color_vectors (i, changed,
          e->edge.xed_by_brush.els, e, gg);
      break;
      case br_glyph:  /*-- line width and line type --*/
      case br_gsize:  /*-- line width only --*/
        changed = update_glyph_vectors (i, changed,
          e->edge.xed_by_brush.els, e, gg);
      break;
      case br_hide:  /*-- hidden --*/
        changed = update_hidden_vectors (i, changed,
          e->edge.xed_by_brush.els, e, gg);
      break;
      case br_select:
        changed = update_selected_vectors (i, changed,
          e->edge.xed_by_brush.els, e, gg);
      break;
      case br_off:
        ;
      break;
    }

    /*-- link by id --*/
    if (!gg->linkby_cv && nd > 1) symbol_link_by_id (i, e, gg);
    /*-- --*/
  }

  return (changed);
}

static gboolean
active_paint_edges (displayd *display, ggobid *gg)
{
  datad *e = display->e;
  gint k;
  gboolean changed;
  cpaneld *cpanel = &gg->current_display->cpanel;

  /* Zero out xed_by_brush[] before looping */
  e->edge.nxed_by_brush = 0;
  for (k=0; k<e->edge.n; k++)
    e->edge.xed_by_brush.els[k] = false;
 
  for (k=0; k<e->edge.n; k++) {
    if (xed_by_brush (k, display, gg)) {
      e->edge.nxed_by_brush++ ;
      e->edge.xed_by_brush.els[k] = true;
    }
  }

  changed = false;
  if (cpanel->brush_on_p) {
    if (gg->linkby_cv) {
      /*-- link by categorical variable --*/
      /*changed = build_symbol_vectors_by_var (cpanel, e, gg);*/
    } else {
      /*-- link by id  --*/
      changed = build_edge_symbol_vectors (cpanel, e, gg);
    }
  }

  return (changed);
}

