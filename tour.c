/* tour.c */

#include <gtk/gtk.h>
#include <strings.h>
#include <math.h>

#include "vars.h"
#include "externs.h"

void
alloc_tour(displayd *dsp)
{
  gint nc = xg.ncols;
  gint i;

/* 2 x ncols */

  dsp->u0 = (gfloat **) g_malloc((gint) 2 * sizeof(gfloat *));
  for (i=0; i<2; i++)
    dsp->u0[i] = (gfloat *) g_malloc0(nc * sizeof(gfloat));

  dsp->u1 = (gfloat **) g_malloc((gint) 2 * sizeof(gfloat *));
  for (i=0; i<2; i++)
    dsp->u1[i] = (gfloat *) g_malloc0(nc * sizeof(gfloat));

  dsp->u = (gfloat **) g_malloc((gint) 2 * sizeof(gfloat *));
  for (i=0; i<2; i++)
    dsp->u[i] = (gfloat *) g_malloc0(nc * sizeof(gfloat));

  dsp->uold = (gfloat **) g_malloc((gint) 2 * sizeof(gfloat *));
  for (i=0; i<2; i++)
    dsp->uold[i] = (gfloat *) g_malloc0(nc * sizeof(gfloat));

  dsp->v0 = (gfloat **) g_malloc((gint) 2 * sizeof(gfloat *));
  for (i=0; i<2; i++)
    dsp->v0[i] = (gfloat *) g_malloc0(nc * sizeof(gfloat));

  dsp->v1 = (gfloat **) g_malloc((gint) 2 * sizeof(gfloat *));
  for (i=0; i<2; i++)
    dsp->v1[i] = (gfloat *) g_malloc0(nc * sizeof(gfloat));

  dsp->tv = (gfloat **) g_malloc((gint) 2 * sizeof(gfloat *));
  for (i=0; i<2; i++)
    dsp->tv[i] = (gfloat *) g_malloc0(nc * sizeof(gfloat));

  dsp->tour_vars = (gint *) g_malloc0(nc * sizeof(gint));

  dsp->tau = (gfloat *) g_malloc0(nc * sizeof(gfloat));
  dsp->tinc = (gfloat *) g_malloc0(nc * sizeof(gfloat));

}

void
free_tour(displayd *dsp)
{
  gint k;

  g_free((gpointer) dsp->tour_vars);
  g_free((gpointer) dsp->tau);
  g_free((gpointer) dsp->tinc);

  for (k=0; k<2; k++)
    g_free((gpointer) dsp->u[k]);
  g_free((gpointer) dsp->u);
  for (k=0; k<2; k++)
    g_free((gpointer) dsp->u0[k]);
  g_free((gpointer) dsp->u0);
  for (k=0; k<2; k++)
    g_free((gpointer) dsp->u1[k]);
  g_free((gpointer) dsp->u1);
  for (k=0; k<2; k++)
    g_free((gpointer) dsp->uold[k]);
  g_free((gpointer) dsp->uold);

  for (k=0; k<2; k++)
    g_free((gpointer) dsp->v0[k]);
  g_free((gpointer) dsp->v0);
  for (k=0; k<2; k++)
    g_free((gpointer) dsp->v1[k]);
  g_free((gpointer) dsp->v1);
  for (k=0; k<2; k++)
    g_free((gpointer) dsp->tv[k]);
  g_free((gpointer) dsp->tv);

}

void
zero_tau(displayd *dsp) {
  gint k;

  for (k=0; k<xg.ncols; k++) {
    dsp->tau[k]  = 0.0;
    dsp->tinc[k] = 0.0;
  }
}

void
zero_tinc(displayd *dsp) {
  gint k;

  for (k=0; k<xg.ncols; k++) {
    dsp->tinc[k] = 0.0;
  }
}

/* This function initializes the tour variables - it should only be
   called more than once, when a new tour is started since a new
   subset of variable might be used, or when there is new data. */

void 
cpanel_tour_init(cpaneld *cpanel) {
    cpanel->is_tour_paused = false;
    cpanel->is_tour_local_scan = false;
    cpanel->is_tour_stepping = false;
    cpanel->is_tour_backtracking = false;
    cpanel->tour_step = TOURSTEP0;
    cpanel->tour_ls_dir = TOUR_LS_IN;
    cpanel->tour_path_len = 1.;
}

void 
display_tour_init(displayd *dsp) {
    gint i, j;

    alloc_tour(dsp);
 
    /* Initialize starting subset of active variables */
    dsp->ntour_vars = 3;
    dsp->tour_vars[0] = 0;
    dsp->tour_vars[1] = 1;
    dsp->tour_vars[2] = 2;

    /* declare starting base as first two chosen variables */
    for (i=0; i<2; i++)
      for (j=0; j<xg.ncols; j++)
        dsp->u0[i][j] = dsp->u1[i][j] = dsp->u[i][j] = dsp->uold[i][j] =
          dsp->v0[i][j] = dsp->v1[i][j] = 0.0;

    dsp->u1[0][dsp->tour_vars[0]] =
      dsp->u0[0][dsp->tour_vars[0]] = dsp->u[0][dsp->tour_vars[0]] =
      dsp->v0[0][dsp->tour_vars[0]] = dsp->v1[0][dsp->tour_vars[0]] = 1.0;
    dsp->u1[1][dsp->tour_vars[1]] = dsp->u[1][dsp->tour_vars[1]] =
      dsp->u0[1][dsp->tour_vars[1]] =
      dsp->v0[1][dsp->tour_vars[1]] = dsp->v1[1][dsp->tour_vars[1]] = 1.0;

    dsp->ts[0] = 0;
    dsp->ts[1] = M_PI_2;
    dsp->coss[0] = 1.0;
    dsp->coss[1] = 0.0;
    dsp->sins[1] = 1.0;
    dsp->sins[0] = 0.0;
    dsp->icoss[0] = PRECISION2;
    dsp->icoss[1] = 0;
    dsp->isins[1] = PRECISION2;
    dsp->isins[0] = 0;

    zero_tau(dsp);
    dsp->delta = 0.0;
    dsp->dv = 1.0;
}

void
tour_reproject (splotd *sp, glong **world_data)
/*
 * This routine uses the data projected ginto the span of
 * the starting basis and ending basis, and then rotates in this
 * space.
*/
{
  gint i, j, m, n=xg.ncols;
  gfloat costf[2], sintf[2];
  gint costi[2], sinti[2];
  displayd *dsp = (displayd *) sp->displayptr;

  for (i=0; i<2; i++)
  {
    costf[i] = (gfloat) cos( (gdouble) dsp->tinc[i]);
    sintf[i] = (gfloat) sin( (gdouble) dsp->tinc[i]);
    costi[i] = (gint)(costf[i] * PRECISION2);
    sinti[i] = (gint) (sintf[i] * PRECISION2);
  }

  /* basically do calculations ready for use in drawing
   * segments in variable circles */
  /* do these in integer to speed calculations? */
  for (i=0; i<n; i++)
  {
    dsp->tv[0][i] = costf[0]*dsp->v0[0][i] + sintf[0]*dsp->v1[0][i];
    dsp->tv[1][i] = costf[1]*dsp->v0[1][i] + sintf[1]*dsp->v1[1][i];
  }

  for (i=0; i<n; i++)
  {
    dsp->u[0][i] = dsp->coss[0] * dsp->tv[0][i] - dsp->sins[0]*dsp->tv[1][i];
    dsp->u[1][i] = -dsp->coss[1] * dsp->tv[0][i] + dsp->sins[1]*dsp->tv[1][i];
  }

  /* This version of tour doesn't use preprojection */
  for (m=0; m<xg.nrows_in_plot; m++)
  {
    i = xg.rows_in_plot[m];

    sp->planar[i].x = 0;
    sp->planar[i].y = 0;
    for (j=0; j<n; j++)
    {
      sp->planar[i].x += (gint)(dsp->u[0][j]*world_data[i][j]);
      sp->planar[i].y += (gint)(dsp->u[1][j]*world_data[i][j]);
    }
  }
}

void
do_last_increment(displayd *dsp, cpaneld *cpanel)
{
  gboolean doit = false;

  if ((dsp->tinc[0] != cpanel->tour_path_len*dsp->tau[0]) ||
      (dsp->tinc[1] != cpanel->tour_path_len*dsp->tau[1]))
  {
    dsp->tinc[0] = cpanel->tour_path_len*dsp->tau[0];
    dsp->tinc[1] = cpanel->tour_path_len*dsp->tau[1];
    doit = true;
  }
  if (doit) {
    display_tailpipe (dsp);
  }
}

void
copy_basis(gfloat **source, gfloat **target, gint n, gint d)
{
  gint i, j;

  for (i=0; i<d; i++)
    for (j=0; j<n; j++)
      target[i][j] = source[i][j];
}

void
init_basis(displayd *dsp)
{
/*
 * Set u0 (old first basis) to be u(t) (new first basis)
*/
  copy_basis(dsp->u, dsp->u0, xg.ncols, 2);
}


void
norm(gfloat *x, gint n)
{
  gint j;
  gdouble xn = 0;

  for (j=0; j<n; j++)
    xn = xn + (gdouble) (x[j]*x[j]);
  xn = sqrt(xn);
  for (j=0; j<n; j++)
    x[j] = x[j]/(gfloat)xn;
}

gfloat
calc_norm(gfloat *x, gint n)
{
  gint j;
  gdouble xn = 0;

  for (j=0; j<n; j++)
    xn = xn + (gdouble)(x[j]*x[j]);
  xn = sqrt(xn);

  return((gfloat)xn);
}

gfloat
calc_norm_sq(gfloat *x, gint n)
{
  gint j;
  gdouble xn = 0;

  for (j=0; j<n; j++)
    xn = xn + (gdouble)(x[j]*x[j]);

  return((gfloat)xn);
}

gfloat
inner_prod(gfloat *x1, gfloat *x2, gint n)
{
  gdouble xip;
  gint j;

  xip = 0.;
  for (j=0; j<n; j++)
    xip = xip + (gdouble)x1[j]*(gdouble)x2[j];
  return((gfloat)xip);
}

gfloat
sq_inner_prod(gfloat *x1, gfloat *x2, gint n)
{
  gfloat xip;
  gint j;

  xip = 0;
  for (j=0; j<n; j++)
    xip = xip + x1[j]*x2[j];
  return(xip*xip);
}

void
gram_schmidt(gfloat *x1, gfloat *x2, gint n)
{
  gint j;
  gfloat ip;

  ip = inner_prod(x1, x2, n);
  for (j=0; j<n; j++)
    x2[j] = x2[j] - ip*x1[j];

  norm(x2, n);
}

gint
check_proximity(gfloat **base1, gfloat **base2, gint n)
/*
 * This routine checks if two bases are close to identical
*/
{
  gfloat diff, ssq1 = 0.0, ssq2 = 0.0;
  gint i, indic = 0;
  gfloat tol = 0.001;

  for (i=0; i<n; i++)
  {
    diff = base1[0][i] - base2[0][i];
    ssq1 = ssq1 + (diff * diff);

    diff = base1[1][i] - base2[1][i];
    ssq2 = ssq2 + (diff * diff);
  }

  if ((ssq1 < tol) && (ssq2 < tol))
    indic = 1;

  return(indic);
}

void
gt_basis(displayd *dsp)
/*
 * Generate two random p dimensional vectors to form new ending basis
*/
{
  gint j, check = 1;
  gdouble frunif[2];
  gdouble r, fac, frnorm[2];

/*
 * Method suggested by Press, Flannery, Teukolsky, and Vetterling (1986)
 * "Numerical Recipes" p.202-3, for generating random normal variates .
*/

  /* Zero out u1 before filling; this might fix a bug we are
     encountering with returning from a receive tour.
  */
  for (j=0; j<xg.ncols; j++)
    dsp->u1[0][j] = dsp->u1[1][j] = 0.0 ;

  if (dsp->ntour_vars > 2) {
    for (j=0; j<dsp->ntour_vars; j++) {
      while (check) {

        rnorm2(&frunif[0], &frunif[1]);
        r = frunif[0] * frunif[0] + frunif[1] * frunif[1];
  
        if (r < 1)
        {
          check = 0;
          fac = sqrt(-2. * log(r) / r);
          frnorm[0] = frunif[0] * fac;
          frnorm[1] = frunif[1] * fac;
        }
      }
      check = 1;
      dsp->u1[0][dsp->tour_vars[j]] = (gfloat) frnorm[0];
      dsp->u1[1][dsp->tour_vars[j]] = (gfloat) frnorm[1];
    }
    norm(dsp->u1[0], xg.ncols);
    norm(dsp->u1[1], xg.ncols);
/*
 * Orthogonalize the second vector on the first using Gram-Schmidt
*/
    gram_schmidt(dsp->u1[0], dsp->u1[1], xg.ncols);
  }
  else
  {
    dsp->u1[0][dsp->tour_vars[0]] = 1.;
    dsp->u1[1][dsp->tour_vars[1]] = 1.;
  }
}

void
basis_dir_ang(displayd *dsp)
{
  gfloat x, y ;
  gint k;
  gint n = xg.ncols;
  static gfloat angle_tol = 0.001;

/* calculate values to minimize angle between two base pairs */
  x = sq_inner_prod(dsp->u0[0], dsp->u1[0], n) +
    sq_inner_prod(dsp->u0[0], dsp->u1[1], n) -
    inner_prod(dsp->u0[1], dsp->u1[0], n) *
    inner_prod(dsp->u0[1], dsp->u1[0], n) -
    inner_prod(dsp->u0[1], dsp->u1[1], n) *
    inner_prod(dsp->u0[1], dsp->u1[1], n);

  y = inner_prod(dsp->u0[0], dsp->u1[0], n) *
    inner_prod(dsp->u0[1], dsp->u1[0], n) +
    inner_prod(dsp->u0[0], dsp->u1[1], n) *
    inner_prod(dsp->u0[1], dsp->u1[1], n);

/* calculate angles of rotation from bases (u) to princ dirs (v) */
  if (fabs(x) < angle_tol)
  {
    dsp->ts[0] = 0.0;
    dsp->ts[1] = M_PI_2;
  }
  else
  {
    dsp->ts[0] = (gfloat) atan2((gdouble)(2.*y),(gdouble)x)/2.;
    dsp->ts[1] = dsp->ts[0] + M_PI_2;
  }

/* calculate cosines and sines of s */
  for (k=0; k<2; k++)
  {
    dsp->coss[k] = (gfloat) cos((gdouble)dsp->ts[k]);
    dsp->sins[k] = (gfloat) sin( (gdouble) dsp->ts[k]);
    dsp->icoss[k] = (gint) (dsp->coss[k] * PRECISION2);
    dsp->isins[k] = (gint) (dsp->sins[k] * PRECISION2);
  }
}

void
princ_dirs(displayd *dsp)
{
  gint j;
  gint n = xg.ncols;
  
/* calculate first princ dirs */ /* if there are frozen vars u0 won't
                                     have norm 1, but since this is just
                                     rotation it should be ok */
  for (j=0; j<n; j++)
  {
    dsp->v0[0][j] = dsp->coss[0]*dsp->u0[0][j] + dsp->sins[0]*dsp->u0[1][j];
    dsp->v0[1][j] = dsp->coss[1]*dsp->u0[0][j] + dsp->sins[1]*dsp->u0[1][j];
  }
  norm(dsp->v0[0], n);
  norm(dsp->v0[1], n);
  
/* calculate second princ dirs by projecting v0 onto v1 */
  for (j=0; j<n; j++)
  {
    dsp->v1[0][j] = inner_prod(dsp->v0[0], dsp->u1[0], n) * dsp->u1[0][j] +
          inner_prod(dsp->v0[0], dsp->u1[1], n) * dsp->u1[1][j] ;
    dsp->v1[1][j] = inner_prod(dsp->v0[1], dsp->u1[0], n) * dsp->u1[0][j] +
          inner_prod(dsp->v0[1], dsp->u1[1], n) * dsp->u1[1][j] ;
  }

}

void
princ_angs(displayd *dsp, cpaneld *cpanel)
{
  gint j, k, n=xg.ncols;
  gfloat tmpf1, tmpf2;
  gfloat tol2 = 0.01;
  static gfloat angle_tol = 0.001;

/*
 * if the norms vanish need to regenerate another basis and new
 * princ dirs, otherwise no rotation occurs - to be put in code
 * where new basis is generated between consecutive tours
*/

  if (calc_norm(dsp->v1[0], n) < angle_tol)
    for (j=0; j<n; j++)
      dsp->v1[0][j] = dsp->u1[0][j];
  if (calc_norm(dsp->v1[1], n) < angle_tol)
    for (j=0; j<n; j++)
      dsp->v1[1][j] = dsp->u1[1][j];
  norm(dsp->v1[0], n);
  norm(dsp->v1[1], n);

/* calculate principle angles for movement, and calculate stepsize */
/* put in check for outside of cos domain, ie >1, <-1. */
  tmpf1 = inner_prod(dsp->v0[0], dsp->v1[0], n);
  tmpf2 = inner_prod(dsp->v0[1], dsp->v1[1], n);
  if (tmpf1 > 1.0)
    tmpf1 = 1.0;
  else if (tmpf1 < -1.0)
    tmpf1 = -1.0;
  if (tmpf2 > 1.0)
    tmpf2 = 1.0;
  else if (tmpf2 < -1.0)
    tmpf2 = -1.0;
  dsp->tau[0] = (gfloat) acos((gdouble) tmpf1);
  dsp->tau[1] = (gfloat) acos((gdouble) tmpf2);

  if ((dsp->tau[0] < tol2) && (dsp->tau[1] < tol2))
  {
    zero_tau(dsp);
    k = 0;
    for (j=0; j<dsp->ntour_vars; j++)
    {
      dsp->u1[0][j] = dsp->u1[1][j] = 0.0;
      if (j == dsp->tour_vars[k])
      {
        k++;
      }
      else
        dsp->u0[0][j] = dsp->u0[1 ][j] = 0.;
    }

  }

  if (dsp->tau[0] < tol2)
    dsp->tau[0] = 0.;
  if (dsp->tau[1] < tol2)
    dsp->tau[1] = 0.;
  zero_tinc(dsp);
  
  dsp->dv = sqrt(dsp->tau[0]*dsp->tau[0] + dsp->tau[1]*dsp->tau[1]);
  dsp->delta = cpanel->tour_step/dsp->dv;
  
  /* orthogonalize v1 wrt v0 by Gram-Schmidt and normalize */
  if (dsp->tau[0] > angle_tol)
    gram_schmidt(dsp->v0[0], dsp->v1[0], n);
  if (dsp->tau[1] > angle_tol)
    gram_schmidt(dsp->v0[1], dsp->v1[1], n);
}

void
geodesic_tour_path(displayd *dsp, cpaneld *cpanel) {
  basis_dir_ang(dsp);
  princ_dirs(dsp);
  princ_angs(dsp, cpanel);
}

void
determine_endbasis_and_path(displayd *dsp, cpaneld *cpanel)
{
  /* general scan tour */
    if (!check_proximity(dsp->u, dsp->u0, xg.ncols))
    {
      init_basis(dsp);
    }
    gt_basis(dsp);

/*
 * Calculate path.
*/
  zero_tau(dsp);
  geodesic_tour_path(dsp, cpanel);
}

void
increment_tour(displayd *dsp)
{
  display_tailpipe (dsp);

  /*  tour_var_lines(xg);*/
}

gint
check_tour(displayd *dsp, cpaneld *cpanel)
{
  gint return_val = 1;

  dsp->tinc[0] += (dsp->delta * dsp->tau[0]);
  dsp->tinc[1] += (dsp->delta * dsp->tau[1]);

  if ((dsp->tinc[0] > cpanel->tour_path_len*dsp->tau[0]) || 
    (dsp->tinc[1] > cpanel->tour_path_len*dsp->tau[1]) ||
    ((dsp->tinc[0] == cpanel->tour_path_len*dsp->tau[0]) && 
    (dsp->tinc[1] == cpanel->tour_path_len*dsp->tau[1])))
      return_val = 0;

  return (return_val);
}

void
run_tour (displayd *dsp)
{
  cpaneld *cpanel = &dsp->cpanel;

/*
 * This controls the tour, effectively. It checks if we are at the end of 
 * the current path, if not then increments the tour a step.
*/
  if (check_tour(dsp, cpanel)) {
    increment_tour(dsp);
  }
/*
 * Calculation of new path for various different modes.
*/
  else {
/*
 * Do a final projection into the ending plane if just finished a tour
*/
      do_last_increment(dsp, cpanel);
      determine_endbasis_and_path(dsp, cpanel);
  }
}

void 
tour_do_step (displayd *dsp) {
  run_tour (dsp);
}

void *
tour_thread (void *args)
{
  displayd *dsp = current_display;
  cpaneld *cpanel = &dsp->cpanel;
g_printerr ("(tour_thread)\n");

  while (true) {
    if (mode_get () == TOUR2D && !cpanel->is_tour_paused) {
      gdk_threads_enter ();
      run_tour (current_display);
      gdk_threads_leave ();
    }
  }

  return (NULL);
}

static int tour_idle = 0;
gint
tour_idle_func (gpointer idled)
{
  displayd *dsp = current_display;
  cpaneld *cpanel = &dsp->cpanel;
  gboolean doit = !cpanel->is_tour_paused;

  if (doit)
    run_tour (current_display);

  return (doit);
}

void tour_func (gboolean state)
{
/*-- gtk_idle_add --*/
/*
  if (state)
    tour_idle = gtk_idle_add_priority (G_PRIORITY_LOW,
                                       (GtkFunction) tour_idle_func, NULL);
  else {
    gtk_idle_remove (tour_idle);
    tour_idle = 0;
  }
*/

   if (state)
     tour_idle = gtk_timeout_add (40, tour_idle_func, NULL);
   else
     gtk_timeout_remove (tour_idle);
}
