/* missing.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
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
missing_world_free (datad *d, ggobid *gg)
{
  arrayl_free (&d->missing_world, 0, 0);
  arrayl_free (&d->missing_jitter, 0, 0);
}

void
missing_world_alloc (datad *d, ggobid *gg)
{
  if (d->missing_world.vals != NULL) missing_world_free (d, gg);

  arrayl_alloc (&d->missing_world, d->nrows, d->ncols);
  arrayl_alloc (&d->missing_jitter, d->nrows, d->ncols);
}

void
missing_arrays_add_cols (datad *d, ggobid *gg)
{
  if (d->nmissing > 0 && d->missing.ncols < d->ncols) {
    arrays_add_cols (&d->missing, d->ncols);
    arrayl_add_cols (&d->missing_jitter, d->ncols);
    arrayl_add_cols (&d->missing_world, d->ncols);
  }
}

void
missing_arrays_add_rows (gint nrows, datad *d)
{
  if (d->nmissing > 0) {
    arrays_add_rows (&d->missing, nrows);
    arrayl_add_rows (&d->missing_jitter, nrows);
    arrayl_add_rows (&d->missing_world, nrows);
  }
}

/*------------------------------------------------------------------*/
/*      Scaling and jittering missing value plots                   */
/*------------------------------------------------------------------*/

void
missing_lim_set (datad *d, ggobid *gg)
{
  gint i, j;
  gshort min, max;

  min = max = d->missing.vals[0][0];
  for (i=0; i<d->nrows; i++) {
    for (j=0; j<d->ncols; j++) {
      if (d->missing.vals[i][j] < min) min = d->missing.vals[i][j];
      if (d->missing.vals[i][j] > max) max = d->missing.vals[i][j];
    }
  }
  d->missing_lim.min = (gfloat) min;
  d->missing_lim.max = (gfloat) max;
}

/*
 * This jitters the missings indicator vector for one variable
*/
void
missing_jitter_variable (gint jcol, datad *d, ggobid *gg)
{
  gint i, m;
  gfloat precis = (gfloat) PRECISION1;
  gfloat frand, fworld, fjit;

  for (i=0; i<d->nrows; i++) {
    m = d->rows_in_plot[i];

    frand = jitter_randval (d->jitter.type) * precis;

    if (d->jitter.convex) {
      fworld = (gfloat)
        (d->missing_world.vals[m][jcol] - d->missing_jitter.vals[m][jcol]);
      fjit = d->missing_jitter_factor * (frand - fworld);
    } else
      fjit = d->missing_jitter_factor * frand;

    d->missing_jitter.vals[m][jcol] = (glong) fjit;
  }
}

void
missing_jitter_value_set (gfloat value, datad *d, ggobid *gg) {
  d->missing_jitter_factor = (value > 0) ? value : 0;
}

static void
missing_jitter_init (datad *d, ggobid *gg)
{
  gint j;

  missing_jitter_value_set (.2, d, gg);
  for (j=0; j<d->ncols; j++)
    missing_jitter_variable (j, d, gg);
}

void
missing_to_world (datad *d, ggobid *gg)
{
  gint i, j, m;
  gfloat ftmp;
  gfloat precis = PRECISION1;
  gfloat min = d->missing_lim.min;
  gfloat range = d->missing_lim.max - d->missing_lim.min;
  static gboolean initd = false;

  if (!initd) {
    missing_jitter_init (d, gg);
    initd = true;
  }

  for (j=0; j<d->ncols; j++) {
    for (i=0; i<d->nrows_in_plot; i++) {
      m = d->rows_in_plot[i];
      ftmp = -1.0 + 2.0*(d->missing.vals[m][j] - min) / range;
      d->missing_world.vals[m][j] = (glong) (precis * ftmp);

      /* Add in the jitter values */
      d->missing_world.vals[m][j] += d->missing_jitter.vals[m][j];
    }
  }
}

void
missing_rejitter (gint *vars, gint nvars, datad *d, ggobid *gg) {
  gint j;

  for (j=0; j<nvars; j++)
    missing_jitter_variable (vars[j], d, gg);

  missing_to_world (d, gg);

  /*-- redisplay only the missings displays --*/
  displays_tailpipe (REDISPLAY_MISSING, FULL, gg); 
}

/*
 * For the datad currently selected in gg->impute.notebook,
 * generate a new datad using d->missing.  Maybe I should only
 * create missingness variables for those variables which have
 * missing values ...
*/
void missings_datad_cb (GtkWidget *w, ggobid *gg)
{
  GtkObject *obj = GTK_OBJECT(gg->impute.window);
  GtkWidget *clist = get_clist_from_object (obj);
  datad *d = (datad *) gtk_object_get_data (GTK_OBJECT (clist), "datad");
  static gchar *lnames[] = {"present", "missing"};

  if (d && d->nmissing > 0) {
    GtkWidget *notebook;
    datad *dnew;
    gint i, j, k;
    vartabled *vt, *vtnew;
    gint *cols;
    gint *cols_with_missings, ncols_with_missings;

    ncols_with_missings = 0;
    cols_with_missings = g_malloc (d->ncols * sizeof (gint));
    for (j=0; j<d->ncols; j++) {
      vt = vartable_element_get (j, d);
      if (vt->nmissing)
        cols_with_missings[ncols_with_missings++] = j;
    }

    notebook = (GtkWidget *) gtk_object_get_data (obj, "notebook");
    dnew = datad_create (d->nrows, ncols_with_missings, gg);
    dnew->name = g_strdup_printf ("%s (missing)", d->name);

    for (i=0; i<d->nrows; i++) {
      for (j=0; j<ncols_with_missings; j++) {
        k = cols_with_missings[j];
        dnew->raw.vals[i][j] = (gfloat) d->missing.vals[i][k];
      }
    }

    /*
     * ids to support linking: if the current datad doesn't
     * have ids, they need to be assigned.
    */
    if (d->rowid.id.nels == 0) {
      rowids_alloc (d);
      for (i=0; i<d->nrows; i++)
        d->rowid.id.els[i] = i;
      rowidv_init (d);
    }
    rowids_alloc (dnew);
    for (i=0; i<dnew->nrows; i++)
      dnew->rowid.id.els[i] = d->rowid.id.els[i];
    /*rowidv_init (dnew);*/  /* called in datad_init */
    /*-- --*/
    
/*
 * I'm going to make all the variables categorical.  For the moment,
 * there can be only two categories: present (0), missing (1).  In
 * the future, we might want to support other categories:  censored,
 * left-censored, etc.
*/
    vartable_alloc (dnew);
    vartable_init (dnew);
    for (j=0; j<ncols_with_missings; j++) {
      k = cols_with_missings[j];
      vt = vartable_element_get (k, d);
      vtnew = vartable_element_get (j, dnew);
      vtnew->collab = g_strdup (vt->collab);
      vtnew->collab_tform = g_strdup (vtnew->collab);

      /*-- categorical variable definitions --*/
      vtnew->categorical_p = true;
      vtnew->nlevels = 2;
      vtnew->level_values = NULL;
      vtnew->level_names = g_array_new (false, false, sizeof (gchar *));
      for (i=0; i<2; i++) {
        vtnew->level_values = g_list_append (vtnew->level_values,
          GINT_TO_POINTER(i));
        g_array_append_val (vtnew->level_names, lnames[i]);
      }

      /*-- prepare to jitter, and set limits to [0,1] --*/
      vtnew->lim_specified_p = true;  /*-- user-specified limits --*/
      vtnew->lim_specified.min = 0.0;
      vtnew->lim_specified_tform.min = 0.0;
      vtnew->lim_specified.max = 1.0;
      vtnew->lim_specified_tform.max = 1.0;
      vtnew->jitter_factor = .2;
    }

    rowlabels_alloc (dnew, gg);
    for (i=0; i<d->nrows; i++) {
      g_array_insert_val (dnew->rowlab, i,
        (gchar *) g_array_index (d->rowlab, gchar *, i));
    }

    datad_init (dnew, gg, false);

    /*-- jitter the data --*/
    /*-- forces unnecessary redisplay, unfortunately --*/
    cols = g_malloc (dnew->ncols * sizeof (gint));
    for (i=0; i<dnew->ncols; i++) cols[i] = i;
    rejitter (cols, dnew->ncols, dnew, gg); 

    /*-- copy the existing glyph and color --*/
    for (i=0; i<d->nrows; i++) {
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
