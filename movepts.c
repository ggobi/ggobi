/* move_points.c */
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

static gboolean movepts_history_contains (gint, gint, datad *, ggobid *);

/*------------------------------------------------------------------------*/
/*                       history                                          */
/*------------------------------------------------------------------------*/

static gboolean
movepts_history_contains (gint i, gint j, datad *d, ggobid *gg) {

  if (g_slist_length (d->movepts_history) > 0) {
    GSList *l;
    celld *cell;
    for (l = d->movepts_history; l; l = l->next) {
      cell = (celld *) l->data;
      if (cell->i == i && cell->j == j) {
        return true;
      }
    }
  }

  return false;
}

void
movepts_history_add (gint id, splotd *sp, datad *d, ggobid *gg)
{
/*
 * So that it's possible to do 'undo last', always add two
 * celld elements.  In the case that motion is not happening
 * in two directions, or one of them is redundant, then it
 * can be (-1, -1, NULL).
*/ 
  celld *cell;

  cell = (celld *) g_malloc (sizeof (celld));
  cell->i = cell->j = -1;
  if (gg->movepts.direction == horizontal || gg->movepts.direction == both) {
    /*-- the cell is (id, sp->xyvars.x), gg->raw.vals[id][sp->xyvars.x] --*/
    if (!movepts_history_contains (id, sp->xyvars.x, d, gg)) {
      cell->i = id;
      cell->j = sp->xyvars.x;
      cell->val = d->raw.vals[id][sp->xyvars.x];
    }
  }
  d->movepts_history = g_slist_append (d->movepts_history, cell);

  cell = (celld *) g_malloc (sizeof (celld));
  cell->i = cell->j = -1;
  if (gg->movepts.direction == vertical || gg->movepts.direction == both) {
    /*-- the cell is (id, sp->xyvars.y), gg->raw.vals[id][sp->xyvars.y] --*/
    if (!movepts_history_contains (id, sp->xyvars.y, d, gg)) {
      cell->i = id;
      cell->j = sp->xyvars.y;
      cell->val = d->raw.vals[id][sp->xyvars.y];
    }
  }
  d->movepts_history = g_slist_append (d->movepts_history, cell);
}

void
movepts_history_delete_last (datad *d, ggobid *gg)
{
  gint n;

  if ((n = g_slist_length (d->movepts_history)) > 0) {
    celld *cell = (celld *) g_slist_nth_data (d->movepts_history, n-1);

    /*-- especially ignore cells with indices == -1 --*/
    if (cell->i > -1 && cell->i < d->nrows_in_plot) {
      if (cell->j > -1 && cell->j < d->ncols) {
        d->raw.vals[cell->i][cell->j] =
          d->tform.vals[cell->i][cell->j] = cell->val;
      }
    }

    d->movepts_history = g_slist_remove (d->movepts_history, cell);
    g_free (cell);
  }
}

/*------------------------------------------------------------------------*/

void
move_pt (gint id, gint x, gint y, splotd *sp, datad *d, ggobid *gg) {
  gint i, k;
  gboolean horiz, vert;

  horiz = gg->movepts.direction == horizontal || gg->movepts.direction == both;
  vert = gg->movepts.direction == vert || gg->movepts.direction == both;

  if (horiz)
    sp->screen[id].x = x;
  if (vert)
    sp->screen[id].y = y;

  /* run the pipeline backwards for case 'id' */
  splot_reverse_pipeline (sp, id, &gg->movepts.eps, horiz, vert, gg);

  if (gg->movepts.cluster_p) {
    if (d->nclusters > 1) {
      gint cur_clust = d->clusterids.vals[id];

      /*
       * Move all points which belong to the same cluster
       * as the selected point.
      */
      for (i=0; i<d->nrows_in_plot; i++) {
        k = d->rows_in_plot[i];
        if (k == id)
          ;
        else {
          if (d->clusterids.vals[k] == cur_clust) {
            if (!d->hidden_now[k]) {   /* ignore erased values altogether */
              if (horiz)
                sp->planar[k].x += gg->movepts.eps.x;
              if (vert)
                sp->planar[k].y += gg->movepts.eps.y;

              /*-- run only the latter portion of the reverse pipeline --*/
              splot_plane_to_world (sp, k, gg);
              world_to_raw (k, sp, d, gg);
            }
          }
        }
      }
    }
  }

  /* and now forward again, all the way ... */
  tform_to_world (d, gg);
  displays_tailpipe (REDISPLAY_ALL, gg);
}

