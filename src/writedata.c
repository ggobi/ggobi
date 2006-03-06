/*-- writedata.c --*/
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

#include <string.h>
#include <gtk/gtk.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
/*
#include <limits.h>
#include <float.h>
*/

#include "vars.h"
#include "externs.h"
#include "writedata.h"

#ifdef SAVE_UNTIL_WRITE_CSV_IMPLEMENTED
static gboolean write_ascii_data (gchar *, gint *, gint, gint *, gint, GGobiData *, ggobid *);
#endif

static gint
set_rowv (gint *rowv, gchar *rootname, GGobiData *d, ggobid *gg)
{
  gint i, j, k;
  gint nrows = 0;
  GSList *l;

  switch (gg->save.row_ind) {

    case DISPLAYEDROWS:
    /*
     * Otherwise just copy the row numbers representing unerased
     * points into rowv, and return their count.
    */
      for (i=0, j=0; i<d->nrows_in_plot; i++) {
        k = d->rows_in_plot.els[i];
        if (!d->hidden_now.els[k])
          rowv[j++] = k;
      }
      nrows = j;
      break;

    case LABELLEDROWS:
      /*
       * Otherwise just copy the row numbers representing sticky
       * labels into rowv, and return their count.
      */
      for (l = d->sticky_ids; l; l = l->next)
        rowv[i] = GPOINTER_TO_INT (l->data);
      nrows = g_slist_length (d->sticky_ids);
      break;

    case ALLROWS:
    /* 
     * Finally, let rowv be (0,1,2,,,,d->nrows)
    */
      for (i=0; i<d->nrows; i++)
        rowv[i] = i;
      nrows = d->nrows;
      break;

    default:
      fprintf (stderr, "error in row_ind; impossible type %d\n",
        gg->save.row_ind);
      break;
  }

  return (nrows);
}

static gint
set_colv (gint *colv, gchar *rootname, GGobiData *d, ggobid *gg)
{
  gint i;
  gint ncols = 0;

  switch (gg->save.column_ind) {

    case ALLCOLS:
      /* 
       * let colv be (0,1,2,,,,gg->ncols)
      */
      for (i=0; i<d->ncols; i++)
        colv[i] = i;
      ncols = d->ncols;
      break;

    case SELECTEDCOLS:
      ncols = selected_cols_get (colv, d, gg);
      fprintf(stderr, "%d\n", ncols);
      if (ncols == 0)
        ncols = plotted_cols_get (colv, d, gg);
      break;
    

    default:
      fprintf (stderr, "error in col_ind; impossible type %d\n",
        gg->save.column_ind);
      break;
  }
  
  return (ncols);
}

gboolean
write_csv (gchar *rootname, gint *rowv, gint nr, gint *colv, gint nc,
  GGobiData *d, ggobid *gg)
{
  gchar fname[164];
  gchar *message;
  FILE *fp;
  gint i, j, ir, jc;
  gfloat **fdatap;

  sprintf (fname, "%s.csv", rootname);

  if ((fp = fopen (fname, "w")) == NULL) {
    message = g_strdup_printf ("The file '%s' can not be created\n", fname);
    quick_message (message, false);
    g_free (message);
    return false;
  } else {
    fdatap = (gg->save.stage == RAWDATA) ? d->raw.vals : d->tform.vals;

    for (i=0; i<nr; i++) {
      ir = rowv[i];
      for (j=0; j<nc; j++) {
        jc = colv[j];
        if (d->nmissing > 0 && d->missing.vals[ir][jc]) {
          if (gg->save.missing_ind == MISSINGSNA) {
            fprintf (fp, "NA ");
          }  else if (gg->save.missing_ind == MISSINGSDOT) {
            fprintf (fp, ". ");
          } 
        } 
        else {
          fprintf (fp, "%g ", fdatap[ir][jc]);
        } 
      }
      fprintf (fp, "\n");
    }

    fclose(fp);
    return true;
  }
}

gboolean
ggobi_file_set_create (gchar *rootname, GGobiData *d, ggobid *gg)
{
  gint nr, nc;
  gint *rowv, *colv;
  gint i;
  gboolean skipit;

  if (d == NULL)
    d = (GGobiData *) g_slist_nth_data(gg->d, 0);

/* Determine the rows to be saved */
  rowv = (gint *) g_malloc (d->nrows * sizeof (gint));
  nr = set_rowv (rowv, rootname, d, gg);
  if (nr == 0) {
    gchar *message = g_strdup_printf (
      "You have not successfully specified any rows; sorry");
    quick_message (message, false);
    g_free (message);
    g_free ((gchar *) rowv);
    return false;
  }

/* Determine the columns to be saved */
  colv = (gint *) g_malloc (d->ncols * sizeof (gint));
  nc = set_colv (colv, rootname, d, gg);
  if (nc == 0) {
    gchar *message = g_strdup_printf (
      "You have not successfully specified any columns; sorry");
    quick_message (message, false);
    g_free (message);
    g_free ((gchar *) rowv);
    g_free ((gchar *) colv);
    return false;
  }

  /*
   * Save .dat first:  ascii_data or binary_data, raw or tform, missings as
   * 'na' or as currently imputed values
  */
  if (gg->save.format == BINARYDATA) { 
    if (write_binary_data (rootname, rowv, nr, colv, nc, d, gg) == 0) {
      g_free ((gchar *) rowv);
      g_free ((gchar *) colv);
      return false;
    }
  } else {
    if (write_ascii_data (rootname, rowv, nr, colv, nc, d, gg) == 0) {
      g_free ((gchar *) rowv);
      g_free ((gchar *) colv);
      return false;
    }
  }

/*
 * Continue saving files: .doc?
 * Don't bother with .missing
*/

  g_free ((char *) rowv);
  g_free ((char *) colv);

  return true;
}




