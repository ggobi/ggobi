/*-- writedata.c --*/
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

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <float.h>

#include "vars.h"
#include "externs.h"
#include "writedata.h"

gboolean write_binary_data (gchar *, gint *, gint, gint *, gint, datad *, ggobid *);
gboolean write_ascii_data (gchar *, gint *, gint, gint *, gint, datad *, ggobid *);
gboolean save_collabels (gchar *, gint *colv, gint nc, datad *, ggobid *);
gboolean save_rowlabels (gchar *, gint *rowv, gint nr, datad *, ggobid *);
gboolean brush_save_colors (gchar *, gint *, gint, datad *, ggobid *);
gboolean brush_save_erase (gchar *, gint *, gint, datad *, ggobid *);
gboolean brush_save_glyphs (gchar *, gint *, gint, datad *, ggobid *);
gboolean save_lines (gchar *, gboolean, gboolean, gint *, gint, datad *d, ggobid *);
gint linedata_get (endpointsd *, gshort *, gint *, gint, datad *, ggobid *);

static gint
set_rowv (gint *rowv, gchar *rootname, datad *d, ggobid *gg)
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
        k = d->rows_in_plot[i];
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
set_colv (gint *colv, gchar *rootname, datad *d, ggobid *gg)
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
write_ascii_data (gchar *rootname, gint *rowv, gint nr, gint *colv, gint nc,
  datad *d, ggobid *gg)
{
  gchar fname[164];
  gchar *message;
  FILE *fp;
  gint i, j, ir, jc;
  gfloat **fdatap;

  if (gg->save.stage == RAWDATA || gg->save.stage == TFORMDATA)
    sprintf (fname, "%s.dat", rootname);

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

void
strip_blanks (gchar *str)
{
  gint src, dest;

  for (src=0, dest=0; src<(gint)(strlen(str)+1); src++)
    if (str[src] != ' ')
      str[dest++] = str[src];
}

gboolean
ggobi_file_set_create (gchar *rootname, datad *d, ggobid *gg)
{
  gint nr, nc;
  gint *rowv, *colv;
  gchar *fname;
  FILE *fp;
  gint i, j;
  gboolean skipit;

  /*
   * An inconsistency:  Can't save binary data and still
   * write out "na"
  */
  if (gg->save.format == BINARYDATA &&
      d->nmissing > 0 &&
        (gg->save.missing_ind == MISSINGSNA ||
         gg->save.missing_ind == MISSINGSDOT))
  {
    gchar *message = g_strdup_printf (
      "Sorry, GGobi can't write 'NA' or '.' in binary format.");
    quick_message (message, false);
    g_free (message);
    return false;
  }


/* Step 1: verify that the rootname is writable */

/*
  if ((fp = fopen (rootname, "w")) == NULL) {
    gchar *message = g_strdup_printf (
      "The file '%s' can not be opened for writing\n", rootname);
    quick_message (message, false);
    g_free (message);
    return false;
  } else {
    fclose (fp);
  }
*/

  if (d == NULL)
    d = (datad *) g_slist_nth_data(gg->d, 0);

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

/* Save column labels */
  if (!save_collabels (rootname, colv, nc, d, gg)) {
    g_free ((gchar *) rowv);
    g_free ((gchar *) colv);
    return false;
  }

/* Save row labels */
  if (!save_rowlabels (rootname, rowv, nr, d, gg)) {
    g_free ((gchar *) rowv);
    g_free ((gchar *) colv);
    return false;
  }

/* Save colors */
  skipit = true;
  /*-- if no color differs from the default color, don't save colors --*/
  for (i=0; i<nr; i++) {
    if (d->color_now.els[rowv[i]] != 0) {
      skipit = false;
      break;
    }
  }
  if (!skipit) {
    if (!brush_save_colors (rootname, rowv, nr, d, gg)) {
      g_free ((gchar *) rowv);
      g_free ((gchar *) colv);
      return false;
    }
  }

/* Save glyphs */
  skipit = true;
  /*-- if no glyph differs from the default, don't save glyphs --*/
  for (i=0; i<nr; i++) {
    if (d->glyph_now[rowv[i]].type != gg->glyph_0.type ||
        d->glyph_now[rowv[i]].size != gg->glyph_0.size)
    {
      skipit = false;
      break;
    }
  }
  if (!skipit) {
    if (!brush_save_glyphs (rootname, rowv, nr, d, gg)) {
      g_free ((gchar *) rowv);
      g_free ((gchar *) colv);
      return false;
    }
  }

/* Save erase -- unless using erase vector to choose what to copy */
  if (gg->save.row_ind != DISPLAYEDROWS) {
    skipit = true;
    /*-- if nothing is erased, skip it --*/
    for (i=0; i<nr; i++) {
      if (d->hidden.els[rowv[i]] == 1) {
        skipit = false;
        break;
      }
    }
    if (!skipit) {
      if (!brush_save_erase (rootname, rowv, nr, d, gg)) {
        g_free ((gchar *) rowv);
        g_free ((gchar *) colv);
        return false;
      }
    }
  }

/* Save rgroups */
  if (d->nrgroups > 0) {
    sprintf (fname, "%s.rgroups", rootname);
    if ( (fp = fopen (fname, "w")) == NULL) {
      gchar *message = g_strdup_printf (
        "The file '%s' can not be opened for writing\n", fname);
      quick_message (message, false);
      g_free (message);
      g_free ((gchar *) rowv);
      g_free ((gchar *) colv);
      return false;
    } else {
      for (j=0; j<nr; j++)
        fprintf (fp, "%d ", d->rgroup_ids[rowv[j]] + 1);
      fprintf (fp, "\n");
      fclose (fp);
    }
  }

/*
 * Continue saving files: .doc?
 * Don't bother with .missing, .nlinkable
*/

  g_free ((char *) rowv);
  g_free ((char *) colv);

  return true;
}

/*
 * Third section: routines that will be used by both the
 * first and the second sections to save individual files.
*/

gboolean
write_binary_data (gchar *rootname, gint *rowv, gint nr, gint *colv, gint nc,
  datad *d, ggobid *gg)
{
  gchar *fname;
  FILE *fp;
  gint i, j, ir, jc;
  gfloat xfoo;
  gfloat **datap;

  if (rowv == (gint *) NULL) {
    rowv = (gint *) g_malloc (nr * sizeof(gint));
    for (i=0; i<nr; i++)
      rowv[i] = i;
  }

  fname = g_strdup_printf ("%s.bin", rootname);
  fp = fopen (fname, "w");
  g_free (fname);

  if (fp == NULL) {
    gchar *message = g_strdup_printf (
      "The file '%s.bin' can not be created\n", rootname);
    quick_message (message, false);
    g_free (message);
    return false;
  } else {

    /* First the number of rows and columns */
    fwrite ((gchar *) &nr, sizeof (nr), 1, fp);
    fwrite ((gchar *) &nc, sizeof (nc), 1, fp);

    datap = (gg->save.stage == RAWDATA) ? d->raw.vals : d->tform.vals;

    for (i=0; i<nr; i++) {
      ir = rowv[i];
      for (j=0; j<nc; j++)
      {
        if (colv == (gint *) NULL)  /* Write all columns, in default order */
          jc = j;
        else
          jc = colv[j];  /* Write the columns as specified */
        if (d->nmissing > 0 && d->missing.vals[i][j])
          xfoo = FLT_MAX;
        else
          xfoo = datap[ir][jc];
        fwrite ((gchar *) &xfoo, sizeof (xfoo), 1, fp);
      }
    }

    fclose (fp);
    return (true);
  }
}

/*
 * save limits if they exist
*/
gboolean
save_collabels (gchar *rootname, gint *colv, gint nc, datad *d, ggobid *gg)
{
  gint j;
  FILE *fp;
  gchar *fname;

  fname = g_strdup_printf ("%s.col", rootname);
  fp = fopen (fname, "w");
  g_free (fname);

  if (fp == NULL) {
    gchar *message = g_strdup_printf (
      "Failed to open %s.col for writing.\n", rootname);
    quick_message (message, false);
    g_free (message);
    return false;
  }
  else {
    if (gg->save.stage == RAWDATA) {
      for (j=0; j<nc; j++)
        fprintf (fp, "%s\n", d->vartable[colv[j]].collab);
    } else {  /*-- TFORMDATA --*/
      for (j=0; j<nc; j++)
        fprintf (fp, "%s\n", d->vartable[colv[j]].collab_tform);
    }
    fclose (fp);
    return true;
  }
}

gboolean
save_rowlabels (gchar *rootname, gint *rowv, gint nr, datad *d, ggobid *gg)
{
  gint i;
  FILE *fp;
  gchar *fname;
  gint nels = d->rowlab->len;

  if (nels > 0) {
    fname = g_strdup_printf ("%s.row", rootname);
    fp = fopen (fname, "w");
    g_free (fname);

    if (fp == NULL) {
      gchar *message = g_strdup_printf ("Failed to open %s.row for writing.\n",
        rootname);
      quick_message (message, false);
      g_free (message);
      return false;
    }
    else
    {
      for (i=0; i<nr; i++) {
        fprintf (fp, "%s\n", 
          (gchar *) g_array_index (d->rowlab, gchar *, rowv[i]));
      }
      fclose(fp);
    }
  }
  return true;
}

gboolean
brush_save_colors (gchar *rootname, gint *rowv, gint nr, datad *d, ggobid *gg)
{
  gchar *fname;
  gint i;
  FILE *fp;

  if (gg->mono_p)
    return true;

  fname = g_strdup_printf ("%s.colors", rootname);
  fp = fopen (fname, "w");
  g_free (fname);

  if (fp == NULL)
  {
    gchar *message = g_strdup_printf (
      "The file '%s.colors' can't be opened for writing\n", rootname);
    quick_message (message, false);
    g_free (message);
    return (false);
  }
  else
  {
    for (i=0; i<nr; i++)
      fprintf (fp, "%d\n", d->color_now.els[rowv[i]]);

    if (fclose (fp) == EOF)
      fprintf(stderr, "error in writing color vector\n");
  }

  return (true);
}

gboolean
brush_save_glyphs (gchar *rootname, gint *rowv, gint nr, datad *d, ggobid *gg)
{
  gchar *fname;
  gint i;
  FILE *fp;
  gchar *gstr;

  fname = g_strdup_printf ("%s.glyphs", rootname);
  fp = fopen (fname, "w");
  g_free (fname);

  if (fp == NULL)
  {
    gchar *message = g_strdup_printf (
      "The file '%s.colors' can't be opened for writing\n", rootname);
    quick_message (message, false);
    g_free (message);
    g_free (fname);
    return false;

  } else {

    for (i=0; i<nr; i++) {
      switch (d->glyph[i].type) {
        case PLUS_GLYPH:
/*          gstr = "+";*/
          gstr = "plus";
          break;
        case X_GLYPH:
          gstr = "x";
          break;
        case OPEN_RECTANGLE:
          gstr = "or";
          break;
        case FILLED_RECTANGLE:
          gstr = "fr";
          break;
        case OPEN_CIRCLE:
          gstr = "oc";
          break;
        case FILLED_CIRCLE:
          gstr = "fc";
          break;
        case POINT_GLYPH:
          gstr = ".";
          break;
      }

      fprintf (fp, "%s %d\n", gstr, d->glyph[i].size);
    }
    if (fclose (fp) == EOF)
      fprintf (stderr, "error in writing glyphs vector\n");
    return true;
  }
}

gboolean
brush_save_erase (gchar *rootname, gint *rowv, gint nr, datad *d, ggobid *gg)
{
  gchar *fname;
  gint i;
  FILE *fp;

  fname = g_strdup_printf ("%s.erase", rootname);
  fp = fopen (fname, "w");
  g_free (fname);

  if (fp == NULL) {
    gchar *message = g_strdup_printf (
      "The file '%s.erase' can't be opened for writing\n", rootname);
    quick_message (message, false);
    g_free (message);
    return false;
  }

  for (i=0; i<nr; i++)
    fprintf(fp, "%ld\n", (glong) d->hidden.els[rowv[i]]);

  fclose(fp);
  return true;
}

/*------------------------------------------------------------------------*/

gboolean
save_missing (gchar *rootname, gint *rowv, gint nr, gint *colv, gint nc,
  datad *d, ggobid *gg)
{
  gint i;
  gchar *fname;
  gboolean success = true;
  FILE *fp;

  if (rowv == (gint *) NULL) {
    rowv = (gint *) g_malloc (nr * sizeof (gint));
    for (i=0; i<nr; i++)
      rowv[i] = i;
  }

  fname = g_strdup_printf ("%s.missing", rootname);
  fp = fopen (fname, "w");
  g_free (fname);

  if (fp == NULL) {
    gchar *message = g_strdup_printf (
      "Problem writing out the missing file, %s\n", fname);
    quick_message (message, false);
    g_free (message);
    return false;
  }
  else
  {
    gint i, j, jc, m;
    for (i=0; i<nr; i++)
    {
      m = rowv[i];

      for (j=0; j<nc; j++) {
        if (colv == (gint *) NULL)  /* Write all columns, in default order */
          jc = j;
        else
          jc = colv[j];  /* Write the columns as specified */
          fprintf (fp, "%d ", d->missing.vals[m][jc]);
      }
      fprintf (fp, "\n");
    }
    fclose (fp);
  }

  return (success);
}

