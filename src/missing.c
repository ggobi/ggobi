/* missing.c */
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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <float.h>
#include <math.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*--------------------------------------------------------------------*/
/*             Memory management routines                             */
/*--------------------------------------------------------------------*/

void
missing_arrays_add_cols (GGobiData * d, ggobid * gg)
{
  if (d->nmissing > 0 && d->missing.ncols < d->ncols) {
    arrays_add_cols (&d->missing, d->ncols);
  }
}

void
missing_arrays_add_rows (gint nrows, GGobiData * d)
{
  if (d->nmissing > 0) {
    arrays_add_rows (&d->missing, nrows);
  }
}

/*------------------------------------------------------------------*/
/*      Scaling and jittering missing value plots                   */
/*------------------------------------------------------------------*/

/*
 * For the datad currently selected in gg->impute.notebook,
 * generate a new datad using d->missing.  Maybe I should only
 * create missingness variables for those variables which have
 * missing values ...
*/
void
missings_datad_cb (GtkWidget * w, ggobid * gg)
{
  GObject *obj = G_OBJECT (gg->impute.window);
  GtkWidget *tree_view = get_tree_view_from_object (obj);
  GGobiData *d =
    (GGobiData *) g_object_get_data (G_OBJECT (tree_view), "datad");
  static gchar *lnames[] = { "present", "missing" };

  if (d && d->nmissing > 0) {
    GtkWidget *notebook;
    GGobiData *dnew;
    gint i, j, k;
    vartabled *vt, *vtnew;
    gint *cols;
    gint *cols_with_missings, ncols_with_missings;

    ncols_with_missings = 0;
    cols_with_missings = g_malloc (d->ncols * sizeof (gint));
    for (j = 0; j < d->ncols; j++) {
      vt = vartable_element_get (j, d);
      if (vt->nmissing)
        cols_with_missings[ncols_with_missings++] = j;
    }

    notebook = (GtkWidget *) g_object_get_data (obj, "notebook");
    dnew = ggobi_data_new (d->nrows, ncols_with_missings);
    dnew->name = g_strdup_printf ("%s (missing)", d->name);

    for (i = 0; i < d->nrows; i++) {
      for (j = 0; j < ncols_with_missings; j++) {
        k = cols_with_missings[j];
        dnew->raw.vals[i][j] = (gfloat) d->missing.vals[i][k];
      }
    }

    /*
     * ids to support linking: if the current datad doesn't
     * have ids, they need to be assigned.
     */
    if (d->rowIds == NULL) {
      gchar **rowids = (gchar **) g_malloc (d->nrows * sizeof (gchar *));
      for (i = 0; i < d->nrows; i++)
        rowids[i] = g_strdup_printf ("%d", i);
      datad_record_ids_set (d, rowids, true);
      for (i = 0; i < d->nrows; i++)
        g_free (rowids[i]);
      g_free (rowids);
    }
    datad_record_ids_set (dnew, d->rowIds, true);
    /*-- --*/

/*
 * I'm going to make all the variables categorical.  For the moment,
 * there can be only two categories: present (0), missing (1).  In
 * the future, we might want to support other categories:  censored,
 * left-censored, etc.
*/
    for (j = 0; j < ncols_with_missings; j++) {
      k = cols_with_missings[j];
      vt = vartable_element_get (k, d);
      vtnew = vartable_element_get (j, dnew);
      vtnew->collab = g_strdup (vt->collab);
      vtnew->collab_tform = g_strdup (vtnew->collab);

      /*-- categorical variable definitions --*/
      vtnew->vartype = categorical;
      vtnew->nlevels = 2;
      vtnew->level_values = (gint *) g_malloc (sizeof (gint) * 2);
      vtnew->level_counts = (gint *) g_malloc (sizeof (gchar *) * 2);
      vtnew->level_names = (gchar **) g_malloc (sizeof (gchar *) * 2);
      for (i = 0; i < 2; i++) {
        vtnew->level_values[i] = i;
        vtnew->level_names[i] = g_strdup (lnames[i]);
      }
      vtnew->level_counts[0] = d->nrows - vt->nmissing;
      vtnew->level_counts[1] = vt->nmissing;

      /*-- prepare to jitter, and set limits to [0,1] --*/
      vtnew->lim_specified_p = true;  /*-- user-specified limits --*/
      vtnew->lim_specified.min = 0.0;
      vtnew->lim_specified_tform.min = 0.0;
      vtnew->lim_specified.max = 1.0;
      vtnew->lim_specified_tform.max = 1.0;
      vtnew->jitter_factor = .2;
    }

    for (i = 0; i < d->nrows; i++) {
      g_array_append_val (dnew->rowlab,
                          g_array_index (d->rowlab, gchar *, i));
    }

    datad_init (dnew, gg, false);

    /*-- jitter the data --*/
    /*-- forces unnecessary redisplay, unfortunately --*/
    cols = g_malloc (dnew->ncols * sizeof (gint));
    for (i = 0; i < dnew->ncols; i++)
      cols[i] = i;
    rejitter (cols, dnew->ncols, dnew, gg);

    /*-- copy the existing glyph and color --*/
    for (i = 0; i < d->nrows; i++) {
      dnew->color.els[i] = d->color.els[i];
      dnew->color_now.els[i] = d->color_now.els[i];
      dnew->glyph.els[i].type = d->glyph.els[i].type;
      dnew->glyph_now.els[i].type = d->glyph_now.els[i].type;
      dnew->glyph.els[i].size = d->glyph.els[i].size;
      dnew->glyph_now.els[i].size = d->glyph_now.els[i].size;
    }

    /*-- this should be executed in response to any datad_added event --*/
    display_menu_build (gg);

    g_free (cols);
    g_free (cols_with_missings);
  }
}
