/*-- sphere.c --*/

#include <math.h>

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

static gint sphere_npcs = 0;

/*-- add to ggobi.h?  --*/
gfloat *xg_eigenval;
gfloat *xg_a;
gfloat **xg_vc, **xg_vc_active;
gfloat **xg_b;
gfloat *xg_mean;

gint xg_nspherevars;
gint *xg_spherevars;

extern gboolean sphere_get_state (void);
extern gint dsvd (float **a, int m, int n, float *w, float **v);
extern gint sphere_npcs;

void
zero_ab (void)
{
  gint i, j;

  for (i=0; i<xg.ncols; i++) {
    xg_a[i] = 0.;
    for (j=0; j<xg.ncols; j++)
      xg_b[i][j] = 0.;
  }
}

void
vc_set (gint var)
{
  gint i, j;
  gfloat tmpf = 0.;
  gint n = xg.nlinkable_in_plot;

  for (i=0; i<n; i++)
    tmpf += xg.tform1[xg.rows_in_plot[i]][var];
  xg_mean[var] = tmpf / ((gfloat)n);

  tmpf = 0.;
  for (i=0; i<xg.ncols; i++) {
    for (j=0; j<n; j++) {
      tmpf = tmpf +
        (xg.tform1[xg.rows_in_plot[j]][var] - xg_mean[var]) *
        (xg.tform1[xg.rows_in_plot[j]][i] - xg_mean[i]);
    }
    tmpf /= ((gfloat)(n - 1));
    xg_vc[var][i] = tmpf;
    xg_vc[i][var] = tmpf;
  }
}

/* this routine overwrites the var-cov matrix of the old active variables
 * with the new active variables. then this matrix is put through
 * singular value decomposition and overwritten with the matrix of
 * eigenvectors, and the eigenvalues are returned in a.
*/
void
vc_active_set (gint nsvars, gint *svars)
{
  gint i, j;

  for (i=0; i<nsvars; i++)
    for (j=0; j<nsvars; j++)
      xg_vc_active[i][j] = xg_vc[svars[i]][svars[j]];
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

/*
 * As long as xg_vc_active is not equal to the identity matrix, 
 * perform a singular value decomposition.
*/
gboolean sphere_svd (gint nsvars, gint *svars)
{
  gboolean vc_equals_I = vc_identity_p (xg_vc_active, nsvars);

  if (!vc_equals_I) {
    gint i;
    zero_ab ();
    dsvd (xg_vc_active, nsvars, nsvars, xg_a, xg_b);
    for (i=0; i<nsvars; i++) {
      xg_eigenval[i] = xg_a[i];
    }
  }
    
  return (!vc_equals_I);
}

/*-- This is the one it's all about:  it sets the values in tform2 --*/

void
spherize_data (gint num_pcs, gint nsvars, gint *svars)
{
  gint i, j, k, m;
  gfloat tmpf;

  for (i=0; i<num_pcs; i++) {
    xg_a[i] = (gfloat) sqrt ((gdouble) xg_eigenval[i]);
  }
  
  for (m=0; m<xg.nrows_in_plot; m++) {
    i = xg.rows_in_plot[m];

    for (j=0; j<num_pcs; j++) {
      tmpf = 0.;
      for (k=0; k<nsvars; k++) {
        tmpf = tmpf +
          xg_vc_active[k][j] * (xg.tform1[i][svars[k]] - xg_mean[svars[k]]);
      }
      tmpf /= (xg_a[j]);
      xg_b[0][j] = tmpf;
    }
    for (j=0; j<num_pcs; j++)
      xg.tform2[i][svars[j]] = xg_b[0][j];
  }
}

void sphere_apply_cb (void)  /*-- when apply button is pressed --*/
{
  gint j;

  if ((sphere_npcs > 0) && (sphere_npcs <= xg_nspherevars))
  {
    if (xg_eigenval[sphere_npcs-1] == 0.0 ||
        xg_eigenval[0]/xg_eigenval[sphere_npcs-1] > 10000.0)
    {
      quick_message ("Need to choose fewer PCs. Var-cov close to singular.",
        false);
    }
    else {
      spherize_data (sphere_npcs, xg_nspherevars, xg_spherevars);

      for (j=0; j<sphere_npcs; j++)
        vc_set (j);

/*      xg.is_princ_comp = true;*/
/*      pc_axes_sensitive_set (true);*/
/*      plot_once ();*/
    }
  }
}
