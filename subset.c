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

static void
add_to_subset (gint i) {

  if (xg.nrgroups > 0) {
    gint j, el;
    xg.rgroups[i].excluded = false;
    for (j=0; j<xg.rgroups[i].nels; j++) {
      el = xg.rgroups[i].els[j];
      xg.erased[el] = xg.excluded[el] = false;
      xg.rows_in_plot[xg.nrows_in_plot++] = el;
    }

  } else {
    xg.erased[i] = xg.excluded[i] = false;
    xg.rows_in_plot[xg.nrows_in_plot++] = i;
  }
}

/*-- effectively remove everything from the current subset --*/
static void
subset_clear () {
  gint i, rgid;
  gint top = (xg.nrgroups > 0) ? xg.nrgroups : xg.nlinkable;

  xg.nrows_in_plot = 0;

  for (i=0; i<top; i++) {
    if (xg.nrgroups > 0) {
      rgid = xg.rgroup_ids[i];
      xg.rgroups[rgid].hidden = true;
    } else
      xg.erased[i] = true;
  }
}

/*------------------------------------------------------------------*/

/*
 * Much of this is code that will have to be executed whenever
 * rows_in_plot changes, but that's only happening in subsetting for now
*/
void
subset_apply (gboolean rescale_p) {
  gint i;

  /*-- add all the unlinkable guys to the sample --*/
  for (i=xg.nlinkable; i<xg.nrows; i++)
    xg.rows_in_plot[xg.nrows_in_plot++] = i;

  /* 
   * Make sure the value of nlinkable_in_plot corresponds to
   * nrows_in_plot 
  xg.nlinkable_in_plot = 0;
  for (i=0; i<xg.nlinkable; i++)
    if (!xg.excluded[i])
      xg.nlinkable_in_plot++;
  */

  if (rescale_p)
    vardata_lim_update ();  /*-- ?? --*/

  tform_to_world ();

/*
  if (xg.is_pp) {
    xg.recalc_max_min = True;
    reset_pp_plot();
    pp_index(xg,0,1);
  }
*/

  displays_tailpipe (REDISPLAY_ALL);

  /*-- for each plot?  for each plot in brushing mode? 
   * after the new screen coords are found --*/
  /*assign_points_to_bins ();*/
}

void
subset_include_all () {
  gint i, rgid;

  xg.nrows_in_plot = xg.nlinkable;

  for (i=0; i<xg.nlinkable; i++) {
    xg.erased[i] = xg.excluded[i] = false;
    xg.rows_in_plot[i] = i;
  }

  if (xg.nrgroups > 0) {
    rgid = xg.rgroup_ids[i];
    xg.rgroups[rgid].hidden = xg.rgroups[rgid].excluded = false;
  }
}

/*
 * This algorithm taken from Knuth, Seminumerical Algorithms;
 * Vol 2 of his series.
*/
gboolean
subset_random (gint n) {
  gint t, m;
  gboolean doneit = false;
  gfloat rrand;
  gint top = (xg.nrgroups > 0) ? xg.nrgroups : xg.nlinkable;

  subset_clear ();

  if (n > 0 && n < top) {

    for (t=0, m=0; t<top && m<n; t++)
    {
      rrand = (gfloat) randvalue();
      if ( ((top - t) * rrand) < (n - m) ) {
        add_to_subset (t);
        m++;
      }
    }

    doneit = true;
  }

  return (doneit);
}

gboolean
subset_block (gint bstart, gint bsize)
{
  gint i, b_end;
  gboolean doneit = false;
  gint top = (xg.nrgroups > 0) ? xg.nrgroups : xg.nlinkable;
  top -= 1;

  b_end = bstart + bsize;
  b_end = (b_end >= top) ? top : b_end;

  if (b_end > 0 && b_end <= top &&
      bstart >= 0 && bstart < top &&
      bstart < b_end)
  {
    subset_clear ();

    for (i=bstart; i<b_end; i++)
      add_to_subset (i);

    doneit = true;
  }
  else quick_message ("The limits aren't correctly specified.", false);
 
  return true;
}

gboolean
subset_everyn (gint estart, gint estep)
{
  gint i;
  gint top = (xg.nrgroups > 0) ? xg.nrgroups : xg.nlinkable;
  gboolean doneit = false;

  top -= 1;
  if (estart >= 0 && estart < top-1 && estep >= 0 && estep < top)
  {
    subset_clear ();

    for (i=estart; i<top; i+=estep)
      add_to_subset (i);

    doneit = true;

  } else quick_message ("Interval not correctly specified.", false);

  return doneit;
}

/*-- create a subset of only the points with sticky ids --*/
/*-- Added by James Brook, Oct 1994 --*/
gboolean
subset_sticky ()
{
  gint id;
  GSList *l;
  gint top = (xg.nrgroups > 0) ? xg.nrgroups : xg.nlinkable;

  if (g_slist_length (xg.sticky_ids) > 0) {

    subset_clear ();

    for (l = xg.sticky_ids; l; l = l->next) {
      id = GPOINTER_TO_INT (l->data);
      if (id < top)
        add_to_subset ((xg.nrgroups > 0) ? xg.rgroup_ids[id] : id);
    }
  }

  return true;
}

gboolean
subset_rowlab (gchar *rowlab)
{
  gint i;
  gint top = (xg.nrgroups > 0) ? xg.nrgroups : xg.nlinkable;

  subset_clear ();

  for (i=0; i<top; i++) {
    if (!strcmp (xg.rowlab[i], rowlab)) {
      add_to_subset ((xg.nrgroups > 0) ? xg.rgroup_ids[i] : i);
    }
  }

  return true;
}
