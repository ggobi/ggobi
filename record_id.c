/*-- rowids.c --*/
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

#define MAXIDSIZE 104

/*--------------------------------------------------------------------*/
/*                   Memory allocation, initialization                */
/*--------------------------------------------------------------------*/

void rowids_free (datad *d, ggobid *gg)
{
  g_array_free (d->rowid.name, true);  /*-- true: free each element --*/
  g_array_free (d->rowid.uniq, true);
  vectori_free (&d->rowid.id);

  d->rowid.name = NULL;
  d->rowid.uniq = NULL;
}


void
rowids_alloc (datad *d, ggobid *gg) 
{
  if (d->rowid.name != NULL) rowids_free (d, gg);

  d->rowid.name = g_array_new (false, false, sizeof (gchar *));
  d->rowid.uniq = g_array_new (false, false, sizeof (gchar *));
  vectori_alloc (&d->rowid.id, d->nrows);
}

/*--------------------------------------------------------------------*/
/*                       Utilities                                    */
/*--------------------------------------------------------------------*/

void
cswap (gchar *s1, gchar *s2)
{
  char tmp[MAXIDSIZE];

  strcpy (tmp, s1);
  strcpy (s1, s2);
  strcpy (s2, tmp);
}

void
qstrsort (gchar **arr, gint low, gint high)
{
  gint left = low, right = high;
  gchar pivot[MAXIDSIZE];
  strcpy (pivot, arr[ (left + right) / 2 ]);

  while (right >= left) {
    /* find the next elements on the wrong sides of the pivot and swap them */
    while ((strcmp (arr[right], pivot)) > 0) right--;
    while ((strcmp (arr[left],  pivot)) < 0) left++;
    if (left <= right) {
      cswap (arr[left], arr[right]);
      left++;   right--;
    }
  }
  /* recursively sort the lower and upper parts of the array */
  if (low < right) qstrsort (arr, low, right);
  if (left < high) qstrsort (arr, left, high);
}

/*--------------------------------------------------------------------*/
/*      Sorting and uniq'ing id names to map them to integers         */
/*--------------------------------------------------------------------*/

void
rowid_names_unique_set (datad *d)
{
  gint i;
  gchar **ids_sorted = (gchar **) g_malloc (d->nrows * sizeof (gchar *));
  for (i=0; i<d->nrows; i++) {
    ids_sorted[i] = g_strdup ((gchar *)
      g_array_index (d->rowid.name, gchar *, i));
if (i<3) {
g_printerr ("%s\n", (gchar *) g_array_index (d->rowid.name, gchar *, i));
}
  }

  qstrsort (ids_sorted, 0, d->nrows-1);

  /*
   * rowid.uniq is a GArray containing an alphabetically sorted
   * array of unique row ids.  Its length can range from 1 to nrows.
  */
  g_array_append_val (d->rowid.uniq, ids_sorted[0]);
  for (i=1; i<d->nrows; i++)
    if (strcmp (ids_sorted[i], ids_sorted[i-1]) != 0)
      g_array_append_val (d->rowid.uniq, ids_sorted[i]);
}

/*--------------------------------------------------------------------*/
/*           Assigning integer ids to each record/case/row            */
/*--------------------------------------------------------------------*/

void
rowids_assign (datad *d)
{
  gint i, k;

  for (i=0; i<d->nrows; i++) {
    for (k=0; k<d->rowid.uniq->len; k++) {
      if (!strcmp ((gchar *) g_array_index (d->rowid.name, gchar *, i),
                   (gchar *) g_array_index (d->rowid.uniq, gchar *, k)))
      {
        d->rowid.id.els[i] = k;
        break;
      }
    }
  }
}

/*--------------------------------------------------------------------*/

void
row_ids_init (datad *d)
{
{
  gint k;
  for (k=0; k<d->rowid.name->len; k++) {
    g_printerr ("%s\n", (gchar *) g_array_index (d->rowid.name, gchar *, k));
  }
}

  rowid_names_unique_set (d);

/*
{
  gint k;
  for (k=0; k<d->rowid.uniq->len; k++) {
    g_printerr ("%s\n", (gchar *) g_array_index (d->rowid.uniq, gchar *, k));
  }
}
*/

  rowids_assign (d);
}
