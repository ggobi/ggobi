/* vardata.c */

#include <stdlib.h>
#include <math.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

#ifdef __cplusplus
extern "C" {
#endif
extern gfloat no_change (gfloat);
#ifdef __cplusplus
}
#endif

void vardata_alloc (datad *d, ggobid *gg)
{
  if (d->vardata != NULL)
    g_free ((gpointer) d->vardata);

  d->vardata = (vardatad *) g_malloc (d->ncols * sizeof (vardatad));
}

void vardata_realloc (gint n, datad *d, ggobid *gg)
{
  d->vardata = (vardatad *) g_realloc ((gpointer) d->vardata,
    n * sizeof (vardatad));
}


void vardata_init (datad *d, ggobid *gg)
{
  gint j;
  for (j=0; j<d->ncols; j++) {
    d->vardata[j].selected = false;
    d->vardata[j].nmissing = 0;
    d->vardata[j].mean = 0.0;
    d->vardata[j].median = 0.0;
    d->vardata[j].lim_raw.min = 0.0;
    d->vardata[j].lim_raw.max = 0.0;
    d->vardata[j].lim_tform.min = 0.0;
    d->vardata[j].lim_tform.max = 0.0;

    d->vardata[j].domain_incr = 0.;
    d->vardata[j].domain_adj = no_change;
    d->vardata[j].inv_domain_adj = no_change;
    d->vardata[j].tform1 = NO_TFORM1;
    d->vardata[j].param = 0.;
	d->vardata[j].tform2 = NO_TFORM2;

    d->vardata[j].jitter_factor = 0.0;

    d->vardata[j].collab = NULL;
    d->vardata[j].collab_tform = NULL;
    d->vardata[j].groupid = 0;
  }
}

/*-------------------------------------------------------------------------*/
/*                 variable groups                                         */
/*-------------------------------------------------------------------------*/

/*
 * This assumes that the values of variable group ids
 * are {0,1,2,...,nvgroups-1}
*/
gint
nvgroups (datad *d, ggobid *gg)
{
  gint j, ngr = 0;

  for (j=0; j<d->ncols; j++)
    if (d->vardata[j].groupid > ngr)
      ngr = d->vardata[j].groupid;

  return (ngr+1);
}

void
vgroups_sort (datad *d, ggobid *gg) 
{
  gint maxid, id, newid, j;
  gboolean found;

  /*
   * Find maximum vgroup id.
  */
  maxid = 0;
  for (j=0; j<d->ncols; j++) {
    if (d->vardata[j].groupid > maxid)
      maxid = d->vardata[j].groupid;
  }

  /*
   * Find minimum vgroup id, set it to 0.  Find next, set it to 1; etc.
  */
  id = 0;
  newid = -1;
  while (id <= maxid) {
    found = false;
    for (j=0; j<d->ncols; j++) {
      if (d->vardata[j].groupid == id) {
        newid++;
        found = true;
        break;
      }
    }
    if (found)
      for (j=0; j<d->ncols; j++)
        if (d->vardata[j].groupid == id)
          d->vardata[j].groupid = newid;
    id++;
  }
}



/*-------------------------------------------------------------------------*/
/*                 finding the statistics for the table                    */
/*-------------------------------------------------------------------------*/

void
vardata_stats_print (datad *d, ggobid *gg) 
{
  gint j;
  for (j=0; j<d->ncols; j++) {
    g_printerr ("mean=%f, median=%f\n",
      d->vardata[j].mean, d->vardata[j].median);
    g_printerr ("lims: %7.2f %7.2f %7.2f %7.2f\n",
      d->vardata[j].lim_raw.min, d->vardata[j].lim_raw.max,
      d->vardata[j].lim_tform.min, d->vardata[j].lim_tform.max);
  }
}

void
vardata_stats_set (datad *d, ggobid *gg) 
{
  gint j, i, m, np;
  gfloat *sumv = (gfloat *) g_malloc0 (d->ncols * sizeof (gfloat));
  gfloat *x = (gfloat *) g_malloc (d->nrows * sizeof (gfloat));

  /*
   * this could be done with less code, but this 
   * minimizes looping and function calls, and should
   * be as as fast as we can make it
  */

  for (j=0; j<d->ncols; j++) {
    d->vardata[j].lim_raw.min = d->raw.vals[0][j];
    d->vardata[j].lim_raw.max = d->raw.vals[0][j];
    d->vardata[j].lim_tform.min = d->tform2.vals[0][j];
    d->vardata[j].lim_tform.max = d->tform2.vals[0][j];
  }

  for (j=0; j<d->ncols; j++) {
    np = 0;
    for (m=0; m<d->nrows_in_plot; m++) {
      i = d->rows_in_plot[m];
      if (d->nmissing > 0 && MISSING_P(i,j))
        ;
      else {

        if (d->raw.vals[i][j] < d->vardata[j].lim_raw.min)
          d->vardata[j].lim_raw.min = d->raw.vals[i][j];
        else if (d->raw.vals[i][j] > d->vardata[j].lim_raw.max)
          d->vardata[j].lim_raw.max = d->raw.vals[i][j];

        if (d->tform2.vals[i][j] < d->vardata[j].lim_tform.min)
          d->vardata[j].lim_tform.min = d->tform2.vals[i][j];
        else if (d->tform2.vals[i][j] > d->vardata[j].lim_tform.max)
          d->vardata[j].lim_tform.max = d->tform2.vals[i][j];

        sumv[j] += d->raw.vals[i][j];
        x[np] = d->raw.vals[i][j];
        np++;
      }
    }
    d->vardata[j].mean = sumv[j] / (gfloat) d->nrows;

    /*-- median: sort the temporary vector, and find its center --*/
    qsort((void *) x, np, sizeof (gfloat), fcompare);
    d->vardata[j].median = 
      ((np % 2) != 0) ?  x[(np-1)/2] : (x[np/2-1] + x[np/2])/2. ;
  }

  g_free ((gpointer) sumv);
  g_free ((gpointer) x);
}

