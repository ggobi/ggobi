/*-- sphere.c --*/

#include <math.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*-- persistent, dimensioned using gg.ncols --*/
static gint nspherevars = 0;
static gint *spherevars = NULL;
static gint sphere_npcs = 0;

static gfloat *eigenval = NULL;
static gfloat **eigenvec = NULL;
static gfloat **vc = NULL;
static gfloat *tform1_mean = NULL;
/* */

/*-- these will all be moved to externs.h --*/
extern gint dsvd (float **, int, int, float *, float **);
extern void sphere_totvar_set  (gfloat);
extern void sphere_condnum_set  (gfloat);
extern void transform2_values_set (gint, gint);
extern void sphere_enable (gboolean);
/* */

/*-------------------------------------------------------------------------*/
/*      dynamic memory allocation routines                                 */
/*-------------------------------------------------------------------------*/

/*-- note: these need to change when a variable is cloned --*/
void
sphere_malloc () {
  gint j;

  spherevars = (gint *) g_malloc0 (gg.ncols * sizeof (gint));
  eigenval = (gfloat *) g_malloc0 (gg.ncols * sizeof (gfloat));

  eigenvec = (gfloat **) g_malloc (gg.ncols * sizeof (gfloat *));
  for (j=0; j<gg.ncols; j++)
    eigenvec[j] = (gfloat *) g_malloc0 (gg.ncols * sizeof (gfloat));

  vc = (gfloat **) g_malloc (gg.ncols * sizeof (gfloat *));
  for (j=0; j<gg.ncols; j++)
    vc[j] = (gfloat *) g_malloc0 (gg.ncols * sizeof (gfloat));

  tform1_mean = (gfloat *) g_malloc0 (gg.ncols * sizeof (gfloat));
}

void
sphere_free () {
  gint j;

  g_free ((gpointer) spherevars);
  g_free ((gpointer) eigenval);

  for (j=0; j<gg.ncols; j++)
    g_free ((gpointer) eigenvec[j]);
  g_free ((gpointer) eigenvec);

  for (j=0; j<gg.ncols; j++)
    g_free ((gpointer) vc[j]);
  g_free ((gpointer) vc);

  g_free ((gpointer) tform1_mean);
}

/*-------------------------------------------------------------------------*/
/*      get and set the number of principal components;                    */
/*      changed when the spin button is reset                              */
/*-------------------------------------------------------------------------*/

void pca_diagnostics_set (void) {
/*
 * Compute and set the total variance and the condition number
 * of the principal components analysis
*/
  gint j;
  gfloat ftmp1=0.0, ftmp2=0.0;
     
  for (j=0; j<sphere_npcs; j++)
    ftmp1 += eigenval[j];
  for (j=0; j<nspherevars; j++)
    ftmp2 += eigenval[j];

  sphere_totvar_set (ftmp1/ftmp2);
  sphere_condnum_set (eigenval[0]/eigenval[sphere_npcs-1]);
}

extern gboolean scree_mapped_p (void);

void sphere_npcs_set (gint n) {
  sphere_npcs = n;
  if (!scree_mapped_p ())
    return;

  if (sphere_npcs<1) {
     quick_message ("Need to choose at least 1 PC.", false);
     sphere_enable (false);

  } else if (sphere_npcs > nspherevars) {
     gchar *msg = g_strdup_printf ("Need to choose at most %d PCs.\n",
       nspherevars);
     quick_message (msg, false);
     sphere_enable (false);
     g_free (msg);

  } else {
    pca_diagnostics_set ();
    sphere_enable (true);
  }
}
gint sphere_npcs_get ()
{
  return sphere_npcs;
}

/*-------------------------------------------------------------------------*/
/*    get and set the sphered variables:                                   */
/*    the user chooses a subset of these as principal components           */
/*-------------------------------------------------------------------------*/

void
spherevars_set () {
  if (spherevars == NULL) sphere_malloc ();

  nspherevars = selected_cols_get (spherevars, false);
  if (nspherevars == 0)
    nspherevars = plotted_cols_get (spherevars, false);
}
gint
spherevars_get (gint *vars) {
  gint j;
  for (j=0; j<nspherevars; j++)
    vars[j] = spherevars[j];
  return nspherevars;
}
gint 
nspherevars_get () { return nspherevars; }

/*-------------------------------------------------------------------------*/
/*    eigenvector and eigenvalues routines                                 */
/*-------------------------------------------------------------------------*/

void eigenvals_get (gfloat *vals) {
  gint j;
  for (j=0; j<nspherevars; j++)
    vals[j] = eigenval[j];
}

void
eigenval_clear (void)
{
  gint j;

  for (j=0; j<gg.ncols; j++)
    eigenval[j] = 0.;
}

/*
 * this routine overwrites the var-cov matrix of the old active variables
 * with the new active variables. then this matrix is put through
 * singular value decomposition and overwritten with the matrix of
 * eigenvectors, and the eigenvalues are returned in a.
*/
void
eigenvec_set ()
{
  gint i, j;

  for (i=0; i<nspherevars; i++)
    for (j=0; j<nspherevars; j++)
      eigenvec[i][j] = vc[spherevars[i]][spherevars[j]];
}


/*-------------------------------------------------------------------------*/
/*       variance-covariance matrix routines                               */
/*-------------------------------------------------------------------------*/

void
sphere_varcovar_set (void)
{
  gint i, j, k, var;
  gfloat tmpf = 0.;
  gint n = gg.nrows;

  for (k=0; k<nspherevars; k++) {
    var = spherevars[k];

    for (i=0; i<n; i++)
      tmpf += gg.tform1.data[gg.rows_in_plot[i]][var];
    tform1_mean[var] = tmpf / ((gfloat)n);

    tmpf = 0.;
    for (i=0; i<gg.ncols; i++) {
      for (j=0; j<n; j++) {
        tmpf = tmpf +
          (gg.tform1.data[gg.rows_in_plot[j]][var] - tform1_mean[var]) *
          (gg.tform1.data[gg.rows_in_plot[j]][i] - tform1_mean[i]);
      }
      tmpf /= ((gfloat)(n - 1));
      vc[var][i] = tmpf;
      vc[i][var] = tmpf;
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
gboolean sphere_svd (void)
{
  gint i, j, rank;
  gboolean vc_equals_I = vc_identity_p (eigenvec, nspherevars);
  paird *pairs = (paird *) g_malloc (nspherevars * sizeof (paird));
  /*-- scratch vector and array --*/
  gfloat *e = (gfloat *) g_malloc (nspherevars * sizeof (gfloat));
  gfloat **b = (gfloat **) g_malloc (nspherevars * sizeof (gfloat *));
  for (j=0; j<nspherevars; j++)
    b[j] = (gfloat *) g_malloc0 (nspherevars * sizeof (gfloat));

  if (!vc_equals_I) {
    gint j;
    eigenval_clear ();  /*-- zero out the vector of eigenvalues --*/
    dsvd (eigenvec, nspherevars, nspherevars, eigenval, b);
    for (j=0; j<nspherevars; j++) {
      eigenval[j] = (gfloat) sqrt ((gdouble) eigenval[j]);
    }
  }

  /*-- obtain ranks to use in sorting eigenvals and eigenvec --*/
  for (i=0; i<nspherevars; i++) {
    pairs[i].f = spherevars[i];
    pairs[i].indx = i;
  }
  qsort ((gchar *) pairs, nspherevars, sizeof (paird), pcompare);

  /*-- sort the eigenvalues and eigenvectors into temporary arrays --*/
  for (i=0; i<nspherevars; i++) {
    rank = pairs[i].indx;
    e[i] = eigenval[rank];
    for (j=0; j<nspherevars; j++)
      b[i][j] = eigenvec[rank][j];
  }
  /*-- copy the sorted eigenvalues and eigenvectors back --*/
  for (i=0; i<nspherevars; i++) {
    eigenval[i] = e[i];
    for (j=0; j<nspherevars; j++)
      eigenvec[i][j] = b[i][j];
  }

  /*-- free temporary variables --*/
  g_free ((gpointer) pairs);
  for (j=0; j<nspherevars; j++)
    g_free (b[j]);
  g_free (b);
  g_free (e);

  return (!vc_equals_I);
}

void
spherize_data (gint num_pcs, gint nsvars, gint *svars)
{
  gint i, j, k, m;
  gfloat tmpf;

  for (m=0; m<gg.nrows_in_plot; m++) {
    i = gg.rows_in_plot[m];

    for (j=0; j<num_pcs; j++) {
      tmpf = 0.;
      for (k=0; k<nsvars; k++) {
        tmpf = tmpf + eigenvec[k][j] *
          (gg.tform1.data[i][svars[k]] - tform1_mean[svars[k]]);
      }
      gg.tform2.data[i][svars[j]] = tmpf / eigenval[j];
    }
  }
}

void sphere_apply_cb (void) { 
/*
 * finally, sphere the number of principal components selected;
 * executed when the apply button is pressed
*/
  gint j;

  if ((sphere_npcs > 0) && (sphere_npcs <= nspherevars))
  {
    if (eigenval[sphere_npcs-1] == 0.0 ||
        eigenval[0]/eigenval[sphere_npcs-1] > 10000.0)
    {
      quick_message ("Need to choose fewer PCs. Var-cov close to singular.",
        false);
    }
    else {
      spherize_data (sphere_npcs, nspherevars, spherevars);
      sphere_varcovar_set ();
/*    pc_axes_sensitive_set (true);*/

      for (j=0; j<nspherevars; j++)
        tform_label_update (spherevars[j]);

      /*-- these three lines replicated from transform.c --*/
      vardata_lim_update ();
      tform_to_world ();
      displays_tailpipe (REDISPLAY_PRESENT);
    }
  }
}


/*-------------------------------------------------------------------------*/
/*          Principal components analysis routines:                        */
/*  executed when the sphere button is pressed and the scree plot opened   */
/*-------------------------------------------------------------------------*/

void sphere_transform_set (void) {
  gint j;
  for (j=0; j<nspherevars; j++)
    transform2_values_set (SPHERE, spherevars[j]); 
}


gboolean
pca_calc (void) {
  gboolean svd_ok;
g_printerr ("(pca_calc)\n");

  if (spherevars == NULL)
    sphere_malloc ();

  spherevars_set ();
  sphere_transform_set ();
  sphere_varcovar_set ();
  
   /* If nspherevars > 1 use svd routine, otherwise just standardize */
  if (nspherevars > 1) {
    eigenvec_set ();
    svd_ok = sphere_svd ();
  }

  return svd_ok;
}
