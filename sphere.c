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

  d->sphere.vars = (gint *) g_malloc0 (d->ncols * sizeof (gint));
  d->sphere.eigenval = (gfloat *) g_malloc0 (d->ncols * sizeof (gfloat));

  d->sphere.eigenvec = (gfloat **) g_malloc (d->ncols * sizeof (gfloat *));
  for (j=0; j<d->ncols; j++)
    d->sphere.eigenvec[j] = (gfloat *) g_malloc0 (d->ncols * sizeof (gfloat));

  d->sphere.vc = (gfloat **) g_malloc (d->ncols * sizeof (gfloat *));
  for (j=0; j<d->ncols; j++)
    d->sphere.vc[j] = (gfloat *) g_malloc0 (d->ncols * sizeof (gfloat));

  d->sphere.tform_mean = (gfloat *) g_malloc0 (d->ncols * sizeof (gfloat));
}

void
sphere_free (datad *d, ggobid *gg) {
  gint j;

  g_free ((gpointer) d->sphere.vars);
  g_free ((gpointer) d->sphere.eigenval);

  for (j=0; j<d->ncols; j++)
    g_free ((gpointer) d->sphere.eigenvec[j]);
  g_free ((gpointer) d->sphere.eigenvec);

  for (j=0; j<d->ncols; j++)
    g_free ((gpointer) d->sphere.vc[j]);
  g_free ((gpointer) d->sphere.vc);

  g_free ((gpointer) d->sphere.tform_mean);
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
  gfloat firstpc = d->sphere.eigenval[0];
  gfloat lastpc = d->sphere.eigenval[d->sphere.npcs-1];

  for (j=0; j<d->sphere.npcs; j++)
    ftmp1 += d->sphere.eigenval[j];
  for (j=0; j<d->sphere.nvars; j++)
    ftmp2 += d->sphere.eigenval[j];

  sphere_totvar_set (ftmp1/ftmp2, d, gg);
  sphere_condnum_set (firstpc/lastpc, gg);
}


void
sphere_npcs_set (gint n, datad *d, ggobid *gg)
{
  d->sphere.npcs = n;
  if (!scree_mapped_p (gg)) {
    return;
  }

  if (d->sphere.npcs<1) {
     quick_message ("Need to choose at least 1 PC.", false);
     sphere_enable (false, gg);

  } else if (d->sphere.npcs > d->sphere.nvars) {
     gchar *msg = g_strdup_printf ("Need to choose at most %d PCs.\n",
       d->sphere.nvars);
     quick_message (msg, false);
     sphere_enable (false, gg);
     g_free (msg);

  } else {
    pca_diagnostics_set (d, gg);
    sphere_enable (true, gg);
  }
}
gint npcs_get (datad *d, ggobid *gg)
{
  return d->sphere.npcs;
}

/*-------------------------------------------------------------------------*/
/*    get and set the sphered variables:                                   */
/*    the user chooses the first npcs of these as principal components     */
/*-------------------------------------------------------------------------*/

void
spherevars_set (datad *d, ggobid *gg) {

  if (d->sphere.vars == NULL) {
    sphere_malloc (d, gg);
  }

  d->sphere.nvars = selected_cols_get (d->sphere.vars, d, gg);
  if (d->sphere.nvars == 0)
    d->sphere.nvars = plotted_cols_get (d->sphere.vars, d, gg);
}

/*-------------------------------------------------------------------------*/
/*    eigenvector and eigenvalues routines                                 */
/*-------------------------------------------------------------------------*/

void eigenvals_get (gfloat *vals, datad *d, ggobid *gg) {
  gint j;
  for (j=0; j<d->sphere.nvars; j++)
    vals[j] = d->sphere.eigenval[j];
}

void
eigenval_clear (datad *d, ggobid *gg)
{
  gint j;

  for (j=0; j<d->ncols; j++)
    d->sphere.eigenval[j] = 0.;
}

void
eigenvec_clear (datad *d, ggobid *gg)
{
  gint j, k;
  for (j=0; j<d->ncols; j++)
    for (k=0; k<d->ncols; k++)
      d->sphere.eigenvec[j][k] = 0.0;
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

  for (i=0; i<d->sphere.nvars; i++)
    for (j=0; j<d->sphere.nvars; j++)
      d->sphere.eigenvec[i][j] =
        d->sphere.vc[d->sphere.vars[i]][d->sphere.vars[j]];
}


/*-------------------------------------------------------------------------*/
/*       variance-covariance matrix routines                               */
/*-------------------------------------------------------------------------*/

void
sphere_varcovar_set (datad *d, ggobid *gg)
{
  gint i, j, k, var;
  gfloat tmpf = 0.;
  gint n = d->nrows_in_plot;

  for (k=0; k<d->sphere.nvars; k++) {
    var = d->sphere.vars[k];

    tmpf = 0.;
    for (i=0; i<n; i++)
      tmpf += d->tform.vals[d->rows_in_plot[i]][var];
    d->sphere.tform_mean[var] = tmpf / ((gfloat)n);

    tmpf = 0.;
    for (i=0; i<d->ncols; i++) {
      for (j=0; j<n; j++) {
        tmpf = tmpf +
        (d->tform.vals[d->rows_in_plot[j]][var] - d->sphere.tform_mean[var]) *
        (d->tform.vals[d->rows_in_plot[j]][i] - d->sphere.tform_mean[i]);
      }
      tmpf /= ((gfloat)(n - 1));
      d->sphere.vc[var][i] = d->sphere.vc[i][var] = tmpf;
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
  gint i, j, k, rank;
  gboolean vc_equals_I = vc_identity_p (d->sphere.eigenvec,
                                        d->sphere.nvars);
  paird *pairs = (paird *) g_malloc (d->sphere.nvars * sizeof (paird));
  /*-- scratch vector and array --*/
  gfloat *e = (gfloat *) g_malloc (d->sphere.nvars * sizeof (gfloat));
  gfloat **b = (gfloat **) g_malloc (d->sphere.nvars * sizeof (gfloat *));
  for (j=0; j<d->sphere.nvars; j++)
    b[j] = (gfloat *) g_malloc0 (d->sphere.nvars * sizeof (gfloat));

  if (!vc_equals_I) {
    eigenval_clear (d, gg);  /*-- zero out the vector of eigenvalues --*/
    dsvd (d->sphere.eigenvec, d->sphere.nvars, d->sphere.nvars,
          d->sphere.eigenval, b);
    for (j=0; j<d->sphere.nvars; j++) {
      d->sphere.eigenval[j] = (gfloat) sqrt ((gdouble) d->sphere.eigenval[j]);
    }
  }

  /*-- obtain ranks to use in sorting eigenvals and eigenvec --*/
  for (i=0; i<d->sphere.nvars; i++) {
    pairs[i].f = (gfloat) d->sphere.eigenval[i];
    pairs[i].indx = i;
  }
  qsort ((gchar *) pairs, d->sphere.nvars, sizeof (paird), pcompare);

  /*-- sort the eigenvalues and eigenvectors into temporary arrays --*/
  for (i=0; i<d->sphere.nvars; i++) {
    k = (d->sphere.nvars - i) - 1;  /*-- to reverse the order --*/
    rank = pairs[i].indx;
    e[k] = d->sphere.eigenval[rank];
    for (j=0; j<d->sphere.nvars; j++)
      b[j][k] = d->sphere.eigenvec[j][rank];
  }

  /*-- copy the sorted eigenvalues and eigenvectors back --*/
  for (i=0; i<d->sphere.nvars; i++) {
    d->sphere.eigenval[i] = e[i];
    for (j=0; j<d->sphere.nvars; j++)
      d->sphere.eigenvec[j][i] = b[j][i];
  }

  /*-- free temporary variables --*/
  g_free ((gpointer) pairs);
  for (j=0; j<d->sphere.nvars; j++)
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
          (d->tform.vals[i][svars[k]] - d->sphere.tform_mean[svars[k]]);
      }
      d->tform.vals[i][svars[j]] = tmpf / d->sphere.eigenval[j];
    }
  }
}

/*-------------------------------------------------------------------------*/
/*          Principal components analysis routines:                        */
/*  executed when the sphere button is pressed and the scree plot opened   */
/*-------------------------------------------------------------------------*/

/*-- why is this?  until apply is clicked, they're not going to be
     transformed, are they?
void sphere_transform_set (datad *d, ggobid *gg) {
  gint j;
  for (j=0; j<d->sphere.nvars; j++)
    transform2_values_set (SPHERE, d->sphere.vars[j], d, gg); 
}
--*/


gboolean
pca_calc (datad *d, ggobid *gg) {
  gboolean svd_ok;

  if (d->sphere.vars == NULL) {
    sphere_malloc (d, gg);
  }

  eigenvec_clear (d, gg);

  spherevars_set (d, gg);
/*
 * I don't understand the reason for this, so I'm commenting it out -- dfs
  sphere_transform_set (d, gg);
*/
  sphere_varcovar_set (d, gg);
  
   /* If nspherevars > 1 use svd routine, otherwise just standardize */
  if (d->sphere.nvars > 1) {
    eigenvec_set (d, gg);
    svd_ok = sphere_svd (d, gg);
  }

  return svd_ok;
}
