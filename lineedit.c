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
edgeedit_init (ggobid *gg)
{
  gg->edgeedit.a = -1;  /*-- index of point where new edge begins --*/
}

gint
find_nearest_edge (splotd *sp, displayd *display, ggobid *gg)
{
  gint sqdist, near, j, lineid, xdist;
  gint from, to, yd;
  icoords a, b, distab, distac, c;
  gfloat proj;
  gboolean doit;
  datad *e = display->e;
  datad *d = display->d;
  icoords *mpos = &sp->mousepos;

  lineid = -1;
  near = 20*20;  /* If nothing is close, don't show any label */

  if (e && e->edge.n > 0) {
    endpointsd *endpoints = resolveEdgePoints(e, d);

    xdist = sqdist = 1000 * 1000;
    for (j=0; j<e->edge.n; j++) {
      doit = edge_endpoints_get (j, &from, &to, d, endpoints, e);
      doit = doit && (!d->hidden_now.els[from] && !d->hidden_now.els[to]);

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
            yd = MIN(abs(distac.y), abs(mpos->y - b.y));
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
          if (sqdist <= near && (gint) fabs((gfloat) distac.x) < xdist) {
            near = sqdist;
            xdist = (gint) fabs((gfloat) distac.x) ;
            lineid = j;
          }
        }

        /* other lines */
        else if (distab.x != 0 && distab.y != 0) {
          proj = ((gfloat) ((distac.x * distab.x) + (distac.y * distab.y))) /
                 ((gfloat) ((distab.x * distab.x) + (distab.y * distab.y)));

          c.x = (gint) (proj * (gfloat) (b.x - a.x)) + a.x;
          c.y = (gint) (proj * (gfloat) (b.y - a.y)) + a.y;

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
