/* subset.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <string.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

void
subset_init (datad *d, ggobid *gg)
{
  gfloat fnr = (gfloat) d->nrows;

  d->subset.bstart_adj = (GtkAdjustment *)
    gtk_adjustment_new (1.0, 1.0, (fnr-2.0), 1.0, 5.0, 0.0);
  d->subset.bsize_adj = (GtkAdjustment *)
    gtk_adjustment_new (fnr/10.0, 1.0, fnr, 1.0, 5.0, 0.0);

  d->subset.estart_adj = (GtkAdjustment *)
    gtk_adjustment_new (1.0, 1.0, fnr-2.0, 1.0, 5.0, 0.0);
  d->subset.estep_adj = (GtkAdjustment *)
    gtk_adjustment_new (fnr/10.0, 1.0, fnr-1, 1.0, 5.0, 0.0);
}

/*------------------------------------------------------------------*/
/*         utilities used within this file                          */
/*------------------------------------------------------------------*/

static gboolean
add_to_subset (gint i, datad *d, ggobid *gg) {
  gboolean added = false;

  added = true;
  d->sampled.els[i] = true;

  return added;
}

/*-- remove everything from the subset before constructing a new one --*/
static void
subset_clear (datad *d, ggobid *gg) {
  gint i;

  for (i=0; i<d->nrows; i++)
    d->sampled.els[i] = false;
}

/*------------------------------------------------------------------*/

void
subset_apply (datad *d, ggobid *gg) {

  rows_in_plot_set (d, gg);
  clusters_set (d, gg);

  if (gg->cluster_ui.window != NULL)
    cluster_table_update (d, gg);

  tform_to_world (d, gg);

/*
  if (gg->is_pp) {
    gg->recalc_max_min = True;
    reset_pp_plot ();
    pp_index (gg, 0,1);
  }
*/

  displays_tailpipe (FULL, gg);  /*-- points rebinned here --*/
}

void
subset_include_all (datad *d, ggobid *gg) {
  gint i;

  for (i=0; i<d->nrows; i++)
    d->sampled.els[i] = true;
}

/*
 * This algorithm taken from Knuth, Seminumerical Algorithms;
 * Vol 2 of his series.
*/
gboolean
subset_random (gint n, datad *d, ggobid *gg) {
  gint t, m;
  gboolean doneit = false;
  gfloat rrand;

  gint top = d->nrows;

  subset_clear (d, gg);

  if (n > 0 && n < top) {

    for (t=0, m=0; t<top && m<n; t++) {
      rrand = (gfloat) randvalue ();
      if (((top - t) * rrand) < (n - m)) {
        if (add_to_subset (t, d, gg))
          m++;
      }
    }

    doneit = true;
  }

  return (doneit);
}

gboolean
subset_block (gint bstart, gint bsize, datad *d, ggobid *gg)
{
  gint i, b_end;
  gboolean doneit = false;
  gint top = d->nrows;
  top -= 1;

  b_end = bstart + bsize;
  b_end = (b_end >= top) ? top : b_end;

  if (b_end > 0 && b_end <= top &&
      bstart >= 0 && bstart < top &&
      bstart < b_end)
  {
    subset_clear (d, gg);

    for (i=bstart; i<b_end; i++) {
      add_to_subset (i, d, gg);
    }

    doneit = true;
  }
  else quick_message ("The limits aren't correctly specified.", false);
 
  return doneit;
}

gboolean
subset_everyn (gint estart, gint estep, datad *d, ggobid *gg)
{
  gint i;
  gint top = d->nrows;

  gboolean doneit = false;

  top -= 1;
  if (estart >= 0 && estart < top-1 && estep >= 0 && estep < top) {
    subset_clear (d, gg);

    i = estart;
    while (i < top) {
      if (add_to_subset (i, d, gg))
        i += estep;
      else
        i++;
    }

    doneit = true;

  } else quick_message ("Interval not correctly specified.", false);

  return doneit;
}

/*-- create a subset of only the points with sticky ids --*/
/*-- Added by James Brook, Oct 1994 --*/
gboolean
subset_sticky (datad *d, ggobid *gg)
{
  gint id;
  GSList *l;
  gint top = d->nrows;


  if (g_slist_length (d->sticky_ids) > 0) {

    subset_clear (d, gg);

    for (l = d->sticky_ids; l; l = l->next) {
      id = GPOINTER_TO_INT (l->data);
      if (id < top)
        add_to_subset (id, d, gg);
    }
  }

  return true;
}

gboolean
subset_rowlab (gchar *rowlab, datad *d, ggobid *gg)
{
  gint i;
  gint top = d->nrows;

  /*-- remove all sticky labels --*/
  GtkWidget *w = widget_find_by_name (gg->control_panel[IDENT],
    "IDENTIFY:remove_sticky_labels");
  gtk_signal_emit_by_name (GTK_OBJECT (w), "clicked", gg);
  /*-- --*/

  subset_clear (d, gg);

  for (i=0; i<top; i++) {
    if (!strcmp ((gchar *) g_array_index (d->rowlab, gchar *, i), rowlab)) {
      add_to_subset (i, d, gg);
    }
  }

  return true;
}
