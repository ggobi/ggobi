/* identify.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"

gint
find_nearest_point (icoords *lcursor_pos, splotd *splot, datad *d, ggobid *gg)
{
/*
 * Returns index of nearest un-hidden point
*/
  gint i, k, sqdist, near, xdist, ydist, npoint;

  npoint = -1;

  near = 20*20;  /* If nothing is close, don't show any label */

  for (i=0; i<d->nrows_in_plot; i++) {
    if (!d->hidden_now.els[ k=d->rows_in_plot[i] ]) {
      xdist = splot->screen[k].x - lcursor_pos->x;
      ydist = splot->screen[k].y - lcursor_pos->y;
      sqdist = xdist*xdist + ydist*ydist;
      if (sqdist < near) {
        near = sqdist;
        npoint = k;
      }
    }
  }
  return (npoint);
}

/*-- still having trouble getting identify turned off properly --*/
RedrawStyle
identify_activate (gint state, displayd *display, ggobid *gg)
{
  RedrawStyle redraw_style = NONE;

  if (state == off) {
    datad *d = display->d;
    if (d->nearest_point != -1) redraw_style = QUICK;
    d->nearest_point = -1;
  }

  return redraw_style;
}

void
sticky_id_toggle (datad *d, ggobid *gg)
{
  gint i = 0;
  gboolean i_in_list = false;
  gpointer ptr = NULL;
  void sticky_id_link_by_id (gint, gint, datad *, ggobid *);

  if (d->nearest_point != -1) {

    if (g_slist_length (d->sticky_ids) > 0) {
      GSList *l;
      for (l = d->sticky_ids; l; l = l->next) {
        i = GPOINTER_TO_INT (l->data);
        if (i == d->nearest_point) {
          i_in_list = true;
          ptr = l->data;
          break;
        }
      }
    }

    if (i_in_list) {
      d->sticky_ids = g_slist_remove (d->sticky_ids, ptr);
      sticky_id_link_by_id (STICKY_REMOVE, d->nearest_point, d, gg);
       /* This will become an event on the datad when we move to
          Gtk objects (soon now!) */
      gtk_signal_emit(GTK_OBJECT(gg),
		      GGobiSignals[STICKY_POINT_REMOVED_SIGNAL], d->nearest_point,
		      (gint) UNSTICKY, d);
    } else {
      ptr = GINT_TO_POINTER (d->nearest_point);
      d->sticky_ids = g_slist_append (d->sticky_ids, ptr);
      sticky_id_link_by_id (STICKY_ADD, d->nearest_point, d, gg);
       /* This will become an event on the datad when we move to
          Gtk objects (soon now!) */
      gtk_signal_emit(GTK_OBJECT(gg),
		      GGobiSignals[STICKY_POINT_ADDED_SIGNAL], d->nearest_point,
		      (gint) STICKY, d);
    }
  }
}

/*----------------------------------------------------------------------*/
/*                Linking to other datad's by id                        */
/*----------------------------------------------------------------------*/

void
identify_link_by_id (gint k, datad *source_d, ggobid *gg)
{
  datad *d;
  GSList *l;
  gint i, id;
  gboolean inrange;

  /*-- k is the row number in source_d --*/
  if (k < 0)
    return;

  if (source_d->rowid.id.nels > 0) {
    id = source_d->rowid.id.els[k];
    if (id < 0)  /*-- this would indicate a bug --*/
      return;

    for (l = gg->d; l; l = l->next) {
      d = (datad *) l->data;
      inrange = false;

      if (d == source_d)
        continue;        /*-- skip the originating datad --*/

      /*-- if this id exists is in the range of d's ids ... --*/
      if (d->rowid.id.nels > 0 && d->rowid.idv.nels > id) {
        /*-- i is the row number, irrespective of rows_in_plot --*/
        i = d->rowid.idv.els[id];
        if (id < 0)   /*-- then no cases in d have this id --*/
          ;

        else {

          /*-- if we get here, d has one case with the indicated id --*/
          if (!d->hidden_now.els[i] && d->sampled.els[i]) {
            inrange = true;
            if (i != d->nearest_point) {
              d->nearest_point_prev = d->nearest_point;
              d->nearest_point = i;
            }
          }
        }
      }

      /*
       * if this id does not exist, or is not in the range of d's ids,
       * or does not have a match, then make sure d->nearest_point is
       * set to -1
      */
      if (!inrange) {
        d->nearest_point_prev = d->nearest_point;
        d->nearest_point = -1;
      }
    }  /*-- end for --*/

  }  /*-- end if --*/
}

void
sticky_id_link_by_id (gint whattodo, gint k, datad *source_d, ggobid *gg)
{
  datad *d;
  GSList *l;
  gint i, n, id;
  gboolean i_in_list = false;
  GSList *ll;
  gpointer ptr = NULL;


  /*-- k is the row number in source_d --*/

  if (source_d->rowid.id.nels > 0) {
    id = source_d->rowid.id.els[k];
    if (id < 0)  /*-- this would indicate a bug --*/
      return;

    for (l = gg->d; l; l = l->next) {
      d = (datad *) l->data;
      if (d == source_d)
        continue;        /*-- skip the originating datad --*/
 
      /*-- if this id exists is in the range of d's ids ... --*/
      if (d->rowid.id.nels > 0 && d->rowid.idv.nels > id) {
        /*-- i is the row number, irrespective of rows_in_plot --*/
        i = d->rowid.idv.els[id];
        if (i < 0)  /*-- then no cases in d have this id --*/
          continue;

        if (g_slist_length (d->sticky_ids) > 0) {
          for (ll = d->sticky_ids; ll; ll = ll->next) {
            n = GPOINTER_TO_INT (ll->data);
            if (n == i) {  /*-- the row number of the id --*/
              i_in_list = true;
              ptr = ll->data;
              break;
            }
          }
        }

        if (i_in_list && whattodo == STICKY_REMOVE) {
          d->sticky_ids = g_slist_remove (d->sticky_ids, ptr);
        } else if (!i_in_list && whattodo == STICKY_ADD) {
          ptr = GINT_TO_POINTER (i);
          d->sticky_ids = g_slist_append (d->sticky_ids, ptr);
        }
      }
    }
  }
}
