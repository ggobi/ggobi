/* lineedit.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <math.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

void
edgeedit_init (displayd *display, ggobid *gg)
{
  gg->edgeedit.a = -1;  /*-- index of point where new edge begins --*/
}

/*--------------------------------------------------------------------*/
/*                      Control panel section                         */
/*--------------------------------------------------------------------*/

void
cpanel_edgeedit_init (cpaneld *cpanel, ggobid *gg) {
  cpanel->ee_adding_p = true;
  cpanel->ee_deleting_p = false;
}

/*
 * To handle the case where there are multiple scatterplots which
 * may have different edgeedit options and parameters selected
*/
void
cpanel_edgeedit_set (cpaneld *cpanel, ggobid *gg) {
  /*-- set the radio buttons for adding/deleting edges --*/
/*
  GTK_TOGGLE_BUTTON (gg->edgeedit.brush_on_btn)->active = cpanel->brush_on_p;
*/
}

gint
find_nearest_line (splotd *sp, displayd *display, ggobid *gg)
{
  gint sqdist, near, j, lineid, xdist;
  gint from, to;
  icoords a, b, distab, distac, c;
  gfloat proj;
  gboolean doit;
  datad *e = display->e;
  datad *d = display->d;
  endpointsd *endpoints;
  icoords *mpos = &sp->mousepos;

  lineid = -1;
  if (e->edge.n > 0) {
    xdist = sqdist = near = 1000 * 1000;
    endpoints = e->edge.endpoints;
    for (j=0; j<e->edge.n; j++) {
      from = d->rowid.idv.els[endpoints[j].a];
      to = d->rowid.idv.els[endpoints[j].b];
      doit = (!d->hidden_now.els[from] && !d->hidden_now.els[to]);

      if (doit) {
        a.x = sp->screen[from].x;
        a.y = sp->screen[from].y;
        b.x = sp->screen[to].x;
        b.y = sp->screen[to].y;

        distab.x = b.x - a.x;
        distab.y = b.y - a.y;
        distac.x = mpos->x - a.x;
        distac.y = mpos->y - a.y;

        /* vertical lines */
        if (distab.x == 0 && distab.y != 0) {
          sqdist = distac.x * distac.x;
          if (BETWEEN(a.y, b.y, mpos->y))
            ;
          else {
            int yd = MIN(abs(distac.y), abs(mpos->y - b.y));
            sqdist += (yd * yd);
          }
          if (sqdist <= near) {
            near = sqdist;
            lineid = j;
          }
        }

        /* horizontal lines */
        else if (distab.y == 0 && distab.x != 0) {
          sqdist = distac.y * distac.y ;
          if (sqdist <= near && (int) fabs((float) distac.x) < xdist) {
            near = sqdist;
            xdist = (int) fabs((float) distac.x) ;
            lineid = j;
          }
        }

        /* other lines */
        else if (distab.x != 0 && distab.y != 0) {
          proj = ((float) ((distac.x * distab.x) + (distac.y * distab.y))) /
                 ((float) ((distab.x * distab.x) + (distab.y * distab.y)));

          c.x = (int) (proj * (float) (b.x - a.x)) + a.x;
          c.y = (int) (proj * (float) (b.y - a.y)) + a.y;

          if (BETWEEN(a.x, b.x, c.x) && BETWEEN(a.y, b.y, c.y)) {
            sqdist = (mpos->x - c.x) * (mpos->x - c.x) +
                 (mpos->y - c.y) * (mpos->y - c.y);
          } else {
            sqdist = MIN(
             (mpos->x - a.x) * (mpos->x - a.x) +
             (mpos->y - a.y) * (mpos->y - a.y),
             (mpos->x - b.x) * (mpos->x - b.x) +
             (mpos->y - b.y) * (mpos->y - b.y) );
          }
          if (sqdist < near) {
            near = sqdist;
            lineid = j;
          }
        }
      }
    }
  }
  return(lineid);
}

/*
void
line_edit_proc(xgobidata *xg)
{
  int root_x, root_y;
  unsigned int kb;
  Window root, child;
  icoords cpos;
  static int ocpos_x = 0, ocpos_y = 0;
  static int icount = 0;

*
 * Get the current pointer position.
*
  if (XQueryPointer(display, xg->plot_window, &root, &child,
            &root_x, &root_y, &cpos.x, &cpos.y, &kb))
  {
    *
     * If the pointer is inside the plotting region ...
    *
    if ((0 < cpos.x && cpos.x < xg->max.x) &&
      (0 < cpos.y && cpos.y < xg->max.y))
    {
      icount = 1;
      *
       * If the pointer has moved ...
      *
      if ((cpos.x != ocpos_x) || (cpos.y != ocpos_y))
      {
        ocpos_x = cpos.x;
        ocpos_y = cpos.y;

        if (is_le_adding)
          le_nearest_pt = find_nearest_point(&cpos, xg);

        else if (is_le_deleting &&
          (xg->connect_the_points||xg->plot_the_arrows))
        {
          le_nearest_line = find_nearest_line(&cpos, xg);
        }

        quickplot_once(xg);
      }
    }
    else
    {
      le_nearest_pt = -1;
      le_nearest_line = -1;
      if (icount)
      {
        *
         * Plot once ... then there won't be an id in the plot
         * if the cursor is outside the plot window.
        *

        quickplot_once(xg);
        icount = 0;
      }
    }
  }
}
*/

/*
void
add_line(int a, int b, xgobidata *xg)
{
  int n = xg->nlines;

  xg->nlines++;

  realloc_lines(xg);
  xg->connecting_lines[n].a = a+1;
  xg->connecting_lines[n].b = b+1;

  xg->line_color_now[n]  = plotcolors.fg;
  xg->line_color_ids[n]  = plotcolors.fg;
  xg->line_color_prev[n] = plotcolors.fg;

  sort_connecting_lines(xg);
}
*/

/*
void
edgeediting_marks_draw (xgobidata *xg)
{
  if (is_le_adding)
  {
    if (le_start != -1)
    {
      draw_diamond_around_point(le_start, xg->plot_window, xg);
      if (le_nearest_pt != -1 && is_le_adding)
        draw_dotted_line(le_start, le_nearest_pt, xg->plot_window, xg);
    }
    else
    {
      if (le_nearest_pt > -1)
        draw_diamond_around_point(le_nearest_pt, xg->plot_window, xg);
    }
  }
  else if (is_le_deleting && le_nearest_line > -1 &&
    (xg->connect_the_points||xg->plot_the_arrows) )
  {
    draw_heavy_line(le_nearest_line, xg->plot_window, xg);
  }
}
*/


