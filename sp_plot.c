/* sp_plot.c */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

void segments_draw (splotd *);
#ifdef _WIN32
extern void win32_draw_to_pixmap_binned (icoords *, icoords *, gint, splotd *);
extern void win32_draw_to_pixmap_unbinned (gint, splotd *);
#endif

/*
 * The corners of the bin to be copied from pixmap0 to pixmap1.
 * They're defined in splot_draw_to_pixmap0_binned and used in
 * splot_pixmap0_to_pixmap1 when binned == true.
*/
static icoords bin0, bin1;
static icoords loc0, loc1;

/* colors_used now contains integers, 0:ncolors-1 */
static void
splot_point_colors_used_get (splotd *sp, gint *ncolors_used,
  gushort *colors_used, gboolean binned) 
{
  gboolean new_color;
  gint i, k, m;
  displayd *display = (displayd *) sp->displayptr;

  *ncolors_used = 0;
          
  /*
   * Loop once through xg.color_now[], collecting the colors currently
   * in use into the colors_used[] vector.
  */
  if (display->points_show_p) {

    if (!binned) {
      for (i=0; i<xg.nrows_in_plot; i++) {
        m = xg.rows_in_plot[i];
        new_color = true;
        for (k=0; k<*ncolors_used; k++) {
          if (colors_used[k] == xg.color_now[m]) {
            new_color = false;
            break;
          }
        }
        if (new_color) {
          colors_used[*ncolors_used] = xg.color_now[m];
          (*ncolors_used)++;
        }
      }
    } else {  /* used by plot_bins */
      gint ih, iv, j;
      /*
       * This has already been called in draw_to_pixmap0_binned, and
       * it shouldn't be called twice, because it is keeping track of
       * previous values.
      */
      /*get_extended_brush_corners (&bin0, &bin1);*/

      for (ih=bin0.x; ih<=bin1.x; ih++) {
        for (iv=bin0.y; iv<=bin1.y; iv++) {
          for (m=0; m<xg.br_binarray[ih][iv].nels; m++) {
            j = xg.rows_in_plot[xg.br_binarray[ih][iv].els[m]];

            new_color = true;
            for (k=0; k<*ncolors_used; k++) {
              if (colors_used[k] == xg.color_now[j]) {
                new_color = false;
                break;
              }
            }
            if (new_color) {
              colors_used[*ncolors_used] = xg.color_now[j];
              (*ncolors_used)++;
            }
          }
        }
      }
    }
    /*
     * Make sure that the current brushing color is
     * last in the list, so that it is drawn on top of
     * the pile of points.
    */
    for (k=0; k<(*ncolors_used-1); k++) {
      if (colors_used[k] == xg.color_id) {
        colors_used[k] = colors_used[*ncolors_used-1];
        colors_used[*ncolors_used-1] = xg.color_id;
        break;
      }
    }

    /* insurance -- eg if using mono drawing on a color screen */
    if (*ncolors_used == 0) {
      *ncolors_used = 1;
      colors_used[0] = xg.color_now[0];
    }
  }
}

void
splot_draw_to_pixmap0_unbinned (splotd *sp)
{
  gint k;
#ifndef _WIN32
  gint i, m, n;
#endif
  gushort current_color;
  static gint npoint_colors_used = 0;
  static gushort point_colors_used[NCOLORS+2];
  GtkWidget *da = sp->da;
  displayd *display = (displayd *) sp->displayptr;
  gboolean draw_case;

  if (plot_GC == NULL)
    init_plot_GC (sp->pixmap0);

  /* clear the pixmap */
  gdk_gc_set_foreground (plot_GC, &xg.bg_color);
  gdk_draw_rectangle (sp->pixmap0, plot_GC,
                      TRUE, 0, 0,
                      da->allocation.width,
                      da->allocation.height);

  if (!mono_p) {
    splot_point_colors_used_get (sp, &npoint_colors_used,
      point_colors_used, false);

    /*
     * Now loop through point_colors_used[], plotting the points of each
     * color.  This avoids the need to reset the foreground so often.
     * On the other hand, it requires more looping.
    */
    for (k=0; k<npoint_colors_used; k++) {
      current_color = point_colors_used[k];
      gdk_gc_set_foreground (plot_GC, &xg.default_color_table[current_color]);

#ifdef _WIN32
      win32_draw_to_pixmap_unbinned (current_color, sp);
#else
      for (i=0; i<xg.nrows_in_plot; i++) {
        m = xg.rows_in_plot[i];
        draw_case = true;

        /*-- determine whether case m should be plotted --*/
        if (xg.erased_now[m])
          draw_case = false;
        /*-- can prevent drawing of missings for parcoords or scatmat plots --*/
        else if (!display->missings_show_p && xg.nmissing > 0) {
          switch (display->displaytype) {
            case parcoords:
              if (xg.missing.data[m][sp->p1dvar])
                draw_case = false;
              break;
            case scatmat:
              if (sp->p1dvar != -1) {
                if (xg.missing.data[m][sp->p1dvar])
                  draw_case = false;
              } else {
                if (xg.missing.data[m][sp->xyvars.x] ||
                    xg.missing.data[m][sp->xyvars.y])
                {
                  draw_case = false;
                }
              }
              break;
            case scatterplot:
              break;
            }
        }

        if (draw_case && xg.color_now[m] == current_color) {
          if (display->points_show_p)
            draw_glyph (sp->pixmap0, &xg.glyph_now[m], sp->screen, m);

          /*-- parallel coordinate plot whiskers --*/
          if (display->displaytype == parcoords) {
            if (display->segments_show_p) {
              n = 2*m;
              gdk_draw_line (sp->pixmap0, plot_GC,
                sp->whiskers[n].x1, sp->whiskers[n].y1,
                sp->whiskers[n].x2, sp->whiskers[n].y2);
              n++;
              gdk_draw_line (sp->pixmap0, plot_GC,
                sp->whiskers[n].x1, sp->whiskers[n].y1,
                sp->whiskers[n].x2, sp->whiskers[n].y2);
            }
          }
        }
      }
#endif
    }  /* deal with mono later */
  }

  return;
}

void
splot_draw_to_pixmap0_binned (splotd *sp)
{
  icoords loc_clear0, loc_clear1;
#ifndef _WIN32
  gint ih, iv;
  gint i, m, n;
#endif
  gint k;
  displayd *display = (displayd *) sp->displayptr;

  gushort current_color;
  static gint npoint_colors_used = 0;
  static gushort point_colors_used[NCOLORS+2];

  if (plot_GC == NULL)
    init_plot_GC (sp->pixmap0);

/*
 * Instead of clearing and redrawing the entire pixmap0, only
 * clear what's necessary.
*/

  get_extended_brush_corners (&bin0, &bin1);

/*
 * Determine locations of bin corners: upper left edge of loc0;
 * lower right edge of loc1.
*/
  loc0.x = (gint) ((gfloat) bin0.x / (gfloat) xg.br_nbins * (sp->max.x+1.0));
  loc0.y = (gint) ((gfloat) bin0.y / (gfloat) xg.br_nbins * (sp->max.y+1.0));
  loc1.x = (gint)
    ((gfloat) (bin1.x+1) / (gfloat) xg.br_nbins * (sp->max.x+1.0));
  loc1.y = (gint)
    ((gfloat) (bin1.y+1) / (gfloat) xg.br_nbins * (sp->max.y+1.0));

/*
 * Clear an area a few pixels inside that region.  Watch out
 * for border effects.
*/
  loc_clear0.x = (bin0.x == 0) ? 0 : loc0.x + BRUSH_MARGIN;
  loc_clear0.y = (bin0.y == 0) ? 0 : loc0.y + BRUSH_MARGIN;
  loc_clear1.x = (bin1.x == xg.br_nbins-1) ? sp->max.x : loc1.x - BRUSH_MARGIN;
  loc_clear1.y = (bin1.y == xg.br_nbins-1) ? sp->max.y : loc1.y - BRUSH_MARGIN;


  gdk_gc_set_foreground (plot_GC, &xg.bg_color);
  gdk_draw_rectangle (sp->pixmap0, plot_GC,
                      true,  /* fill */
                      loc_clear0.x, loc_clear0.y,
                      1 + loc_clear1.x - loc_clear0.x ,
                      1 + loc_clear1.y - loc_clear0.y);

  if (display->points_show_p) {
    if (!mono_p) {

      splot_point_colors_used_get (sp, &npoint_colors_used,
        point_colors_used, true);  /* true = binned */

      /*
       * Now loop through point_colors_used[], plotting the points of each
       * color in a group.
      */
      for (k=0; k<npoint_colors_used; k++) {
        current_color = point_colors_used[k];
        gdk_gc_set_foreground (plot_GC, &xg.default_color_table[current_color]);

#ifdef _WIN32
        win32_draw_to_pixmap_binned (&bin0, &bin1, current_color, sp);
#else
        for (ih=bin0.x; ih<=bin1.x; ih++) {
          for (iv=bin0.y; iv<=bin1.y; iv++) {
            for (m=0; m<xg.br_binarray[ih][iv].nels; m++) {
              i = xg.rows_in_plot[xg.br_binarray[ih][iv].els[m]];
              if (!xg.erased_now[i] && xg.color_now[i] == current_color) {
                draw_glyph (sp->pixmap0, &xg.glyph_now[i], sp->screen, i);

                /* parallel coordinate plot whiskers */
                if (display->displaytype == parcoords) {
                  n = 2*i;
                  gdk_draw_line (sp->pixmap0, plot_GC,
                    sp->whiskers[n].x1, sp->whiskers[n].y1,
                    sp->whiskers[n].x2, sp->whiskers[n].y2);
                  n++;
                  gdk_draw_line (sp->pixmap0, plot_GC,
                    sp->whiskers[n].x1, sp->whiskers[n].y1,
                    sp->whiskers[n].x2, sp->whiskers[n].y2);
                }
              }
            }
          }
        }
#endif
      }
    }
  }

  return;
}

/*
void
splot_draw_to_pixmap0 (splotd *sp, gboolean binned) 
{
  if (binned)
    splot_draw_to_pixmap0_binned (sp);
  else
    splot_draw_to_pixmap0_unbinned (sp);
}
*/

static void
splot_add_plot_labels (splotd *sp) {
  gint lbearing, rbearing, width, ascent, descent;
  GtkStyle *style = gtk_widget_get_style (sp->da);
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  gint dtype = display->displaytype;

  gdk_gc_set_foreground (plot_GC, &xg.accent_color);

  if (dtype == scatterplot || dtype == scatmat) {
    if ((dtype == scatterplot && cpanel->projection == XYPLOT) ||
        (dtype == scatmat && sp->p1dvar == -1))
    {

      gdk_text_extents (style->font, 
        xg.vardata[ sp->xyvars.x ].collab_tform,
        strlen (xg.vardata[ sp->xyvars.x ].collab_tform),
        &lbearing, &rbearing, &width, &ascent, &descent);
      gdk_draw_string (sp->pixmap1, style->font, plot_GC,
        sp->max.x - width - 5,
        sp->max.y - 5,
        xg.vardata[ sp->xyvars.x ].collab_tform);

      gdk_text_extents (style->font, 
        xg.vardata[ sp->xyvars.y ].collab_tform,
        strlen (xg.vardata[ sp->xyvars.y ].collab_tform),
        &lbearing, &rbearing, &width, &ascent, &descent);
      gdk_draw_string (sp->pixmap1, style->font, plot_GC,
        5, 5 + ascent + descent,
        xg.vardata[ sp->xyvars.y ].collab_tform);
    }

    if ((dtype == scatterplot && cpanel->projection == P1PLOT) ||
        (dtype == scatmat && sp->p1dvar != -1))
    {

      gdk_text_extents (style->font,
        xg.vardata[ sp->p1dvar ].collab_tform,
        strlen (xg.vardata[ sp->p1dvar ].collab_tform),
        &lbearing, &rbearing, &width, &ascent, &descent);
      gdk_draw_string (sp->pixmap1, style->font, plot_GC,
        sp->max.x - width - 5,
        sp->max.y - 5,
        xg.vardata[ sp->p1dvar ].collab_tform);
    }

  } else if (dtype == parcoords) {

    gdk_text_extents (style->font,
      xg.vardata[ sp->p1dvar ].collab_tform,
      strlen (xg.vardata[ sp->p1dvar ].collab_tform),
      &lbearing, &rbearing, &width, &ascent, &descent);
    gdk_draw_string (sp->pixmap1, style->font, plot_GC,
      5,
      sp->max.y - 5,
      xg.vardata[ sp->p1dvar ].collab_tform);

  }
}

void
splot_add_point_label (splotd *sp, gint k) {
  gint lbearing, rbearing, width, ascent, descent;
  GtkStyle *style = gtk_widget_get_style (sp->da);

  gdk_text_extents (style->font,  
    xg.rowlab[k], strlen (xg.rowlab[k]),
    &lbearing, &rbearing, &width, &ascent, &descent);

  if (sp->screen[k].x <= sp->max.x/2)
    gdk_draw_string (sp->pixmap1, style->font, plot_GC,
      sp->screen[k].x+2, sp->screen[k].y-2, xg.rowlab[k]);
  else
    gdk_draw_string (sp->pixmap1, style->font, plot_GC,
      sp->screen[k].x - width - 2, sp->screen[k].y-2, xg.rowlab[k]);
}

void
splot_draw_border (splotd *sp) {
  if (sp != NULL && sp->da != NULL && sp->da->window != NULL) {
    gdk_gc_set_foreground (plot_GC, &xg.accent_color);
    gdk_gc_set_line_attributes (plot_GC,
      3, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);

    gdk_draw_rectangle (sp->pixmap1, plot_GC,
      FALSE, 1, 1, sp->da->allocation.width-3, sp->da->allocation.height-3);
    gdk_draw_pixmap (sp->da->window, plot_GC, sp->pixmap1,
      0, 0, 0, 0, sp->da->allocation.width, sp->da->allocation.height);

    gdk_gc_set_line_attributes (plot_GC,
      0, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
  }
}


/*------------------------------------------------------------------------*/
/*                   line drawing routines                                */
/*------------------------------------------------------------------------*/

static void
splot_line_colors_used_get (splotd *sp, gint *ncolors_used,
 gushort *colors_used)
{
  gboolean new_color;
  gint i, k;
  displayd *display = (displayd *) sp->displayptr;

  /*
   * Loop once through line_color_now[], collecting the colors
   * currently in use into the line_colors_used[] vector.
  */
  *ncolors_used = 1;
  colors_used[0] = xg.line_color_now[0];

  if (display->segments_directed_show_p || display->segments_undirected_show_p)
  {
    for (i=0; i<xg.nsegments; i++) {
      new_color = true;
      for (k=0; k<*ncolors_used; k++) {
        if (colors_used[k] == xg.line_color_now[i]) {
          new_color = false;
          break;
        }
      }
      if (new_color) {
        colors_used[*ncolors_used] = xg.line_color_now[i];
        (*ncolors_used)++;
      }
    }
  }
}

void
segments_draw (splotd *sp)
{
  gint j, k;
  gint from, to;
  gint nl;
  gushort current_color;
  static gushort line_colors_used[NCOLORS+2];
  static gint nline_colors_used = 1;
  gboolean doit;
  displayd *display = (displayd *) sp->displayptr;

  if (!mono_p) {
    splot_line_colors_used_get (sp, &nline_colors_used, line_colors_used);

    /*
     * Now loop through line_colors_used[], plotting the glyphs of each
     * color in a group.
    */
    for (k=0; k<nline_colors_used; k++) {
      current_color = line_colors_used[k];
      nl = 0;

      for (j=0; j<xg.nsegments; j++) {
        from = xg.segment_endpoints[j].a - 1;
        to = xg.segment_endpoints[j].b - 1;
        doit = true;

        /* If not plotting imputed values, and one is missing, skip it */
/*
        if (!plot_imputed_values && plotted_var_missing(from, to, xg))
          doit = false;
*/
        /* If either from or to is not included, move on */
/*
        else if (xg->ncols == xg->ncols_used) {
          if (!xg->clusv[(int)GROUPID(from)].included)
            doit = False;
          else if (!xg->clusv[(int)GROUPID(to)].included)
            doit = False;
        }
*/
        /* If either from or to is erased, move on */
        if (doit && (xg.erased_now[from] || xg.erased_now[to]))
          doit = false;

        if (doit) {
          if (xg.line_color_now[j] == current_color) {
            sp->segments[nl].x1 = sp->screen[from].x;
            sp->segments[nl].y1 = sp->screen[from].y;
            sp->segments[nl].x2 = sp->screen[to].x;
            sp->segments[nl].y2 = sp->screen[to].y;

            if (display->segments_directed_show_p) {
              /*
               * Add thick piece of the lines to suggest a directional arrow
              */
              sp->arrowheads[nl].x1 =
                (gint) (.2*sp->screen[from].x + .8*sp->screen[to].x);
			  sp->arrowheads[nl].y1 =
                (gint) (.2*sp->screen[from].y + .8*sp->screen[to].y);
			  sp->arrowheads[nl].x2 = sp->screen[to].x;
              sp->arrowheads[nl].y2 = sp->screen[to].y;
            }
            nl++;
          }
        }
      }
      if (!mono_p)
        gdk_gc_set_foreground (plot_GC, &xg.default_color_table[current_color]);

      gdk_draw_segments (sp->pixmap1, plot_GC, sp->segments, nl);

      if (display->segments_directed_show_p) {
        gdk_draw_segments (sp->pixmap1, plot_GC, sp->arrowheads, nl);
      }
    }
  }
}

/*------------------------------------------------------------------------*/
/*    getting from pixmap0 to pixmap1, then pixmap1 to the window         */
/*------------------------------------------------------------------------*/

void
splot_pixmap0_to_pixmap1 (splotd *sp, gboolean binned) {
  GtkWidget *w = sp->da;
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  gint mode = mode_get ();

  if (!binned)
    gdk_draw_pixmap (sp->pixmap1, plot_GC, sp->pixmap0,
                     0, 0, 0, 0,
                     w->allocation.width,
                     w->allocation.height);
  else
    gdk_draw_pixmap (sp->pixmap1, plot_GC, sp->pixmap0,
                      loc0.x, loc0.y,
                      loc0.x, loc0.y,
                      1 + loc1.x - loc0.x ,
                      1 + loc1.y - loc0.y);

  if (display->segments_directed_show_p || display->segments_undirected_show_p)
    if (display->displaytype == scatterplot || display->displaytype == scatmat)
      segments_draw (sp);

  if (cpanel->projection == XYPLOT ||
      cpanel->projection == P1PLOT ||
      cpanel->projection == PCPLOT ||
      cpanel->projection == SCATMAT)
  {
    splot_add_plot_labels (sp);
  }

  if (sp == current_splot) {
    splot_draw_border (sp);

    switch (mode) {
      case BRUSH:
        brush_draw_brush (sp);
        brush_draw_label (sp);
        break;
      case SCALE:
        scaling_visual_cues_draw (sp);
        break;
    }
  }
}

void
splot_pixmap1_to_window (splotd *sp) {
  GtkWidget *w = sp->da;

  gdk_draw_pixmap (w->window, plot_GC, sp->pixmap1,
                   0, 0, 0, 0,
                   w->allocation.width,
                   w->allocation.height);
}

/*------------------------------------------------------------------------*/
/*                   convenience routine                                  */
/*------------------------------------------------------------------------*/

void
splot_redraw (splotd *sp, gint redraw_style) {

  /*-- sometimes the first draw happens before configure is called --*/
  if (sp->da == NULL || sp->pixmap0 == NULL) {
    return;
  }

  switch (redraw_style) {
    case FULL:
      splot_draw_to_pixmap0_unbinned (sp);
      splot_pixmap0_to_pixmap1 (sp, false);  /* false = not binned */
      break;

    case QUICK:
      splot_pixmap0_to_pixmap1 (sp, false);  /* false = not binned */
      break;

    case BINNED:
      splot_draw_to_pixmap0_binned (sp);
      splot_pixmap0_to_pixmap1 (sp, true);  /* true = binned */
      break;

    case EXPOSE:
      break;
  }
  splot_pixmap1_to_window (sp);

  sp->redraw_style = EXPOSE;
}
