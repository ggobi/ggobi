/*-- sphere.c --*/

#include <math.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/* */

/*-- these will all be moved to externs.h --*/
extern gint dsvd (float **, int, int, float *, float **);
extern void sphere_totvar_set  (gfloat);
extern void sphere_condnum_set  (gfloat);
extern void transform2_values_set (gint, gint, ggobid *gg);
extern void sphere_enable (gboolean);
/* */

/*-------------------------------------------------------------------------*/
/*      dynamic memory allocation routines                                 */
/*-------------------------------------------------------------------------*/

/*-- note: these need to change when a variable is cloned --*/
void
sphere_malloc (ggobid *gg) 
{
  gint j;

  gg->sphere.spherevars = (gint *) g_malloc0 (gg->ncols * sizeof (gint));
  gg->sphere.eigenval = (gfloat *) g_malloc0 (gg->ncols * sizeof (gfloat));

  gg->sphere.eigenvec = (gfloat **) g_malloc (gg->ncols * sizeof (gfloat *));
  for (j=0; j<gg->ncols; j++)
    gg->sphere.eigenvec[j] = (gfloat *) g_malloc0 (gg->ncols * sizeof (gfloat));

  gg->sphere.vc = (gfloat **) g_malloc (gg->ncols * sizeof (gfloat *));
  for (j=0; j<gg->ncols; j++)
    gg->sphere.vc[j] = (gfloat *) g_malloc0 (gg->ncols * sizeof (gfloat));

  gg->sphere.tform1_mean = (gfloat *) g_malloc0 (gg->ncols * sizeof (gfloat));
}

void
sphere_free (ggobid *gg) {
  gint j;

  g_free ((gpointer) gg->sphere.spherevars);
  g_free ((gpointer) gg->sphere.eigenval);

  for (j=0; j<gg->ncols; j++)
    g_free ((gpointer) gg->sphere.eigenvec[j]);
  g_free ((gpointer) gg->sphere.eigenvec);

  for (j=0; j<gg->ncols; j++)
    g_free ((gpointer) gg->sphere.vc[j]);
  g_free ((gpointer) gg->sphere.vc);

  g_free ((gpointer) gg->sphere.tform1_mean);
}

/*-------------------------------------------------------------------------*/
/*      get and set the number of principal components;                    */
/*      changed when the spin button is reset                              */
/*-------------------------------------------------------------------------*/

void pca_diagnostics_set (ggobid *gg) {
/*
 * Compute and set the total variance and the condition number
 * of the principal components analysis
*/
  gint j;
  gfloat ftmp1=0.0, ftmp2=0.0;
     
  for (j=0; j<gg->sphere.sphere_npcs; j++)
    ftmp1 += gg->sphere.eigenval[j];
  for (j=0; j<gg->sphere.nspherevars; j++)
    ftmp2 += gg->sphere.eigenval[j];

  sphere_totvar_set (ftmp1/ftmp2);
  sphere_condnum_set (gg->sphere.eigenval[0]/gg->sphere.eigenval[gg->sphere.sphere_npcs-1]);
}

extern gboolean scree_mapped_p (void);

void sphere_npcs_set (gint n, ggobid *gg) {
  gg->sphere.sphere_npcs = n;
  if (!scree_mapped_p ())
    return;

  if (gg->sphere.sphere_npcs<1) {
     quick_message ("Need to choose at least 1 PC.", false);
     sphere_enable (false);

  } else if (gg->sphere.sphere_npcs > gg->sphere.nspherevars) {
     gchar *msg = g_strdup_printf ("Need to choose at most %d PCs.\n",
       gg->sphere.nspherevars);
     quick_message (msg, false);
     sphere_enable (false);
     g_free (msg);

  } else {
    pca_diagnostics_set (gg);
    sphere_enable (true);
  }
}
gint sphere_npcs_get (ggobid *gg)
{
  return gg->sphere.sphere_npcs;
}

/*-------------------------------------------------------------------------*/
/*    get and set the sphered variables:                                   */
/*    the user chooses a subset of these as principal components           */
/*-------------------------------------------------------------------------*/

void
spherevars_set (ggobid *gg) {
  if (gg->sphere.spherevars == NULL) sphere_malloc (gg);

  gg->sphere.nspherevars = selected_cols_get (gg->sphere.spherevars, false, gg);
  if (gg->sphere.nspherevars == 0)
    gg->sphere.nspherevars = plotted_cols_get (gg->sphere.spherevars, false, gg);
}
gint
spherevars_get (gint *vars, ggobid *gg) {
  gint j;
  for (j=0; j<gg->sphere.nspherevars; j++)
    vars[j] = gg->sphere.spherevars[j];
  return gg->sphere.nspherevars;
}

gint 
nspherevars_get (ggobid *gg) { return gg->sphere.nspherevars; }

/*-------------------------------------------------------------------------*/
/*    eigenvector and eigenvalues routines                                 */
/*-------------------------------------------------------------------------*/

void eigenvals_get (gfloat *vals, ggobid *gg) {
  gint j;
  for (j=0; j<gg->sphere.nspherevars; j++)
    vals[j] = gg->sphere.eigenval[j];
}

void
eigenval_clear (ggobid *gg)
{
  gint j;

  for (j=0; j<gg->ncols; j++)
    gg->sphere.eigenval[j] = 0.;
}

/*
 * this routine overwrites the var-cov matrix of the old active variables
 * with the new active variables. then this matrix is put through
 * singular value decomposition and overwritten with the matrix of
 * eigenvectors, and the eigenvalues are returned in a.
*/
void
eigenvec_set (ggobid *gg)
{
  gint i, j;

  for (i=0; i<gg->sphere.nspherevars; i++)
    for (j=0; j<gg->sphere.nspherevars; j++)
      gg->sphere.eigenvec[i][j] = gg->sphere.vc[gg->sphere.spherevars[i]][gg->sphere.spherevars[j]];
}


/*-------------------------------------------------------------------------*/
/*       variance-covariance matrix routines                               */
/*-------------------------------------------------------------------------*/

void
sphere_varcovar_set (ggobid *gg)
{
  gint i, j, k, var;
  gfloat tmpf = 0.;
  gint n = gg->nrows;

  for (k=0; k<gg->sphere.nspherevars; k++) {
    var = gg->sphere.spherevars[k];

    for (i=0; i<n; i++)
      tmpf += gg->tform1.data[gg->rows_in_plot[i]][var];
    gg->sphere.tform1_mean[var] = tmpf / ((gfloat)n);

    tmpf = 0.;
    for (i=0; i<gg->ncols; i++) {
      for (j=0; j<n; j++) {
        tmpf = tmpf +
          (gg->tform1.data[gg->rows_in_plot[j]][var] - gg->sphere.tform1_mean[var]) *
          (gg->tform1.data[gg->rows_in_plot[j]][i] - gg->sphere.tform1_mean[i]);
      }
      tmpf /= ((gfloat)(n - 1));
      gg->sphere.vc[var][i] = tmpf;
      gg->sphere.vc[i][var] = tmpf;
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
gboolean sphere_svd (ggobid *gg)
{
  gint i, j, rank;
  gboolean vc_equals_I = vc_identity_p (gg->sphere.eigenvec, gg->sphere.nspherevars);
  paird *pairs = (paird *) g_malloc (gg->sphere.nspherevars * sizeof (paird));
  /*-- scratch vector and array --*/
  gfloat *e = (gfloat *) g_malloc (gg->sphere.nspherevars * sizeof (gfloat));
  gfloat **b = (gfloat **) g_malloc (gg->sphere.nspherevars * sizeof (gfloat *));
  for (j=0; j<gg->sphere.nspherevars; j++)
    b[j] = (gfloat *) g_malloc0 (gg->sphere.nspherevars * sizeof (gfloat));

  if (!vc_equals_I) {
    gint j;
    eigenval_clear (gg);  /*-- zero out the vector of eigenvalues --*/
    dsvd (gg->sphere.eigenvec, gg->sphere.nspherevars, gg->sphere.nspherevars, gg->sphere.eigenval, b);
    for (j=0; j<gg->sphere.nspherevars; j++) {
      gg->sphere.eigenval[j] = (gfloat) sqrt ((gdouble) gg->sphere.eigenval[j]);
    }
  }

  /*-- obtain ranks to use in sorting eigenvals and eigenvec --*/
  for (i=0; i<gg->sphere.nspherevars; i++) {
    pairs[i].f = gg->sphere.spherevars[i];
    pairs[i].indx = i;
  }
  qsort ((gchar *) pairs, gg->sphere.nspherevars, sizeof (paird), pcompare);

  /*-- sort the eigenvalues and eigenvectors into temporary arrays --*/
  for (i=0; i<gg->sphere.nspherevars; i++) {
    rank = pairs[i].indx;
    e[i] = gg->sphere.eigenval[rank];
    for (j=0; j<gg->sphere.nspherevars; j++)
      b[i][j] = gg->sphere.eigenvec[rank][j];
  }
  /*-- copy the sorted eigenvalues and eigenvectors back --*/
  for (i=0; i<gg->sphere.nspherevars; i++) {
    gg->sphere.eigenval[i] = e[i];
    for (j=0; j<gg->sphere.nspherevars; j++)
      gg->sphere.eigenvec[i][j] = b[i][j];
  }

  /*-- free temporary variables --*/
  g_free ((gpointer) pairs);
  for (j=0; j<gg->sphere.nspherevars; j++)
    g_free (b[j]);
  g_free (b);
  g_free (e);

  return (!vc_equals_I);
}

void
spherize_data (gint num_pcs, gint nsvars, gint *svars, ggobid *gg)
{
  gint i, j, k, m;
  gfloat tmpf;

  for (m=0; m<gg->nrows_in_plot; m++) {
    i = gg->rows_in_plot[m];

    for (j=0; j<num_pcs; j++) {
      tmpf = 0.;
      for (k=0; k<nsvars; k++) {
        tmpf = tmpf + gg->sphere.eigenvec[k][j] *
          (gg->tform1.data[i][svars[k]] - gg->sphere.tform1_mean[svars[k]]);
      }
      gg->tform2.data[i][svars[j]] = tmpf / gg->sphere.eigenval[j];
    }
  }
}

void sphere_apply_cb (ggobid *gg) { 
/*
 * finally, sphere the number of principal components selected;
 * executed when the apply button is pressed
*/
  gint j;

  if ((gg->sphere.sphere_npcs > 0) && (gg->sphere.sphere_npcs <= gg->sphere.nspherevars))
  {
    if (gg->sphere.eigenval[gg->sphere.sphere_npcs-1] == 0.0 ||
        gg->sphere.eigenval[0]/gg->sphere.eigenval[gg->sphere.sphere_npcs-1] > 10000.0)
    {
      quick_message ("Need to choose fewer PCs. Var-cov close to singular.",
        false);
    }
    else {
      spherize_data (gg->sphere.sphere_npcs, gg->sphere.nspherevars, gg->sphere.spherevars, gg);
      sphere_varcovar_set (gg);
/*    pc_axes_sensitive_set (true);*/

      for (j=0; j<gg->sphere.nspherevars; j++)
        tform_label_update (gg->sphere.spherevars[j], gg);

      /*-- these three lines replicated from transform.c --*/
      vardata_lim_update (gg);
      tform_to_world (gg);
      displays_tailpipe (REDISPLAY_PRESENT, gg);
    }
  }
}


/*-------------------------------------------------------------------------*/
/*          Principal components analysis routines:                        */
/*  executed when the sphere button is pressed and the scree plot opened   */
/*-------------------------------------------------------------------------*/

void sphere_transform_set (ggobid *gg) {
  gint j;
  for (j=0; j<gg->sphere.nspherevars; j++)
    transform2_values_set (SPHERE, gg->sphere.spherevars[j], gg); 
}


gboolean
pca_calc (ggobid *gg) {
  gboolean svd_ok;
g_printerr ("(pca_calc)\n");

  if (gg->sphere.spherevars == NULL)
    sphere_malloc (gg);

  spherevars_set (gg);
  sphere_transform_set (gg);
  sphere_varcovar_set (gg);
  
   /* If nspherevars > 1 use svd routine, otherwise just standardize */
  if (gg->sphere.nspherevars > 1) {
    eigenvec_set (gg);
    svd_ok = sphere_svd (gg);
  }

  return svd_ok;
}
