/* move_points.c */

#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"

/*------------------------------------------------------------------------*/
/*                       history                                          */
/*------------------------------------------------------------------------*/

static gboolean
move_pt_history_contains (gint i, gint j, ggobid *gg) {

  if (g_slist_length (gg->movepts.history) > 0) {
    GSList *l;
    celld *cell;
    for (l = gg->movepts.history; l; l = l->next) {
      cell = (celld *) l->data;
      if (cell->i == i && cell->j == j) {
        return true;
      }
    }
  }

  return false;
}

void
move_pt_history_add (gint id, splotd *sp, ggobid *gg)
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
    if (!move_pt_history_contains (id, sp->xyvars.x, gg)) {
      cell->i = id;
      cell->j = sp->xyvars.x;
      cell->val = gg->raw.vals[id][sp->xyvars.x];
    }
  }
  gg->movepts.history = g_slist_append (gg->movepts.history, cell);

  cell = (celld *) g_malloc (sizeof (celld));
  cell->i = cell->j = -1;
  if (gg->movepts.direction == vertical || gg->movepts.direction == both) {
    /*-- the cell is (id, sp->xyvars.y), gg->raw.vals[id][sp->xyvars.y] --*/
    if (!move_pt_history_contains (id, sp->xyvars.y, gg)) {
      cell->i = id;
      cell->j = sp->xyvars.y;
      cell->val = gg->raw.vals[id][sp->xyvars.y];
    }
  }
  gg->movepts.history = g_slist_append (gg->movepts.history, cell);
}

void
move_pt_history_delete_last (ggobid *gg)
{
  gint n;

  if ((n = g_slist_length (gg->movepts.history)) > 0) {
    celld *cell = (celld *) g_slist_nth_data (gg->movepts.history, n-1);

    /*-- especially ignore cells with indices == -1 --*/
    if (cell->i > -1 && cell->i < gg->nrows_in_plot) {
      if (cell->j > -1 && cell->j < gg->ncols) {
        gg->raw.vals[cell->i][cell->j] =
          gg->tform1.vals[cell->i][cell->j] =
          gg->tform2.vals[cell->i][cell->j] = cell->val;
      }
    }

    gg->movepts.history = g_slist_remove (gg->movepts.history, cell);
    g_free (cell);
  }
}

/*------------------------------------------------------------------------*/

void
move_pt (gint id, gint x, gint y, splotd *sp, ggobid *gg) {
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
    if (gg->nclust > 1) {
      gint cur_clust = gg->clusterid.vals[id];

      /*
       * Move all points which belong to the same cluster
       * as the selected point.
      */
      for (i=0; i<gg->nrows_in_plot; i++) {
        k = gg->rows_in_plot[i];
        if (k == id)
          ;
        else {
          if (gg->clusterid.vals[k] == cur_clust) {
            if (!gg->hidden_now[k]) {   /* ignore erased values altogether */
              if (horiz)
                sp->planar[k].x += gg->movepts.eps.x;
              if (vert)
                sp->planar[k].y += gg->movepts.eps.y;

              /*-- run only the latter portion of the reverse pipeline --*/
              splot_plane_to_world (sp, k, gg);
              world_to_raw (k, sp, gg);
            }
          }
        }
      }
    }
  }

  /* and now forward again, all the way ... */
  tform_to_world (gg);
  displays_tailpipe (REDISPLAY_ALL, gg);
}

