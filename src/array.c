/*-- array.c --*/
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

#include <gtk/gtk.h>

#include "array.h"

/*-------------------------------------------------------------------------*/
/*                       double array management                           */
/*-------------------------------------------------------------------------*/

void
arrayd_init_null (array_d * arrp)
{
  arrp->nrows = arrp->ncols = 0;
  arrp->vals = (gdouble **) NULL;
}

gdouble ggobi_matrix_get_n_cols(array_d * arrp) {
  return arrp->ncols;
}
gdouble ggobi_matrix_get_n_rows(array_d * arrp) {
  return arrp->nrows;
}


void
arrayd_free (array_d * arrp)
{
  gint nr = 0;
  gint nc = 0;
  gint i;

  /*-- if nr != 0, free only the last nrows-nr rows --*/

  for (i = nr; i < arrp->nrows; i++)
    if (arrp->vals[i] != NULL)
      g_free (arrp->vals[i]);

  if (nr == 0) {
    if (arrp->vals != NULL)
      g_free (arrp->vals);
    arrp->vals = (gdouble **) NULL;
    arrp->nrows = arrp->ncols = 0;
  }
  else {
    arrp->nrows = nr;
    arrp->ncols = nc;
  }
}

/* Zero an array of doubles. */
void
arrayd_zero (array_d * arrp)
{
  int i, j;
  for (i = 0; i < arrp->nrows; i++) {
    for (j = 0; j < arrp->ncols; j++) {
      arrp->vals[i][j] = 0.0;
    }
  }
}

array_d *arrayd_new(gint nr, gint nc) {
  array_d *arrp = g_new0(array_d, 1);
  arrayd_alloc(arrp, nr, nc);
  return arrp;
}

/* allocate an array of doubles */
void
arrayd_alloc (array_d * arrp, gint nr, gint nc)
{
  gint i;

  if ((arrp->nrows != 0) || (arrp->ncols != 0))
    arrayd_free (arrp); 

  arrp->vals = (gdouble **) g_malloc (nr * sizeof (gdouble *));
  for (i = 0; i < nr; i++)
    arrp->vals[i] = (gdouble *) g_malloc (nc * sizeof (gdouble));
  arrp->nrows = nr;
  arrp->ncols = nc;
}

/* allocate an array of doubles populated with 0 */
void
arrayd_alloc_zero (array_d * arrp, gint nr, gint nc)
{
  gint i;

  if ((arrp->nrows != 0) || (arrp->ncols != 0)) {
    arrayd_free (arrp); 
  }

  arrp->vals = (gdouble **) g_malloc (nr * sizeof (gdouble *));
  for (i = 0; i < nr; i++)
    arrp->vals[i] = (gdouble *) g_malloc0 (nc * sizeof (gdouble));
  arrp->nrows = nr;
  arrp->ncols = nc;
}

/* increase the number of rows in an array of doubles */
void
arrayd_add_rows (array_d * arrp, gint nr)
{
  gint i;

  if (nr > arrp->nrows) {

    arrp->vals = (gdouble **) g_realloc (arrp->vals, nr * sizeof (gdouble *));
    for (i = arrp->nrows; i < nr; i++)
      arrp->vals[i] = (gdouble *) g_malloc0 (arrp->ncols * sizeof (gdouble));

    arrp->nrows = nr;
  }
}

/* append columns to a floating point array for a total of nc columns */
void
arrayd_add_cols (array_d * arrp, gint nc)
{
  gint i, j;

  if (nc > arrp->ncols) {
    for (i = 0; i < arrp->nrows; i++) {
      arrp->vals[i] = (gdouble *) g_realloc (arrp->vals[i],
                                             nc * sizeof (gdouble));
      /*-- initialize the new values to 0 --*/
      for (j = arrp->ncols; j < nc; j++)
        arrp->vals[i][j] = 0;
    }
    arrp->ncols = nc;
  }
}

/*-- eliminate the nc columns contained in *cols --*/
void
arrayd_delete_cols (array_d * arrp, GSList *cols)
{
  gint i, k;
  gint jto, jfrom;
  guint nkeepers;
  guint *keepers = find_keepers (arrp->ncols, cols, &nkeepers);

  // FIXME: What happens when nkeepers is zero (all cols deleted)?
  if (cols && nkeepers > 0) {

    /*-- copy before reallocating --*/
    for (k = 0; k < nkeepers; k++) {
      jto = k;
      jfrom = keepers[k];
         /*-- jto has to be less than jfrom --*/
      if (jto != jfrom) {
        for (i = 0; i < arrp->nrows; i++)
          arrp->vals[i][jto] = arrp->vals[i][jfrom];
      }
    }

    for (i = 0; i < arrp->nrows; i++)
      arrp->vals[i] = (gdouble *) g_realloc (arrp->vals[i],
                                             nkeepers * sizeof (gdouble));
    arrp->ncols = nkeepers;
  }
  g_free (keepers);
}

/*-- eliminate the nr rows contained in *rows --*/
void
arrayd_delete_rows (array_d * arrp, GSList *rows)
{
  gint i, j, k;
  gint ito, ifrom;
  guint nkeepers;
  guint *keepers = find_keepers (arrp->nrows, rows, &nkeepers);

  if (rows && nkeepers > 0) {

    /*-- copy before reallocating --*/
    for (k = 0; k < nkeepers; k++) {
      ito = k;
      ifrom = keepers[k];
         /*-- ito has to be less than ifrom for copying --*/
      if (ito != ifrom) {
        for (j = 0; j < arrp->ncols; j++)
          arrp->vals[ito][j] = arrp->vals[ifrom][j];
      }
    }

    for (i = nkeepers; i < arrp->nrows; i++)
      g_free (arrp->vals[i]);
    arrp->vals = (gdouble **) g_realloc (arrp->vals,
                                         nkeepers * sizeof (gdouble *));
    arrp->nrows = nkeepers;
  }
  g_free (keepers);
}

void
arrayd_copy (array_d * arrp_from, array_d * arrp_to)
{
  gint i, j;

  if (arrp_from->ncols == arrp_to->ncols &&
      arrp_from->nrows == arrp_to->nrows) {
    for (i = 0; i < arrp_from->nrows; i++)
      for (j = 0; j < arrp_from->ncols; j++)
        arrp_to->vals[i][j] = arrp_from->vals[i][j];
  }
}

/*-------------------------------------------------------------------------*/
/*                      short integer array management                     */
/*-------------------------------------------------------------------------*/

void
arrays_init_null (array_s * arrp)
{
  arrp->nrows = arrp->ncols = 0;
  arrp->vals = (gshort **) NULL;
}

void
arrays_free (array_s * arrp, gint nr, gint nc)
{
  gint i;

  /*-- if nr != 0, free only the last nrows-nr rows --*/

  for (i = nr; i < arrp->nrows; i++)
    if (arrp->vals[i] != NULL)
      g_free (arrp->vals[i]);

  if (nr == 0) {
    if (arrp->vals != NULL)
      g_free (arrp->vals);
    arrp->vals = (gshort **) NULL;
  }

  arrp->nrows = nr;
  arrp->ncols = nc;
}

/* Zero a short array. */
void
arrays_zero (array_s * arrp)
{
  int i, j;
  for (i = 0; i < arrp->nrows; i++) {
    for (j = 0; j < arrp->ncols; j++) {
      arrp->vals[i][j] = 0;
    }
  }
}

/* allocate a short integer array */
void
arrays_alloc (array_s * arrp, gint nr, gint nc)
{
  int i;

  /*-- if the dimensions match, then the allocated size is surely ok --*/
  if (arrp->nrows != nr || arrp->ncols != nc) {

    if (arrp->nrows != 0 || arrp->ncols != 0)
      arrays_free (arrp, 0, 0);

    arrp->vals = (gshort **) g_malloc (nr * sizeof (gshort *));
    for (i = 0; i < nr; i++)
      arrp->vals[i] = (gshort *) g_malloc (nc * sizeof (gshort));
    arrp->nrows = nr;
    arrp->ncols = nc;
  }
}

/* allocate a short integer array populated with 0 */
void
arrays_alloc_zero (array_s * arrp, gint nr, gint nc)
{
  int i;

  if ((arrp->nrows != 0) || (arrp->ncols != 0))
    arrays_free (arrp, 0, 0);

  arrp->vals = (gshort **) g_malloc (nr * sizeof (gshort *));
  for (i = 0; i < nr; i++)
    arrp->vals[i] = (gshort *) g_malloc0 (nc * sizeof (gshort));
  arrp->nrows = nr;
  arrp->ncols = nc;
}

/* increase the number of rows in a short integer array */
void
arrays_add_rows (array_s * arrp, gint nr)
{
  gint i;

  if (nr > arrp->nrows) {

    arrp->vals = (gshort **) g_realloc (arrp->vals, nr * sizeof (gshort *));
    for (i = arrp->nrows; i < nr; i++)
      arrp->vals[i] = (gshort *) g_malloc0 (arrp->ncols * sizeof (gshort));

    arrp->nrows = nr;
  }
}

/* append columns to a short array for a total of nc columns */
void
arrays_add_cols (array_s * arrp, gint nc)
{
  gint i, j;

  if (nc > arrp->ncols) {
    for (i = 0; i < arrp->nrows; i++) {
      arrp->vals[i] = (gshort *) g_realloc (arrp->vals[i],
                                            nc * sizeof (gshort));
      /*-- initialize the new values to 0 --*/
      for (j = arrp->ncols; j < nc; j++)
        arrp->vals[i][j] = 0;
    }

    arrp->ncols = nc;
  }
}

/*-- eliminate the nc columns contained in *cols --*/
void
arrays_delete_cols (array_s * arrp, GSList *cols)
{
  gint i, k;
  gint jto, jfrom;
  guint nkeepers;
  guint *keepers = find_keepers (arrp->ncols, cols, &nkeepers);

  if (cols && nkeepers > 0) {

    /*-- copy before reallocating --*/
    for (k = 0; k < nkeepers; k++) {
      jto = k;
      jfrom = keepers[k];
         /*-- jto has to be less than jfrom --*/
      if (jto != jfrom) {
        for (i = 0; i < arrp->nrows; i++)
          arrp->vals[i][jto] = arrp->vals[i][jfrom];
      }
    }

    for (i = 0; i < arrp->nrows; i++)
      arrp->vals[i] = (gshort *) g_realloc (arrp->vals[i],
                                            nkeepers * sizeof (gshort));
    arrp->ncols = nkeepers;
  }

  g_free (keepers);
}

/*-- eliminate the nr rows contained in *rows --*/
void
arrays_delete_rows (array_s * arrp, GSList *rows)
{
  gint i, j, k;
  gint ito, ifrom;
  guint nkeepers;
  guint *keepers = find_keepers (arrp->nrows, rows, &nkeepers);

  if (rows && nkeepers > 0) {

    /*-- copy before reallocating --*/
    for (k = 0; k < nkeepers; k++) {
      ito = k;
      ifrom = keepers[k];
         /*-- ito has to be less than ifrom for copying --*/
      if (ito != ifrom) {
        for (j = 0; j < arrp->ncols; j++)
          arrp->vals[ito][j] = arrp->vals[ifrom][j];
      }
    }

    for (i = nkeepers; i < arrp->nrows; i++)
      g_free (arrp->vals[i]);
    arrp->vals = (gshort **) g_realloc (arrp->vals,
                                        nkeepers * sizeof (gshort *));
    arrp->nrows = nkeepers;
  }
  g_free (keepers);
}

/*-------------------------------------------------------------------------*/
/*                      long array management                              */
/*-------------------------------------------------------------------------*/

void
arrayl_init_null (array_l * arrp)
{
  arrp->nrows = arrp->ncols = 0;
  arrp->vals = (glong **) NULL;
}

void
arrayl_free (array_l * arrp, gint nr, gint nc)
{
  gint i;

  /*-- if nr != 0, free only the last nrows-nr rows --*/

  for (i = nr; i < arrp->nrows; i++)
    if (arrp->vals[i] != NULL)
      g_free (arrp->vals[i]);

  if (nr == 0) {
    if (arrp->vals != NULL)
      g_free (arrp->vals);
    arrp->vals = (glong **) NULL;
  }

  arrp->nrows = nr;
  arrp->ncols = nc;
}

/* Zero a long array. */
void
arrayl_zero (array_l * arrp)
{
  int i, j;
  for (i = 0; i < arrp->nrows; i++) {
    for (j = 0; j < arrp->ncols; j++) {
      arrp->vals[i][j] = 0;
    }
  }
}

/* allocate a long array */
void
arrayl_alloc (array_l * arrp, gint nr, gint nc)
{
  int i;

  if ((arrp->nrows != 0) || (arrp->ncols != 0))
    arrayl_free (arrp, 0, 0);

  arrp->vals = (glong **) g_malloc (nr * sizeof (glong *));
  for (i = 0; i < nr; i++)
    arrp->vals[i] = (glong *) g_malloc (nc * sizeof (glong));
  arrp->nrows = nr;
  arrp->ncols = nc;
}

/* allocate a long array populated with 0 */
void
arrayl_alloc_zero (array_l * arrp, gint nr, gint nc)
{
  int i;

  if ((arrp->nrows != 0) || (arrp->ncols != 0))
    arrayl_free (arrp, 0, 0);

  arrp->vals = (glong **) g_malloc (nr * sizeof (glong *));
  for (i = 0; i < nr; i++)
    arrp->vals[i] = (glong *) g_malloc0 (nc * sizeof (glong));
  arrp->nrows = nr;
  arrp->ncols = nc;
}

/* increase the number of rows in a long integer array */
void
arrayl_add_rows (array_l * arrp, gint nr)
{
  int i;

  if (nr > arrp->nrows) {

    arrp->vals = (glong **) g_realloc (arrp->vals, nr * sizeof (glong *));
    for (i = arrp->nrows; i < nr; i++)
      arrp->vals[i] = (glong *) g_malloc0 (arrp->ncols * sizeof (glong));

    arrp->nrows = nr;
  }
}

/* append columns to a long array for a total of nc columns */
void
arrayl_add_cols (array_l * arrp, gint nc)
{
  gint i, j;

  if (nc > arrp->ncols) {
    for (i = 0; i < arrp->nrows; i++) {
      arrp->vals[i] = (glong *) g_realloc (arrp->vals[i],
                                           nc * sizeof (glong));
      /*-- initialize the new values to 0 --*/
      for (j = arrp->ncols; j < nc; j++)
        arrp->vals[i][j] = 0;
    }
    arrp->ncols = nc;
  }
}

/*-- eliminate the nc columns contained in *cols --*/
void
arrayl_delete_cols (array_l * arrp, GSList *cols)
{
  gint i, k;
  gint jto, jfrom;
  guint nkeepers;
  guint *keepers = find_keepers (arrp->ncols, cols, &nkeepers);

  if (cols && nkeepers > 0) {

    /*-- copy before reallocating --*/
    for (k = 0; k < nkeepers; k++) {
      jto = k;
      jfrom = keepers[k];
         /*-- jto has to be less than jfrom --*/
      if (jto != jfrom) {
        for (i = 0; i < arrp->nrows; i++)
          arrp->vals[i][jto] = arrp->vals[i][jfrom];
      }
    }

    for (i = 0; i < arrp->nrows; i++)
      arrp->vals[i] = (glong *) g_realloc (arrp->vals[i],
                                           nkeepers * sizeof (glong));
    arrp->ncols = nkeepers;
  }
  g_free (keepers);
}

/*-- eliminate the nr rows contained in *rows --*/
void
arrayl_delete_rows (array_l * arrp, GSList *rows)
{
  gint i, j, k;
  gint ito, ifrom;
  guint nkeepers;
  guint *keepers = find_keepers (arrp->nrows, rows, &nkeepers);

  if (rows && nkeepers > 0) {

    /*-- copy before reallocating --*/
    for (k = 0; k < nkeepers; k++) {
      ito = k;
      ifrom = keepers[k];
         /*-- ito has to be less than ifrom for copying --*/
      if (ito != ifrom) {
        for (j = 0; j < arrp->ncols; j++)
          arrp->vals[ito][j] = arrp->vals[ifrom][j];
      }
    }

    for (i = nkeepers; i < arrp->nrows; i++)
      g_free (arrp->vals[i]);
    arrp->vals = (glong **) g_realloc (arrp->vals,
                                       nkeepers * sizeof (glong *));
    arrp->nrows = nkeepers;
  }
  g_free (keepers);
}


/* ---------------------------------------------------------------------*/
/*     Used in deleting: figure out which elements to keep              */
/* ---------------------------------------------------------------------*/

guint *
find_keepers (gint ncols_current, GSList *cols, guint * nkeepers)
{
  gint j;
  guint nc = g_slist_length(cols);
  guint *keepers = g_new(guint, ncols_current - nc);

  j = *nkeepers = 0;
  for (j = 0; j < ncols_current; j++) {
    if (cols) {
      if (GPOINTER_TO_INT(cols->data) != j) {
        keepers[(*nkeepers)++] = j;
      }
      else {
        cols = cols->next;
      }
    }
    else {
      keepers[(*nkeepers)++] = j;
    }
  }

  g_return_val_if_fail(*nkeepers == ncols_current - nc, NULL);

  return keepers;
}