/* vardata.c */

#include <stdlib.h>
#include <math.h>

#include <gtk/gtk.h>
#include "vars.h"

void vardata_alloc ()
{
  if (xg.vardata != NULL)
    g_free ((gpointer) xg.vardata);

  xg.vardata = (vardatad *) g_malloc (xg.ncols * sizeof (vardatad));
}

void vardata_realloc (gint n)
{
  xg.vardata = (vardatad *) g_realloc ((gpointer) xg.vardata,
    n * sizeof (vardatad));
}

extern gfloat no_change (gfloat);

void vardata_init ()
{
  gint j;
  for (j=0; j<xg.ncols; j++) {
    xg.vardata[j].selected = false;
    xg.vardata[j].nmissing = 0;
    xg.vardata[j].mean = 0.0;
    xg.vardata[j].median = 0.0;
    xg.vardata[j].lim_raw.min = 0.0;
    xg.vardata[j].lim_raw.max = 0.0;
    xg.vardata[j].lim_tform.min = 0.0;
    xg.vardata[j].lim_tform.max = 0.0;

    xg.vardata[j].domain_incr = 0.;
    xg.vardata[j].domain_adj = no_change;
    xg.vardata[j].inv_domain_adj = no_change;
    xg.vardata[j].tform1 = NO_TFORM1;
    xg.vardata[j].param = 0.;
	xg.vardata[j].tform2 = NO_TFORM2;

    xg.vardata[j].jitter_factor = 0.0;
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
nvgroups ()
{
  gint j, ngr = 0;

  for (j=0; j<xg.ncols; j++)
    if (xg.vardata[j].groupid > ngr)
      ngr = xg.vardata[j].groupid;

  return (ngr+1);
}

void
vgroups_sort () {
  gint maxid, id, newid, j;
  gboolean found;

  /*
   * Find maximum vgroup id.
  */
  maxid = 0;
  for (j=0; j<xg.ncols; j++) {
    if (xg.vardata[j].groupid > maxid)
      maxid = xg.vardata[j].groupid;
  }

  /*
   * Find minimum vgroup id, set it to 0.  Find next, set it to 1; etc.
  */
  id = 0;
  newid = -1;
  while (id <= maxid) {
    found = false;
    for (j=0; j<xg.ncols; j++) {
      if (xg.vardata[j].groupid == id) {
        newid++;
        found = true;
        break;
      }
    }
    if (found)
      for (j=0; j<xg.ncols; j++)
        if (xg.vardata[j].groupid == id)
          xg.vardata[j].groupid = newid;
    id++;
  }
}



/*-------------------------------------------------------------------------*/
/*                 finding the statistics for the table                    */
/*-------------------------------------------------------------------------*/

void
vardata_stats_print () {
  gint j;
  for (j=0; j<xg.ncols; j++) {
    g_printerr ("mean=%f, median=%f\n",
      xg.vardata[j].mean, xg.vardata[j].median);
    g_printerr ("lims: %7.2f %7.2f %7.2f %7.2f\n",
      xg.vardata[j].lim_raw.min, xg.vardata[j].lim_raw.max,
      xg.vardata[j].lim_tform.min, xg.vardata[j].lim_tform.max);
  }
}

void
vardata_stats_set () {
  gint j, i, np;
  gfloat *sumv = (gfloat *) g_malloc0 (xg.ncols * sizeof (gfloat));
  gfloat *x = (gfloat *) g_malloc (xg.nrows * sizeof (gfloat));
  extern gint fcompare (const void *, const void *);

  /*
   * this could be done with less code, but this 
   * minimizes looping and function calls, and should
   * be as as fast as we can make it
  */

  for (j=0; j<xg.ncols; j++) {
    xg.vardata[j].lim_raw.min = xg.raw.data[0][j];
    xg.vardata[j].lim_raw.max = xg.raw.data[0][j];
    xg.vardata[j].lim_tform.min = xg.tform2.data[0][j];
    xg.vardata[j].lim_tform.max = xg.tform2.data[0][j];
  }

  for (j=0; j<xg.ncols; j++) {
    np = 0;
    for (i=0; i<xg.nrows; i++) {
      if (xg.nmissing > 0 && MISSING_P(i,j))
        ;
      else {

        if (xg.raw.data[i][j] < xg.vardata[j].lim_raw.min)
          xg.vardata[j].lim_raw.min = xg.raw.data[i][j];
        else if (xg.raw.data[i][j] > xg.vardata[j].lim_raw.max)
          xg.vardata[j].lim_raw.max = xg.raw.data[i][j];

        if (xg.tform2.data[i][j] < xg.vardata[j].lim_tform.min)
          xg.vardata[j].lim_tform.min = xg.tform2.data[i][j];
        else if (xg.tform2.data[i][j] > xg.vardata[j].lim_tform.max)
          xg.vardata[j].lim_tform.max = xg.tform2.data[i][j];

        sumv[j] += xg.raw.data[i][j];
        x[np] = xg.raw.data[i][j];
        np++;
      }
    }
    xg.vardata[j].mean = sumv[j] / (gfloat) xg.nrows;

    /*-- median: sort the temporary vector, and find its center --*/
    qsort((void *) x, np, sizeof (gfloat), fcompare);
    xg.vardata[j].median = 
      ((np % 2) != 0) ?  x[(np-1)/2] : (x[np/2-1] + x[np/2])/2. ;
  }

  g_free ((gpointer) sumv);
  g_free ((gpointer) x);
}

