/* lineedit.c */
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Common Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*--------------------------------------------------------------------*/

gboolean
record_add (eeMode mode, gint a, gint b, gchar * lbl, gchar * id,
            gchar ** vals, GGobiStage * d, GGobiStage * e, ggobid * gg)
{
  guint i, j;
  GList *l, *sl;
  splotd *sp;
  displayd *dsp;
  GGobiStage *dtarget = d;
  greal *raw = NULL, x;

  g_return_val_if_fail(d->gg == e->gg, false);

  /*-- eventually check whether a->b already exists before adding --*/
  if (mode == ADDING_EDGES) {
    g_assert (ggobi_stage_get_n_edges(e) == e->n_rows);
    g_assert (a >= 0 && b >= 0 && a != b);
    dtarget = e;
  }
  
  if (!id)
    id = g_strdup_printf ("%d", dtarget->n_rows + 1);
  
  if (ggobi_stage_get_row_for_id(d, id) != -1) {
    g_printerr ("That id (%s) is already used\n", id);
    return false;
  }

  i = ggobi_data_add_rows(GGOBI_DATA(dtarget), 1);
  ggobi_stage_set_row_id(dtarget, i, lbl, true);

  if (ggobi_stage_get_n_cols(dtarget)) {
    for (j = 0; j < dtarget->n_cols; j++) {
      if (strcmp (vals[j], "NA") == 0) {  /*-- got a missing --*/
        ggobi_stage_set_missing(dtarget, i, j);
      } else {
        GGobiVariable *var = ggobi_stage_get_variable(d, j);
        x = (greal) atof (vals[j]);
        if (GGOBI_VARIABLE_IS_CATEGORICAL(var)) {
          raw[j] = ggobi_variable_get_level_value_closest(var, x);
        }
        else
          ggobi_stage_set_raw_value(dtarget, i, j, x);
          
      }
      g_signal_emit_by_name(d, "col_data_changed", j);
    }
  }


  if (mode == ADDING_EDGES) {
    GGOBI_STAGE_ATTR_INIT_ALL(dtarget);  
    GGOBI_STAGE_SET_ATTR_COLOR(dtarget, i, GGOBI_STAGE_GET_ATTR_COLOR(dtarget, i), ATTR_SET_PERSISTENT);
    //dtarget->color.els[i] = dtarget->color_now.els[i] = d->color.els[a];

    edges_alloc (e->n_rows, e);
    ggobi_stage_get_edge_data(e)->sym_endpoints[dtarget->n_rows - 1].a = ggobi_stage_get_row_id(d, a);
    ggobi_stage_get_edge_data(e)->sym_endpoints[dtarget->n_rows - 1].b = ggobi_stage_get_row_id(d, b);
    ggobi_stage_get_edge_data(e)->sym_endpoints[dtarget->n_rows - 1].jpartner = -1;  /* XXX */
    unresolveAllEdgePoints (e);
    resolveEdgePoints (e, d);
    /*
     * If this is the first edge in the edge set, do something to
     * make it show up in the display menu.
     */
    if (e->n_rows == 1) {
      void ggobi_edge_menus_update (ggobid * gg);
      ggobi_edge_menus_update (gg);
    }

  }
  else {
    GSList *l;
    GGobiStage *dd;
    for (l = gg->d; l; l = l->next) {
      dd = (GGobiStage *) l->data;
      if (dd != dtarget && ggobi_stage_get_n_edges(dd)) {
        if (hasEdgePoints (dd, dtarget)) {
          unresolveAllEdgePoints (dd);
          resolveEdgePoints (dd, dtarget);
        }
      }
    }
  }

/*
DTL: So need to call unresolveEdgePoints(e, d) to remove it from the 
     list of previously resolved entries.
     Can do better by just re-allocing the endpoints in the
     DatadEndpoints struct and putting the new entry into that,
     except we have to check it resolves correctly, etc. So
     unresolveEdgePoints() will just cause entire collection to be
     recomputed.
*/

/*
 * This will be handled with signals, where each splotd listens
 * for (maybe) point_added or edge_added events.
 * New bug: sp->bar needs to be reinitialized.  This code needs
 * to be class-sensitive, and it isn't.
*/
/* could put some code in splot_record_add  */
  if (mode == ADDING_EDGES) {
    for (l = gg->displays; l; l = l->next) {
      dsp = (displayd *) l->data;
      if (dsp->e == e) {
        for (sl = dsp->splots; sl; sl = sl->next) {
          sp = (splotd *) sl->data;
          if (sp != NULL)
            splot_edges_realloc (dtarget->n_rows - 1, sp, e);
        }
      }
    }
  }

  if (ggobi_stage_get_n_cols(dtarget)) {
    for (l = gg->displays; l; l = l->next) {
      dsp = (displayd *) l->data;
      if (dsp->d == dtarget) {
        for (sl = dsp->splots; sl; sl = sl->next) {
          sp = (splotd *) sl->data;
          if (sp != NULL)
            splot_points_realloc (dtarget->n_rows - 1, sp, d);

          /*-- this is only necessary if there are variables, I think --*/
          if (GGOBI_IS_EXTENDED_SPLOT (sp)) {
            GGobiExtendedSPlotClass *klass;
            klass = GGOBI_EXTENDED_SPLOT_GET_CLASS (sp);
            if (klass->alloc_whiskers)
              sp->whiskers = klass->alloc_whiskers (sp->whiskers, sp,
                                                    d->n_rows, d);

            /*-- each plot type should have its own realloc routines --*/
            if (GGOBI_IS_BARCHART_SPLOT (sp)) {
              barchartSPlotd *bsp = GGOBI_BARCHART_SPLOT (sp);
              barchart_clean_init (bsp);
              barchart_recalc_counts (bsp, d, gg);
            }
          }
        }
      }
    }
  }

  displays_tailpipe (FULL, gg);

  return true;
}

/*
 * Add a data record, filling in all values with reasonable defaults.
 * This is executed when the middle or right button is used for edge
 * editing.
*/
void
record_add_defaults (GGobiStage * d, GGobiStage * e, displayd * display,
                     ggobid * gg)
{
  cpaneld *cpanel = &display->cpanel;
  GGobiStage *dtarget;
  gchar *lbl;
  gchar **vals = NULL;
  gint j;

  dtarget = (cpanel->ee_mode == ADDING_EDGES) ? e : d;

  if (ggobi_stage_get_n_cols(dtarget)) {
    void fetch_default_record_values (gchar ** vals,
                                      GGobiStage *, displayd *, ggobid * gg);
    vals = (gchar **) g_malloc (dtarget->n_cols * sizeof (gchar *));
    fetch_default_record_values (vals, dtarget, display, gg);
  }

  lbl = g_strdup_printf ("%d", dtarget->n_rows + 1); /* record label and id */

  if (cpanel->ee_mode == ADDING_EDGES) {
    record_add (cpanel->ee_mode, gg->edgeedit.a, d->nearest_point,
                lbl, lbl, vals, d, e, gg);
  }
  else if (cpanel->ee_mode == ADDING_POINTS) {
    record_add (cpanel->ee_mode, -1, -1, lbl, lbl, vals, d, e, gg);
  }

  if (ggobi_stage_get_n_cols(dtarget)) {
    for (j = 0; j < dtarget->n_cols; j++)
      g_free (vals[j]);
    g_free (vals);
  }

}

/*--------------------------------------------------------------------*/

void
edgeedit_init (ggobid * gg)
{
  gg->edgeedit.a = -1;  /*-- index of point where new edge begins --*/
}

gint
find_nearest_edge (splotd * sp, displayd * display, ggobid * gg)
{
  gint sqdist, near, j, lineid, xdist;
  gint from, to, yd;
  icoords a, b, distab, distac, c;
  gfloat proj;
  gboolean doit;
  GGobiStage *e = display->e;
  GGobiStage *d = display->d;
  icoords *mpos = &sp->mousepos;

  lineid = -1;
  near = 20 * 20;               /* If nothing is close, don't show any label */

  if (e && ggobi_stage_get_n_edges(e)) {
    endpointsd *endpoints = resolveEdgePoints (e, d);
    if (!endpoints)
      return (-1);

    xdist = sqdist = 1000 * 1000;
    GGOBI_STAGE_ATTR_INIT_ALL(d);  
    for (j = 0; j < ggobi_stage_get_n_edges(e); j++) {
      doit = edge_endpoints_get (j, &from, &to, d, endpoints, e);
      doit = doit && (!GGOBI_STAGE_GET_ATTR_HIDDEN(d, from) && !GGOBI_STAGE_GET_ATTR_HIDDEN(d, to));

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
          if (BETWEEN (a.y, b.y, mpos->y));
          else {
            yd = MIN (abs (distac.y), abs (mpos->y - b.y));
            sqdist += (yd * yd);
          }
          if (sqdist <= near) {
            near = sqdist;
            lineid = j;
          }
        }

        /* horizontal lines */
        else if (distab.y == 0 && distab.x != 0) {
          sqdist = distac.y * distac.y;
          if (sqdist <= near && (gint) fabs ((gfloat) distac.x) < xdist) {
            near = sqdist;
            xdist = (gint) fabs ((gfloat) distac.x);
            lineid = j;
          }
        }

        /* other lines */
        else if (distab.x != 0 && distab.y != 0) {
          proj = ((gfloat) ((distac.x * distab.x) + (distac.y * distab.y))) /
            ((gfloat) ((distab.x * distab.x) + (distab.y * distab.y)));

          c.x = (gint) (proj * (gfloat) (b.x - a.x)) + a.x;
          c.y = (gint) (proj * (gfloat) (b.y - a.y)) + a.y;

          if (BETWEEN (a.x, b.x, c.x) && BETWEEN (a.y, b.y, c.y)) {
            sqdist = (mpos->x - c.x) * (mpos->x - c.x) +
              (mpos->y - c.y) * (mpos->y - c.y);
          }
          else {
            sqdist = MIN ((mpos->x - a.x) * (mpos->x - a.x) +
                          (mpos->y - a.y) * (mpos->y - a.y),
                          (mpos->x - b.x) * (mpos->x - b.x) +
                          (mpos->y - b.y) * (mpos->y - b.y));
          }
          if (sqdist < near) {
            near = sqdist;
            lineid = j;
          }
        }
      }
    }

    /* if this edge is bidirectional, figure out which segment
       we really want to highlight.  Use the distance from the
       mouse to the two endpoints to decide. */
    if (lineid != -1) {
      GGOBI_STAGE_ATTR_INIT(e, hidden);  
      j = endpoints[lineid].jpartner;
      if (j != -1 && !GGOBI_STAGE_GET_ATTR_HIDDEN(e, j)) {

        edge_endpoints_get (lineid, &from, &to, d, endpoints, e);

        a.x = sp->screen[from].x;
        a.y = sp->screen[from].y;
        b.x = sp->screen[to].x;
        b.y = sp->screen[to].y;

        if ((mpos->x - a.x) * (mpos->x - a.x) +
            (mpos->y - a.y) * (mpos->y - a.y) >
            (mpos->x - b.x) * (mpos->x - b.x) +
            (mpos->y - b.y) * (mpos->y - b.y)) {
          lineid = j;
        }
      }
    }
  }
  return (lineid);
}


void
fetch_default_record_values (gchar ** vals, GGobiStage * dtarget,
                             displayd * display, ggobid * gg)
{
  gint j;
  gcoords eps;

  if (dtarget == display->d) {
    /*-- use the screen position --*/
    greal *raw = (greal *) g_malloc (dtarget->n_cols * sizeof (greal));
    pt_screen_to_raw (&gg->current_splot->mousepos, -1, true, true, /* no id, both horiz and vert are true */
                      raw, &eps, dtarget, gg->current_splot, gg);
    for (j = 0; j < dtarget->n_cols; j++) {
      GGobiVariable *var = ggobi_stage_get_variable(dtarget, j);
      if (GGOBI_VARIABLE_IS_CATEGORICAL(var)) 
        vals[j] = g_strdup_printf ("%d", ggobi_variable_get_level_value_closest(var, raw[j]));
      else
        vals[j] = g_strdup_printf ("%g", raw[j]);
    }
    g_free (raw);
  }
  else {                        /* for edges, use NA's */
    for (j = 0; j < dtarget->n_cols; j++)
      vals[j] = g_strdup ("NA");
  }
}
