/* sp_plot.c */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include <gtk/gtk.h>
#include "vars.h"

/* external functions */
extern void init_plot_GC ();
extern void build_glyph (glyphv *, icoords *, gint, GdkPoint *, gint *,
                         GdkSegment *, gint *, rectd *, gint *, rectd *, gint *,
                         arcd *, gint *, arcd *, gint *);
extern void draw_glyphs (GdkDrawable *, GdkPoint *, gint, GdkSegment *, gint,
                         rectd *, gint, rectd *, gint, arcd *, gint,
                         arcd *, gint);
/*                    */

static void
find_point_colors_used (splotd *sp, gint *ncolors_used,
  gushort *colors_used)  /* these are now integers, 0:ncolors-1 */
{
  gboolean new_color;
  gint i, k, m;

  /*
   * Loop once through xg.color_now[], collecting the colors currently
   * in use into the colors_used[] vector.
  */
  if (xg.point_painting_p) {

    *ncolors_used = 1;
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

    /*
     * Furthermore, make sure that the default color
     * (plotcolors.fg) is first, so that it is drawn
     * underneath the pile of points.  And this should
     * be done after checking for color_id, because
     * sometimes color_id = plotcolors.fg just by default.
    for (k=0; k<(*ncolors_used); k++) {
      if (colors_used[k] == plotcolors.fg && k != 0) {
        colors_used[k] = colors_used[0];
        colors_used[0] = plotcolors.fg;
        break;
      }
    }
    */
  }
}

static void
splot_add_plot_labels (cpaneld *cpanel, splotd *sp) {
  gint lbearing, rbearing, width, ascent, descent;
  GtkStyle *style = gtk_widget_get_style (sp->da);
  displayd *display = (displayd *) sp->displayptr;
  gint dtype = display->displaytype;

  gdk_gc_set_foreground (plot_GC, &xg.accent_color);

  if (dtype == scatterplot || dtype == scatmat) {
    if ((dtype == scatterplot && cpanel->projection == XYPLOT) ||
        (dtype == scatmat && sp->p1dvar == -1))
    {

      gdk_text_extents (style->font, 
        xg.collab[ sp->xyvars.x ],
        strlen (xg.collab[ sp->xyvars.x ]),
        &lbearing, &rbearing, &width, &ascent, &descent);
      gdk_draw_string (sp->pixmap, style->font, plot_GC,
        sp->max.x - width - 5,
        sp->max.y - 5,
        xg.collab[ sp->xyvars.x ]);

      gdk_text_extents (style->font, 
        xg.collab[ sp->xyvars.y ],
        strlen (xg.collab[ sp->xyvars.y ]),
        &lbearing, &rbearing, &width, &ascent, &descent);
      gdk_draw_string (sp->pixmap, style->font, plot_GC,
        5, 5 + ascent + descent,
        xg.collab[ sp->xyvars.y ]);
    }

    if ((dtype == scatterplot && cpanel->projection == P1PLOT) ||
        (dtype == scatmat && sp->p1dvar != -1))
    {

      gdk_text_extents (style->font,
        xg.collab[ sp->p1dvar ],
        strlen (xg.collab[ sp->p1dvar ]),
        &lbearing, &rbearing, &width, &ascent, &descent);
      gdk_draw_string (sp->pixmap, style->font, plot_GC,
        sp->max.x - width - 5,
        sp->max.y - 5,
        xg.collab[ sp->p1dvar ]);
    }

  } else if (dtype == parcoords) {

    gdk_text_extents (style->font,
      xg.collab[ sp->p1dvar ],
      strlen (xg.collab[ sp->p1dvar ]),
      &lbearing, &rbearing, &width, &ascent, &descent);
    gdk_draw_string (sp->pixmap, style->font, plot_GC,
      5,
      sp->max.y - 5,
      xg.collab[ sp->p1dvar ]);

  }

}


void
splot_plot (cpaneld *cpanel, splotd *sp)
{
  gint j, k, m;
  gushort current_color;
  static gint npoint_colors_used = 1;
  static gushort point_colors_used[NCOLORS+2];
  gint npt, nseg, nr_open, nr_filled, nc_open, nc_filled;
  GtkWidget *da = sp->da;
  GdkDrawable *window = da->window;

  gboolean mono = false;
  xg.point_painting_p = true;

  if (plot_GC == NULL)
    init_plot_GC (sp->pixmap);

  /* clear the pixmap */
  gdk_gc_set_foreground (plot_GC, &xg.bg_color);
  gdk_draw_rectangle (sp->pixmap, plot_GC,
                      TRUE, 0, 0,
                      da->allocation.width,
                      da->allocation.height);

  if (cpanel->show_points_p) {
    if (!mono) {
      find_point_colors_used (sp, &npoint_colors_used, point_colors_used);

      /*
       * Now look through point_colors_used[], plotting the points of each
       * color in a group.
      */
      for (k=0; k<npoint_colors_used; k++) {
        current_color = point_colors_used[k];
        npt = nseg = nr_open = nr_filled = nc_open = nc_filled = 0;
        for (m=0; m<xg.nrows_in_plot; m++)
        {
          j = xg.rows_in_plot[m];
          if (!xg.erased[j] && xg.color_now[j] == current_color) {
            build_glyph (&xg.glyph_now[j], sp->screen, j,
              sp->points, &npt,           sp->segs, &nseg,
              sp->open_rects, &nr_open,   sp->filled_rects, &nr_filled,
              sp->open_arcs, &nc_open,    sp->filled_arcs, &nc_filled);
          }
        }

        gdk_gc_set_foreground (plot_GC, &xg.default_color_table[current_color]);

        draw_glyphs (sp->pixmap,
          sp->points, npt,           sp->segs, nseg,
          sp->open_rects, nr_open,   sp->filled_rects, nr_filled,
          sp->open_arcs, nc_open, sp->filled_arcs, nc_filled);
      }
    }
    else  /* if (mono_p) */
    {
      npt = nseg = nr_open = nr_filled = nc_open = nc_filled = 0;
      for (m=0; m<xg.nrows_in_plot; m++)
      {
        j = xg.rows_in_plot[m];

        if (!xg.erased[j])
          build_glyph (&xg.glyph_now[j], sp->screen, j,
            sp->points,       &npt,    
            sp->segs,         &nseg,
            sp->open_rects,   &nr_open,
            sp->filled_rects, &nr_filled,
            sp->open_arcs,    &nc_open,
            sp->filled_arcs,  &nc_filled);
      }

      draw_glyphs (sp->pixmap,
        sp->points,       npt,          
        sp->segs,         nseg,
        sp->open_rects,   nr_open,  
        sp->filled_rects, nr_filled,
        sp->open_arcs,    nc_open,
        sp->filled_arcs,  nc_filled);
    }
  }

  if (cpanel->projection == XYPLOT ||
      cpanel->projection == P1PLOT ||
      cpanel->projection == PCPLOT ||
      cpanel->projection == SCATMAT)
  {
    splot_add_plot_labels (cpanel, sp);
  }
/*
  if (sp->mode == SCALE)
    add_crosshair (sp);
*/

  gdk_draw_pixmap (window, plot_GC, sp->pixmap,
                   0, 0, 0, 0,
                   da->allocation.width,
                   da->allocation.height);

  return;
}

/* deleted plot_bins and everything that follows it */

