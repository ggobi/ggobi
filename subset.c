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

extern int strcasecmp(const char *, const char *);
extern int strncasecmp(const char *, const char *, size_t);

void
subset_init (datad *d, ggobid *gg)
{
  gfloat fnr = (gfloat) d->nrows;

  d->subset.random_n = d->nrows;
  d->subset.jvar = -1;

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
  gint i, k;
  gboolean subsetsize = 0;

  if (bstart >= 0 && bstart < d->nrows && bsize > 0) {
    subset_clear (d, gg);

    for (i=bstart, k=1; i<d->nrows && k<=bsize; i++, k++) {
      add_to_subset (i, d, gg);
      subsetsize++;
    }
  }

  if (subsetsize == 0)
    quick_message ("The limits aren't correctly specified.", false);
 
  return (subsetsize > 0);
}

gboolean
subset_range (greal min, greal max, gint j, datad *d, ggobid *gg)
{
  gint i;
  gboolean subsetsize = 0;

  if (min <max) {
    subset_clear (d, gg);

    for (i=0; i<d->nrows; i++) {
      if (d->tform.vals[i][j] >= min && d->tform.vals[i][j] <= max) {
        add_to_subset (i, d, gg);
        subsetsize++;
      }
    }
  }

  if (subsetsize == 0)
    quick_message ("The limits aren't correctly specified.", false);
 
  return (subsetsize > 0);
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
subset_rowlab (gchar *substr, gint substr_pos, gboolean ignore_case,
  datad *d, ggobid *gg)
{
  gint i;
  gint top = d->nrows;
  size_t slen, slen2;
  gchar *lbl;
  GtkWidget *w;

  if (substr == NULL || (slen = strlen(substr)) == 0)
    return false;

  /*-- remove all sticky labels --*/
  w = widget_find_by_name (gg->control_panel[IDENT],
    "IDENTIFY:remove_sticky_labels");
  gtk_signal_emit_by_name (GTK_OBJECT (w), "clicked", gg);
  /*-- --*/

  subset_clear (d, gg);

  for (i=0; i<top; i++) {
    switch (substr_pos) {
      case 0:  /* is identical to the string */
        if (ignore_case) {
          if (!strcasecmp ((gchar *) g_array_index (d->rowlab, gchar *, i),
            substr)) 
              add_to_subset (i, d, gg);
        } else {
          if (!strcmp ((gchar *) g_array_index (d->rowlab, gchar *, i),
            substr)) 
              add_to_subset (i, d, gg);
        }
      break;
      case 1:  /* includes the string -- I have to do more work
                  to ignore case */
        if (strstr ((gchar *) g_array_index (d->rowlab, gchar *, i), substr))
          add_to_subset (i, d, gg);
      break;
      case 2:  /* begins with the string */
        if (ignore_case) {
          if (!strncasecmp ((gchar *) g_array_index (d->rowlab, gchar *, i),
            substr, slen)) 
              add_to_subset (i, d, gg);
        } else {
          if (!strncmp ((gchar *) g_array_index (d->rowlab, gchar *, i),
            substr, slen)) 
              add_to_subset (i, d, gg);
        }
      break;
      case 3:  /* ends with the string */
        lbl = (gchar *) g_array_index (d->rowlab, gchar *, i);
        if ((slen2 = strlen(lbl)) >= slen) {
          if (ignore_case) {
            if (!strcmp (&lbl[slen2-slen], substr))
              add_to_subset (i, d, gg);
          } else {
            if (!strcasecmp (&lbl[slen2-slen], substr))
              add_to_subset (i, d, gg);
          }
        }
      break;
      case 4:  /* does not include the string: ditto about case */
        if (!strstr ((gchar *) g_array_index (d->rowlab, gchar *, i), substr))
          add_to_subset (i, d, gg);
      break;
    }
  }

  return true;
}
