/*-- sphere.c --*/

#include <math.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"


/*-------------------------------------------------------------------------*/
/*      dynamic memory allocation routines                                 */
/*-------------------------------------------------------------------------*/

/*-- note: these need to change when a variable is cloned --*/
void
sphere_malloc (datad *d, ggobid *gg) 
{
  gint j;

  d->sphere.spherevars = (gint *) g_malloc0 (d->ncols * sizeof (gint));
  d->sphere.eigenval = (gfloat *) g_malloc0 (d->ncols * sizeof (gfloat));

  d->sphere.eigenvec = (gfloat **) g_malloc (d->ncols * sizeof (gfloat *));
  for (j=0; j<d->ncols; j++)
    d->sphere.eigenvec[j] = (gfloat *) g_malloc0 (d->ncols * sizeof (gfloat));

  d->sphere.vc = (gfloat **) g_malloc (d->ncols * sizeof (gfloat *));
  for (j=0; j<d->ncols; j++)
    d->sphere.vc[j] = (gfloat *) g_malloc0 (d->ncols * sizeof (gfloat));

  d->sphere.tform1_mean = (gfloat *) g_malloc0 (d->ncols * sizeof (gfloat));
}

void
sphere_free (datad *d, ggobid *gg) {
  gint j;

  g_free ((gpointer) d->sphere.spherevars);
  g_free ((gpointer) d->sphere.eigenval);

  for (j=0; j<d->ncols; j++)
    g_free ((gpointer) d->sphere.eigenvec[j]);
  g_free ((gpointer) d->sphere.eigenvec);

  for (j=0; j<d->ncols; j++)
    g_free ((gpointer) d->sphere.vc[j]);
  g_free ((gpointer) d->sphere.vc);

  g_free ((gpointer) d->sphere.tform1_mean);
}

/*-------------------------------------------------------------------------*/
/*      get and set the number of principal components;                    */
/*      changed when the spin button is reset                              */
/*-------------------------------------------------------------------------*/

void pca_diagnostics_set (datad *d, ggobid *gg) {
/*
 * Compute and set the total variance and the condition number
 * of the principal components analysis
*/
  gint j;
  gfloat ftmp1=0.0, ftmp2=0.0;
     
  for (j=0; j<d->sphere.sphere_npcs; j++)
    ftmp1 += d->sphere.eigenval[j];
  for (j=0; j<d->sphere.nspherevars; j++)
    ftmp2 += d->sphere.eigenval[j];

  sphere_totvar_set (ftmp1/ftmp2, d, gg);
  sphere_condnum_set (d->sphere.eigenval[0]/d->sphere.eigenval[d->sphere.sphere_npcs-1], gg);
}


void sphere_npcs_set (gint n, datad *d, ggobid *gg) {
  d->sphere.sphere_npcs = n;
  if (!scree_mapped_p ())
    return;

  if (d->sphere.sphere_npcs<1) {
     quick_message ("Need to choose at least 1 PC.", false);
     sphere_enable (false, gg);

  } else if (d->sphere.sphere_npcs > d->sphere.nspherevars) {
     gchar *msg = g_strdup_printf ("Need to choose at most %d PCs.\n",
       d->sphere.nspherevars);
     quick_message (msg, false);
     sphere_enable (false, gg);
     g_free (msg);

  } else {
    pca_diagnostics_set (d, gg);
    sphere_enable (true, gg);
  }
}
gint sphere_npcs_get (datad *d, ggobid *gg)
{
  return d->sphere.sphere_npcs;
}

/*-------------------------------------------------------------------------*/
/*    get and set the sphered variables:                                   */
/*    the user chooses a subset of these as principal components           */
/*-------------------------------------------------------------------------*/

void
spherevars_set (datad *d, ggobid *gg) {
  if (d->sphere.spherevars == NULL) sphere_malloc (d, gg);

  d->sphere.nspherevars = selected_cols_get (d->sphere.spherevars,
    false, d, gg);
  if (d->sphere.nspherevars == 0)
    d->sphere.nspherevars = plotted_cols_get (d->sphere.spherevars,
      false, d, gg);
}

gint
spherevars_get (gint *vars, datad *d, ggobid *gg) {
  gint j;
  for (j=0; j<d->sphere.nspherevars; j++)
    vars[j] = d->sphere.spherevars[j];
  return d->sphere.nspherevars;
}

gint 
nspherevars_get (datad *d, ggobid *gg) { return d->sphere.nspherevars; }

/*-------------------------------------------------------------------------*/
/*    eigenvector and eigenvalues routines                                 */
/*-------------------------------------------------------------------------*/

void eigenvals_get (gfloat *vals, datad *d, ggobid *gg) {
  gint j;
  for (j=0; j<d->sphere.nspherevars; j++)
    vals[j] = d->sphere.eigenval[j];
}

void
eigenval_clear (datad *d, ggobid *gg)
{
  gint j;

  for (j=0; j<d->ncols; j++)
    d->sphere.eigenval[j] = 0.;
}

/*
 * this routine overwrites the var-cov matrix of the old active variables
 * with the new active variables. then this matrix is put through
 * singular value decomposition and overwritten with the matrix of
 * eigenvectors, and the eigenvalues are returned in a.
*/
void
eigenvec_set (datad *d, ggobid *gg)
{
  gint i, j;

  for (i=0; i<d->sphere.nspherevars; i++)
    for (j=0; j<d->sphere.nspherevars; j++)
      d->sphere.eigenvec[i][j] =
        d->sphere.vc[d->sphere.spherevars[i]][d->sphere.spherevars[j]];
}


/*-------------------------------------------------------------------------*/
/*       variance-covariance matrix routines                               */
/*-------------------------------------------------------------------------*/

void
sphere_varcovar_set (datad *d, ggobid *gg)
{
  gint i, j, k, var;
  gfloat tmpf = 0.;
  gint n = d->nrows;

  for (k=0; k<d->sphere.nspherevars; k++) {
    var = d->sphere.spherevars[k];

    for (i=0; i<n; i++)
      tmpf += d->tform1.vals[d->rows_in_plot[i]][var];
    d->sphere.tform1_mean[var] = tmpf / ((gfloat)n);

    tmpf = 0.;
    for (i=0; i<d->ncols; i++) {
      for (j=0; j<n; j++) {
        tmpf = tmpf +
          (d->tform1.vals[d->rows_in_plot[j]][var] - d->sphere.tform1_mean[var]) *
          (d->tform1.vals[d->rows_in_plot[j]][i] - d->sphere.tform1_mean[i]);
      }
      tmpf /= ((gfloat)(n - 1));
      d->sphere.vc[var][i] = tmpf;
      d->sphere.vc[i][var] = tmpf;
    }
  }
}

/*
 * returns true if the nxn variance-covariance matrix 'matrx'
 * is within tol of being equal to the identity matrix
*/
gboolean
vc_identity_p (gfloat **matrx, gint n)
{
  gint i, j;
  gboolean retn_val = true;
  gfloat tol=0.001;

  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) {
      if ((i==j) && (fabs ((gdouble)(1.000-matrx[i][j])) > tol)) {
        retn_val = false;
        break;
      }
      else if ((i!=j) && (fabs ((gdouble)matrx[i][j]) > tol)) {
        retn_val = false;
        break;
      }
    }
  }
  return (retn_val);
}

/*-------------------------------------------------------------------------*/
/*       Here's the beef:  sphering routines                               */
/*-------------------------------------------------------------------------*/

/*
 * As long as eigenvec is not equal to the identity matrix, 
 * perform a singular value decomposition.
*/
gboolean sphere_svd (datad *d, ggobid *gg)
{
  gint i, j, rank;
  gboolean vc_equals_I = vc_identity_p (d->sphere.eigenvec,
                                        d->sphere.nspherevars);
  paird *pairs = (paird *) g_malloc (d->sphere.nspherevars * sizeof (paird));
  /*-- scratch vector and array --*/
  gfloat *e = (gfloat *) g_malloc (d->sphere.nspherevars * sizeof (gfloat));
  gfloat **b = (gfloat **) g_malloc (d->sphere.nspherevars * sizeof (gfloat *));
  for (j=0; j<d->sphere.nspherevars; j++)
    b[j] = (gfloat *) g_malloc0 (d->sphere.nspherevars * sizeof (gfloat));

  if (!vc_equals_I) {
    gint j;
    eigenval_clear (d, gg);  /*-- zero out the vector of eigenvalues --*/
    dsvd (d->sphere.eigenvec, d->sphere.nspherevars, d->sphere.nspherevars,
          d->sphere.eigenval, b);
    for (j=0; j<d->sphere.nspherevars; j++) {
      d->sphere.eigenval[j] = (gfloat) sqrt ((gdouble) d->sphere.eigenval[j]);
    }
  }

  /*-- obtain ranks to use in sorting eigenvals and eigenvec --*/
  for (i=0; i<d->sphere.nspherevars; i++) {
    pairs[i].f = d->sphere.spherevars[i];
    pairs[i].indx = i;
  }
  qsort ((gchar *) pairs, d->sphere.nspherevars, sizeof (paird), pcompare);

  /*-- sort the eigenvalues and eigenvectors into temporary arrays --*/
  for (i=0; i<d->sphere.nspherevars; i++) {
    rank = pairs[i].indx;
    e[i] = d->sphere.eigenval[rank];
    for (j=0; j<d->sphere.nspherevars; j++)
      b[i][j] = d->sphere.eigenvec[rank][j];
  }
  /*-- copy the sorted eigenvalues and eigenvectors back --*/
  for (i=0; i<d->sphere.nspherevars; i++) {
    d->sphere.eigenval[i] = e[i];
    for (j=0; j<d->sphere.nspherevars; j++)
      d->sphere.eigenvec[i][j] = b[i][j];
  }

  /*-- free temporary variables --*/
  g_free ((gpointer) pairs);
  for (j=0; j<d->sphere.nspherevars; j++)
    g_free (b[j]);
  g_free (b);
  g_free (e);

  return (!vc_equals_I);
}

void
spherize_data (gint num_pcs, gint nsvars, gint *svars, datad *d, ggobid *gg)
{
  gint i, j, k, m;
  gfloat tmpf;

  for (m=0; m<d->nrows_in_plot; m++) {
    i = d->rows_in_plot[m];

    for (j=0; j<num_pcs; j++) {
      tmpf = 0.;
      for (k=0; k<nsvars; k++) {
        tmpf = tmpf + d->sphere.eigenvec[k][j] *
          (d->tform1.vals[i][svars[k]] - d->sphere.tform1_mean[svars[k]]);
      }
      d->tform2.vals[i][svars[j]] = tmpf / d->sphere.eigenval[j];
    }
  }
}

void sphere_apply_cb (GtkWidget *w, ggobid *gg) { 
/*
 * finally, sphere the number of principal components selected;
 * executed when the apply button is pressed
*/
  gint j;
  datad *d = gg->current_display->d;
  gint firstpc = d->sphere.eigenval[0];
  gint lastpc = d->sphere.eigenval[d->sphere.sphere_npcs-1];

  if ((d->sphere.sphere_npcs > 0) &&
      (d->sphere.sphere_npcs <= d->sphere.nspherevars))
  {
    if (lastpc == 0.0 || firstpc/lastpc > 10000.0) {
      quick_message ("Need to choose fewer PCs. Var-cov close to singular.",
        false);
    }
    else {
      spherize_data (d->sphere.sphere_npcs,
        d->sphere.nspherevars, d->sphere.spherevars, d, gg);
      sphere_varcovar_set (d, gg);
/*    pc_axes_sensitive_set (true);*/

      for (j=0; j<d->sphere.nspherevars; j++)
        tform_label_update (d->sphere.spherevars[j], d, gg);

      /*-- these three lines replicated from transform.c --*/
      vardata_lim_update (d, gg);
      tform_to_world (d, gg);
      displays_tailpipe (REDISPLAY_PRESENT, gg);
    }
  }
}


/*-------------------------------------------------------------------------*/
/*          Principal components analysis routines:                        */
/*  executed when the sphere button is pressed and the scree plot opened   */
/*-------------------------------------------------------------------------*/

void sphere_transform_set (datad *d, ggobid *gg) {
  gint j;
  for (j=0; j<d->sphere.nspherevars; j++)
    transform2_values_set (SPHERE, d->sphere.spherevars[j], d, gg); 
}


gboolean
pca_calc (datad *d, ggobid *gg) {
  gboolean svd_ok;

  if (d->sphere.spherevars == NULL)
    sphere_malloc (d, gg);

  spherevars_set (d, gg);
  sphere_transform_set (d, gg);
  sphere_varcovar_set (d, gg);
  
   /* If nspherevars > 1 use svd routine, otherwise just standardize */
  if (d->sphere.nspherevars > 1) {
    eigenvec_set (d, gg);
    svd_ok = sphere_svd (d, gg);
  }

  return svd_ok;
}
