/* movepts.c */
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Eclipse Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"

static gboolean movepts_history_contains (gint, gint, GGobiData *, ggobid *);

/*------------------------------------------------------------------------*/
/*                       history                                          */
/*------------------------------------------------------------------------*/

static gboolean
movepts_history_contains (gint i, gint j, GGobiData * d, ggobid * gg)
{

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
movepts_history_add (gint id, splotd * sp, GGobiData * d, ggobid * gg)
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
movepts_history_delete_last (GGobiData * d, ggobid * gg)
{
  gint n;

  if ((n = g_slist_length (d->movepts_history)) > 0) {
    celld *cell = (celld *) g_slist_nth_data (d->movepts_history, n - 1);

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
movept_screen_to_raw (splotd * sp, gint ipt, gcoords * eps,
                      gboolean horiz, gboolean vert, ggobid * gg)
{
  gint j;
  gcoords planar;
  displayd *display = (displayd *) sp->displayptr;
  GGobiData *d = display->d;
  greal *world = (greal *) g_malloc0 (d->ncols * sizeof (greal));
  icoords pos;
  greal *raw = (greal *) g_malloc (d->ncols * sizeof (greal));

  pos.x = sp->screen[ipt].x;
  pos.y = sp->screen[ipt].y;
  for (j = 0; j < d->ncols; j++)
    world[j] = d->world.vals[ipt][j];

  pt_screen_to_plane (&pos, ipt, horiz, vert, eps, &planar, sp);
  pt_plane_to_world (sp, &planar, eps, world);

  for (j = 0; j < d->ncols; j++)
    pt_world_to_raw_by_var (j, world, raw, d);

  for (j = 0; j < d->ncols; j++) {
    d->raw.vals[ipt][j] = d->tform.vals[ipt][j] = raw[j];
    d->world.vals[ipt][j] = world[j];
  }
  sp->planar[ipt].x = planar.x;
  sp->planar[ipt].y = planar.y;

  g_free (raw);
  g_free (world);
}

void
movept_plane_to_raw (splotd * sp, gint ipt, gcoords * eps, GGobiData * d,
                     ggobid * gg)
{
  gint j;
  gcoords planar;
  greal *world = (greal *) g_malloc0 (d->ncols * sizeof (greal));
  greal *raw = (greal *) g_malloc (d->ncols * sizeof (greal));

  planar.x = sp->planar[ipt].x;
  planar.y = sp->planar[ipt].y;
  for (j = 0; j < d->ncols; j++)
    world[j] = d->world.vals[ipt][j];

  pt_plane_to_world (sp, &planar, eps, world);

  for (j = 0; j < d->ncols; j++)
    pt_world_to_raw_by_var (j, world, raw, d);

  for (j = 0; j < d->ncols; j++) {
    d->raw.vals[ipt][j] = d->tform.vals[ipt][j] = raw[j];
    d->world.vals[ipt][j] = world[j];
  }

  g_free (raw);
  g_free (world);
}

/*------------------------------------------------------------------------*/

void
move_pt (gint id, gint x, gint y, splotd * sp, GGobiData * d, ggobid * gg)
{
  gint i, k;
  gboolean horiz, vert;

  g_assert (d->clusterid.nels == d->nrows);
  g_assert (d->hidden.nels == d->nrows);

  horiz = gg->movepts.direction == horizontal
    || gg->movepts.direction == both;
  vert = gg->movepts.direction == vertical || gg->movepts.direction == both;

  if (horiz)                    /* Jump the point horizontally to the mouse position */
    sp->screen[id].x = x;
  if (vert)                     /* Jump the point vertically to the mouse position */
    sp->screen[id].y = y;

  /* run the pipeline backwards for case 'id' */
  movept_screen_to_raw (sp, id, &gg->movepts.eps, horiz, vert, gg);

  /* Let this work even if all points are the same glyph and color */
  if (gg->movepts.cluster_p) {
    gint cur_clust = d->clusterid.els[id];

    /*
     * Move all points which belong to the same cluster
     * as the selected point.
     */
    for (i = 0; i < d->nrows_in_plot; i++) {
      k = d->rows_in_plot.els[i];
      if (k == id);
      else {
        if (d->clusterid.els[k] == cur_clust) {
          if (!d->hidden_now.els[k]) {  /* ignore erased values altogether */
            if (horiz)
              sp->planar[k].x += gg->movepts.eps.x;
            if (vert)
              sp->planar[k].y += gg->movepts.eps.y;

            /*-- run only the latter portion of the reverse pipeline --*/
            movept_plane_to_raw (sp, k, &gg->movepts.eps, d, gg);
          }
        }
      }
    }
  }

  /* and now forward again, all the way ... */
  tform_to_world (d, gg);
  displays_tailpipe (FULL, gg);

  {
    /* Now notify anyone who is interested in this move. */
    GGobiPointMoveEvent ev;
    ev.id = id;
    ev.d = d;
    g_signal_emit (G_OBJECT (gg), GGobiSignals[POINT_MOVE_SIGNAL], 0,
                   sp, id, d);
  }
}
