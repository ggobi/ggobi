/* edges.c */
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

/* --------------------------------------------------------------- */
/*                   Dynamic allocation section                    */
/* --------------------------------------------------------------- */

/*
 * The brushing arrays are handled in brush_init.c, and the
 * edges and arrowheads arrays are handled in splot.c
*/

void
edges_alloc (gint nsegs, datad *d)
{
  d->edge.n = nsegs;
  d->edge.endpoints = (endpointsd *)
    g_realloc (d->edge.endpoints, nsegs * sizeof (endpointsd));

  /*-- wrong: this is in d while the other is in e --*/
  vectorb_alloc (&d->edge.xed_by_brush,  nsegs);
}

void
edges_free (datad *d, ggobid *gg)
{
  vectorb_free (&d->edge.xed_by_brush);
  g_free ((gpointer) d->edge.endpoints);
  d->edge.n = 0;
}

/* --------------------------------------------------------------- */
/*               Add and delete edges                              */
/* --------------------------------------------------------------- */

gboolean
edge_add (gint a, gint b, datad *d, datad *e)
{
  /*-- check whether (a,b) exists before adding?  Not for the moment --*/
  gint n = e->edge.n;
  edges_alloc (e->edge.n+1, e);

/*
 * yi.  This means adding a record to e
 * How many row-wise vectors have to grow by one?
 * If it should have data values, what would we do?
*/
  /*pipeline_arrays_add_rows (e->edge.n, e);*/

  e->edge.endpoints[n].a = d->rowid.idv.els[a];
  e->edge.endpoints[n].b = d->rowid.idv.els[b];

  return true;
}


/* --------------------------------------------------------------- */
/*               Add an edgeset                                    */
/* --------------------------------------------------------------- */

gboolean
edgeset_add (displayd *display)
{
  datad *d = display->d;
  datad *e;
  gint k;
  GList *l;
  gboolean added = false;
  splotd *sp;
  ggobid *gg = GGobiFromDisplay (display);

  if (gg->d != NULL) {
    gint nd = g_slist_length (gg->d);

    if (d->rowid.idv.nels > 0) {
      for (k=0; k<nd; k++) { 
        e = (datad*) g_slist_nth_data (gg->d, k);
        if (e != d && e->edge.n > 0) {
          display->e = e;

          /*-- allocate sp->edges --*/
          for (l=display->splots; l; l=l->next) {
            sp = (splotd *) l->data;
            splot_edges_realloc (sp, e, gg);
          }
          added = true;
          break;
        }
      }
    }
  }
  return added;
}

void
edgeset_add_cb (GtkWidget *w, datad *e) {
  displayd *display = (displayd *) gtk_object_get_data (GTK_OBJECT (w),
    "display");

  ggobid *gg = GGobiFromWidget (w, true);
  GList *l;
  splotd *sp;

  display->e = e;

  for (l=display->splots; l; l=l->next) {
    sp = (splotd *) l->data;
    splot_edges_realloc (sp, e, gg);
  }

  display_plot (display, FULL, gg);
}

