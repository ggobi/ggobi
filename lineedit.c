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
#include <string.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*--------------------------------------------------------------------*/

gboolean record_add (eeMode mode, gint a, gint b, gchar *lbl, gchar *id,
  gchar **vals, datad * d, datad * e, ggobid *gg)
{
  gchar *s1, *s2;
  gint i, j;
  GList *l, *sl;
  splotd *sp;
  displayd *dsp;
  datad *dtarget = d;
  greal *raw = NULL;
  vartabled *vt;
  gboolean found_missings = false;

  /*-- eventually check whether a->b already exists before adding --*/
  if (mode == ADDING_EDGES) {
    g_assert (e->edge.n == e->nrows);
    g_assert (a >= 0 && b >= 0 && a != b);

    dtarget = e;
  }

  /*-- Make sure the id is unique --*/
  if (dtarget->idTable && id) {
    gchar *stmp;
    if (id && strlen(id) > 0) stmp = g_strdup (id);
    else stmp = g_strdup_printf ("%d", dtarget->nrows+1);
    for (i=0; i<dtarget->nrows; i++) {
      if (strcmp(stmp, dtarget->rowIds[i]) == 0) {
        g_printerr ("That id (%s) is already used (record %d)\n",
          stmp, i);
        g_free (stmp);
        return false;
      }
    }
    g_free (stmp);
  }

  if (dtarget->ncols) {
    raw = (greal *) g_malloc (dtarget->ncols * sizeof(greal));
    for (j=0; j<dtarget->ncols; j++) {
      if (strcmp (vals[j], "NA") == 0) {  /*-- got a missing --*/
        raw[j] = (greal) 0.0;  /*-- or what? --*/
        found_missings = true;
      } else {
        raw[j] = (greal) atof (vals[j]);
      }
    }
  }

  /*-- Here's what the datad needs --*/
/*
 * some of this can be encapsulated as datad_record_add, as long
 * as problems with the sequence of operations don't arise.
*/
  dtarget->nrows += 1;

  /*-- add a row label --*/
  if (lbl && strlen(lbl) > 0) {
    rowlabel_add (lbl, dtarget);
  } else {
    s1 = g_strdup_printf ("%d", dtarget->nrows);
    rowlabel_add (s1, dtarget);
  }

  /*-- if necessary, add an id --*/
  if (dtarget->idTable) {
    if (id && strlen(id) > 0) {
      datad_record_id_add (id, dtarget);
    } else {
      s2 = g_strdup_printf ("%d", dtarget->nrows);
      datad_record_id_add (s2, dtarget);  /*-- don't free s2 --*/
    }
  }

  pipeline_arrays_check_dimensions (dtarget);
  /*
   * Resetting rows_in_plot causes a rows_in_plot_changed
   * signal to be emitted, and the tour responds to that.
  */
  rows_in_plot_set (dtarget, gg);

  /*-- allocate and initialize brushing arrays --*/
  br_glyph_ids_add (dtarget, gg);
  br_color_ids_add (dtarget, gg);
  br_hidden_alloc (dtarget);
  vectorb_realloc (&dtarget->pts_under_brush, dtarget->nrows);
  clusters_set (dtarget, gg);

  if (found_missings) {
    if (dtarget->nmissing == 0) {
      arrays_alloc (&dtarget->missing, dtarget->nrows, dtarget->ncols);
      arrays_zero (&dtarget->missing);
    } else {
      arrays_add_rows (&dtarget->missing, dtarget->nrows);
    }
    for (j=0; j<dtarget->ncols; j++) {
      if (strcmp (vals[j], "NA") == 0) {  /*-- got a missing --*/
        dtarget->nmissing++;
        dtarget->missing.vals[dtarget->nrows-1][j] = 1;
        vt = vartable_element_get (j, dtarget);
        vt->nmissing++;
      }
    }
  }

  /*-- read in the data, push it through the first part of the pipeline --*/
  if (dtarget->ncols) {
    for (j=0; j<dtarget->ncols; j++) {
      dtarget->raw.vals[dtarget->nrows-1][j] = 
        dtarget->tform.vals[dtarget->nrows-1][j] = raw[j];
      tform_to_world_by_var (j, dtarget, gg);
    }
  }

  if (mode == ADDING_EDGES) {
    edges_alloc(e->nrows, e);
    e->edge.sym_endpoints[dtarget->nrows-1].a = g_strdup (d->rowIds[a]);
    e->edge.sym_endpoints[dtarget->nrows-1].b = g_strdup (d->rowIds[b]);
    e->edge.sym_endpoints[dtarget->nrows-1].jpartner = -1; /* XXX */
    unresolveAllEdgePoints(e);
    resolveEdgePoints (e, d);
  } else {
    GSList *l;
    datad *dd;
    for (l=gg->d; l; l=l->next) {
      dd = (datad *) l->data;
      if (dd != dtarget && dd->edge.n > 0) {
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
*/
/* could put some code in splot_record_add  */
  if (mode == ADDING_EDGES) {
    for (l=gg->displays; l; l=l->next) {
      dsp = (displayd *) l->data;
      if (dsp->e == e) {
        for (sl = dsp->splots; sl; sl = sl->next) {
          sp = (splotd *) sl->data;
          if (sp != NULL)
            splot_edges_realloc (dtarget->nrows-1, sp, e);
        }
      }
    }
  }

  if (dtarget->ncols) {
    for (l=gg->displays; l; l=l->next) {
      dsp = (displayd *) l->data;
      if (dsp->d == dtarget) {
        for (sl = dsp->splots; sl; sl = sl->next) {
          sp = (splotd *) sl->data;
          if (sp != NULL)
            splot_points_realloc (dtarget->nrows-1, sp, d);
          
          /*-- this is only necessary if there are variables, I think --*/
          if (GTK_IS_GGOBI_EXTENDED_SPLOT(sp)) {
            GtkGGobiExtendedSPlotClass *klass;
            klass = GTK_GGOBI_EXTENDED_SPLOT_CLASS(GTK_OBJECT(sp)->klass);
            if(klass->alloc_whiskers)
              sp->whiskers = klass->alloc_whiskers(sp->whiskers, sp,
                d->nrows, d);
          }
        }
      }
    }
  }
/*  */


  displays_tailpipe (FULL, gg);

  return true;
}

/*
 * Add a data record, filling in all values with reasonable defaults.
 * This is executed when the middle or right button is used for edge
 * editing.
*/
void
record_add_defaults (datad *d, datad *e, displayd *display, ggobid *gg)
{
  cpaneld *cpanel = &display->cpanel;
  datad *dtarget;
  gchar *lbl;
  gchar **vals = NULL;
  gint j;

  dtarget = (cpanel->ee_mode == ADDING_EDGES)?e:d;

  if (dtarget->ncols) {
    void fetch_default_record_values (gchar **vals, 
      datad *, displayd *, ggobid *gg);
    vals = (gchar **) g_malloc (dtarget->ncols * sizeof (gchar *));
    fetch_default_record_values (vals, dtarget, display, gg);
  }

  lbl = g_strdup_printf("%d", dtarget->nrows+1); /* record label and id */

  if (cpanel->ee_mode == ADDING_EDGES) {
    record_add (cpanel->ee_mode, gg->edgeedit.a, d->nearest_point,
      lbl, lbl, vals, d, e, gg);
  } else if (cpanel->ee_mode == ADDING_POINTS) {
    record_add (cpanel->ee_mode, -1, -1, lbl, lbl, vals, d, e, gg);
  }

  if (dtarget->ncols) {
    for (j=0; j<dtarget->ncols; j++)
      g_free (vals[j]);
    g_free (vals);
  }

}

/*--------------------------------------------------------------------*/

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
    if(!endpoints)
       return(-1);
  
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


/*--------------------------------------------------------------------*/
/* Reverse pipeline code for populating the table of variable values  */
/*--------------------------------------------------------------------*/

/*
 * if I want to use this in movepts, it needs three more inputs
 *        gboolean horiz, gboolean vert       (both true here)
 *        gint i: the point index             (-1 here?)
 * and one more return value
 *        gcoords *eps                         (meaningful for i != -1)
*/

void
pt_screen_to_plane (icoords *screen, gcoords *planar, splotd *sp)
{
  gfloat scale_x, scale_y;
  greal precis = (greal) PRECISION1;

  scale_x = sp->scale.x;
  scale_y = sp->scale.y;
  scale_x /= 2;
  sp->iscale.x = (greal) sp->max.x * scale_x;
  scale_y /= 2;
  sp->iscale.y = -1 * (greal) sp->max.y * scale_y;

  screen->x -= sp->max.x/2;
  planar->x = (greal) screen->x * precis / sp->iscale.x ;
  planar->x += (greal) sp->pmid.x;

  screen->y -= sp->max.y/2;
  planar->y = (greal) screen->y * precis / sp->iscale.y ;
  planar->y += (greal) sp->pmid.y;
}

void
pt_plane_to_world (splotd *sp, gcoords *planar, greal *world) 
{ 
  displayd *display = (displayd *) sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  gint j, var;

  switch (cpanel->projection) {
    case P1PLOT:
      if (display->p1d_orientation == VERTICAL)
        world[sp->p1dvar] = planar->y;
      else
        world[sp->p1dvar] = planar->x;
    break;
    case XYPLOT:
      world[sp->xyvars.x] = planar->x;
      world[sp->xyvars.y] = planar->y;
    break;
    case TOUR1D:
    /*if (!gg->is_pp) {*/
      for (j=0; j<display->t1d.nactive; j++) {
        var = display->t1d.active_vars.els[j];
        world[var] += (planar->x * (greal) display->t1d.F.vals[0][var]);
      }
    /*}*/
    break;
    case TOUR2D3:
      for (j=0; j<display->t2d3.nactive; j++) {
        var = display->t2d3.active_vars.els[j];
        world[var] += 
         (planar->x * (greal) display->t2d3.F.vals[0][var] +
          planar->y * (greal) display->t2d3.F.vals[1][var]);
      }
	break;
    case TOUR2D:
    /*if (!gg->is_pp) {*/
        for (j=0; j<display->t2d.nactive; j++) {
          var = display->t2d.active_vars.els[j];
          world[var] += 
           (planar->x * (greal) display->t2d.F.vals[0][var] +
            planar->y * (greal) display->t2d.F.vals[1][var]);
        }
    /*}*/
    break;
    case COTOUR:
    /*if (!gg->is_pp) {*/
      for (j=0; j<display->tcorr1.nactive; j++) {
        var = display->tcorr1.active_vars.els[j];
        world[var] += (planar->x * (greal) display->tcorr1.F.vals[0][var]);
      }
      for (j=0; j<display->tcorr2.nactive; j++) {
        var = display->tcorr2.active_vars.els[j];
        world[var] += (planar->y * (greal) display->tcorr2.F.vals[0][var]);
      }
    /*}*/
    break;
    default:
      g_printerr ("reverse pipeline not yet implemented for this projection\n");
  }
}

void
pt_world_to_raw_by_var (gint j, greal *world, greal *raw, datad *d)
{
  gfloat precis = PRECISION1;
  gfloat ftmp, rdiff;
  gfloat x;
  vartabled *vt = vartable_element_get (j, d);

  rdiff = vt->lim.max - vt->lim.min;

  ftmp = (gfloat) (world[j]) / precis;
  x = (ftmp + 1.0) * .5 * rdiff;
  x += vt->lim.min;

  raw[j] = x;
}

void
pt_screen_to_raw (icoords *screen, greal *raw,
  datad *d, splotd *sp, ggobid *gg)
{
  gint j;
  gcoords planar;
  greal *world = (greal *) g_malloc0 (d->ncols * sizeof(greal));

  pt_screen_to_plane (screen, &planar, sp);
  pt_plane_to_world (sp, &planar, world); 

  for (j=0; j<d->ncols; j++)
    pt_world_to_raw_by_var (j, world, raw, d);

  g_free (world);
}

void
fetch_default_record_values (gchar **vals, datad *dtarget, displayd *display,
  ggobid *gg)
{
  gint j;

  if (dtarget == display->d) {
    /*-- use the screen position --*/
    greal *raw = (greal *) g_malloc (dtarget->ncols * sizeof (greal));
    pt_screen_to_raw (&gg->current_splot->mousepos, raw,
      dtarget, gg->current_splot, gg);
    for (j=0; j<dtarget->ncols; j++)
      vals[j] = g_strdup_printf ("%g", raw[j]);
    g_free (raw);
  } else {  /* for edges, use NA's */
    for (j=0; j<dtarget->ncols; j++)
      vals[j] = g_strdup ("NA");
  }
}
