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

/*
Todo:
 not yet rebinning the screen when rows_in_plot changes
   -- move some of this code once hiding/exclusion are implemented
 only half-handling rgroups
 probably a bit more work will be needed when tour is added
*/

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*------------------------------------------------------------------*/
/*         utilities used within this file                          */
/*------------------------------------------------------------------*/

static gboolean
add_to_subset (gint i, datad *d, ggobid *gg) {
  gint j, el;
  gboolean added = false;

  if (d->nrgroups > 0) {
    if (d->rgroups[i].included) {
      added = true;
      d->rgroups[i].sampled = true;
      for (j=0; j<d->rgroups[i].nels; j++) {
        el = d->rgroups[i].els[j];
        d->sampled[el] = true;
      }
    }

  } else {
    if (d->included[i] == true) {
      added = true;
      d->sampled[i] = true;
    }
  }

  return added;
}

/*-- remove everything from the subset before constructing a new one --*/
static void
subset_clear (datad *d, ggobid *gg) {
  gint i, rgid;

  for (i=0; i<d->nrows; i++)
    d->sampled[i] = false;

  for (i=0; i<d->nrgroups; i++) {
    rgid = d->rgroup_ids[i];
    d->rgroups[rgid].sampled = false;
  }
}

/*------------------------------------------------------------------*/

void
subset_apply (gboolean rescale_p, datad *d, ggobid *gg) {

  rows_in_plot_set (d, gg);

  if (rescale_p) {
    /*vartable_lim_update (d, gg);*/
    limits_set (true, true, d);  
    vartable_limits_set (d);
    vartable_stats_set (d);
  }

  tform_to_world (d, gg);

/*
  if (gg->is_pp) {
    gg->recalc_max_min = True;
    reset_pp_plot ();
    pp_index (gg, 0,1);
  }
*/

  displays_tailpipe (REDISPLAY_ALL, gg);

  /*-- for each plot?  for each plot in brushing mode? 
   * after the new screen coords are found --*/
  /*assign_points_to_bins ();*/
}

void
subset_include_all (datad *d, ggobid *gg) {
  gint i, rgid;

  for (i=0; i<d->nrows; i++)
    d->sampled[i] = true;

  if (d->nrgroups > 0) {
    rgid = d->rgroup_ids[i];
    d->rgroups[rgid].sampled = true;
  }
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

  gint top = (d->nrgroups > 0) ? d->nrgroups : d->nrows;

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
  gint top = (d->nrgroups > 0) ? d->nrgroups : d->nrows;
  top -= 1;

  b_end = bstart + bsize;
  b_end = (b_end >= top) ? top : b_end;

  if (b_end > 0 && b_end <= top &&
      bstart >= 0 && bstart < top &&
      bstart < b_end)
  {
    subset_clear (d, gg);

    for (i=bstart; i<b_end; i++) {
      add_to_subset (i, d, gg);  /*-- only added included rows --*/
    }

    doneit = true;
  }
  else quick_message ("The limits aren't correctly specified.", false);
 
  return true;
}

gboolean
subset_everyn (gint estart, gint estep, datad *d, ggobid *gg)
{
  gint i;
  gint top = (d->nrgroups > 0) ? d->nrgroups : d->nrows;

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
  gint top = (d->nrgroups > 0) ? d->nrgroups : d->nrows;


  if (g_slist_length (d->sticky_ids) > 0) {

    subset_clear (d, gg);

    for (l = d->sticky_ids; l; l = l->next) {
      id = GPOINTER_TO_INT (l->data);
      if (id < top)
        add_to_subset ((d->nrgroups > 0) ? d->rgroup_ids[id] : id, d, gg);
    }
  }

  return true;
}

gboolean
subset_rowlab (gchar *rowlab, datad *d, ggobid *gg)
{
  gint i;
  gint top = (d->nrgroups > 0) ? d->nrgroups : d->nrows;

  subset_clear (d, gg);

  for (i=0; i<top; i++) {
    if (!strcmp ((gchar *) g_array_index (d->rowlab, gchar *, i), rowlab)) {
      add_to_subset ((d->nrgroups > 0) ? d->rgroup_ids[i] : i, d, gg);
    }
  }

  return true;
}
