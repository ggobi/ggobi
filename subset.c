/* subset.c */

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
add_to_subset (gint i, ggobid *gg) {
  gint j, el;
  gboolean added = false;

  if (gg->nrgroups > 0) {
    if (gg->rgroups[i].included) {
      added = true;
      gg->rgroups[i].sampled = true;
      for (j=0; j<gg->rgroups[i].nels; j++) {
        el = gg->rgroups[i].els[j];
        gg->sampled[el] = true;
      }
    }

  } else {
    if (gg->included[i] == true) {
      added = true;
      gg->sampled[i] = true;
    }
  }

  return added;
}

/*-- remove everything from the subset before constructing a new one --*/
static void
subset_clear (ggobid *gg) {
  gint i, rgid;

  for (i=0; i<gg->nrows; i++)
    gg->sampled[i] = false;

  for (i=0; i<gg->nrgroups; i++) {
    rgid = gg->rgroup_ids[i];
    gg->rgroups[rgid].sampled = false;
  }
}

/*------------------------------------------------------------------*/

void
subset_apply (gboolean rescale_p, ggobid *gg) {

  rows_in_plot_set (gg);

  if (rescale_p)
    vardata_lim_update ();  /*-- ?? --*/

  tform_to_world ();

/*
  if (gg->is_pp) {
    gg->recalc_max_min = True;
    reset_pp_plot ();
    pp_index (gg, 0,1);
  }
*/

  displays_tailpipe (REDISPLAY_ALL);

  /*-- for each plot?  for each plot in brushing mode? 
   * after the new screen coords are found --*/
  /*assign_points_to_bins ();*/
}

void
subset_include_all (ggobid *gg) {
  gint i, rgid;

  for (i=0; i<gg->nrows; i++)
    gg->sampled[i] = true;

  if (gg->nrgroups > 0) {
    rgid = gg->rgroup_ids[i];
    gg->rgroups[rgid].sampled = true;
  }
}

/*
 * This algorithm taken from Knuth, Seminumerical Algorithms;
 * Vol 2 of his series.
*/
gboolean
subset_random (gint n, ggobid *gg) {
  gint t, m;
  gboolean doneit = false;
  gfloat rrand;
  gint top = (gg->nrgroups > 0) ? gg->nrgroups : gg->nrows;

  subset_clear (gg);

  if (n > 0 && n < top) {

    for (t=0, m=0; t<top && m<n; t++)
    {
      rrand = (gfloat) randvalue();
      if (((top - t) * rrand) < (n - m)) {
        if (add_to_subset (t, gg))
          m++;
      }
    }

    doneit = true;
  }

  return (doneit);
}

gboolean
subset_block (gint bstart, gint bsize, ggobid *gg)
{
  gint i, b_end;
  gboolean doneit = false;
  gint top = (gg->nrgroups > 0) ? gg->nrgroups : gg->nrows;
  top -= 1;

  b_end = bstart + bsize;
  b_end = (b_end >= top) ? top : b_end;

  if (b_end > 0 && b_end <= top &&
      bstart >= 0 && bstart < top &&
      bstart < b_end)
  {
    subset_clear (gg);

    for (i=bstart; i<b_end; i++) {
      add_to_subset (i, gg);  /*-- only added included rows --*/
    }

    doneit = true;
  }
  else quick_message ("The limits aren't correctly specified.", false);
 
  return true;
}

gboolean
subset_everyn (gint estart, gint estep, ggobid *gg)
{
  gint i;
  gint top = (gg->nrgroups > 0) ? gg->nrgroups : gg->nrows;
  gboolean doneit = false;

  top -= 1;
  if (estart >= 0 && estart < top-1 && estep >= 0 && estep < top)
  {
    subset_clear (gg);

    i = estart;
    while (i < top) {
      if (add_to_subset (i, gg))
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
subset_sticky (ggobid *gg)
{
  gint id;
  GSList *l;
  gint top = (gg->nrgroups > 0) ? gg->nrgroups : gg->nrows;

  if (g_slist_length (gg->sticky_ids) > 0) {

    subset_clear (gg);

    for (l = gg->sticky_ids; l; l = l->next) {
      id = GPOINTER_TO_INT (l->data);
      if (id < top)
        add_to_subset ((gg->nrgroups > 0) ? gg->rgroup_ids[id] : id, gg);
    }
  }

  return true;
}

gboolean
subset_rowlab (gchar *rowlab, ggobid *gg)
{
  gint i;
  gint top = (gg->nrgroups > 0) ? gg->nrgroups : gg->nrows;

  subset_clear (gg);

  for (i=0; i<top; i++) {
    if (!strcmp (gg->rowlab[i], rowlab)) {
      add_to_subset ((gg->nrgroups > 0) ? gg->rgroup_ids[i] : i, gg);
    }
  }

  return true;
}
