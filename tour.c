/* tour.c */

#include "stdlib.h"

#include <gtk/gtk.h>
#ifdef USE_STRINGS_H
#include <strings.h>
#endif
#include <math.h>
#include <stdlib.h>

#include "vars.h"
#include "externs.h"

void
alloc_tour (displayd *dsp, ggobid *gg)
{
  datad *d = dsp->d;
  gint nc = d->ncols;

  arrayf_init(&dsp->u0);
  arrayf_alloc(&dsp->u0, nc, nc);

  arrayf_init(&dsp->u1);
  arrayf_alloc(&dsp->u1, nc, nc);

  arrayf_init(&dsp->u);
  arrayf_alloc(&dsp->u, nc, nc);

  arrayf_init(&dsp->uold);
  arrayf_alloc(&dsp->uold, nc, nc);

  arrayf_init(&dsp->v0);
  arrayf_alloc(&dsp->v0, nc, nc);

  arrayf_init(&dsp->v1);
  arrayf_alloc(&dsp->v1, nc, nc);

  arrayf_init(&dsp->v);
  arrayf_alloc(&dsp->v, nc, nc);

  arrayf_init(&dsp->uvevec);
  arrayf_alloc(&dsp->uvevec, nc, nc);

  arrayf_init(&dsp->tv);
  arrayf_alloc(&dsp->tv, nc, nc);

  vectori_init(&dsp->tour_vars);
  vectori_alloc(&dsp->tour_vars, nc);
  vectorf_init(&dsp->lambda);
  vectorf_alloc(&dsp->lambda, nc);
  vectorf_init(&dsp->tau);
  vectorf_alloc(&dsp->tau, nc);
  vectorf_init(&dsp->tinc);
  vectorf_alloc(&dsp->tinc, nc);

/* ncols x ncols */
/* first index is the projection dimension, second index is the num vars */

  /*  dsp->u0 = (gfloat **) g_malloc(nc * sizeof(gfloat *));
  for (i=0; i<nc; i++)
    dsp->u0[i] = (gfloat *) g_malloc0(nc * sizeof(gfloat));
    
  dsp->u1 = (gfloat **) g_malloc(nc * sizeof(gfloat *));
  for (i=0; i<nc; i++)
    dsp->u1[i] = (gfloat *) g_malloc0(nc * sizeof(gfloat));

  dsp->u = (gfloat **) g_malloc(nc * sizeof(gfloat *));
  for (i=0; i<nc; i++)
    dsp->u[i] = (gfloat *) g_malloc0(nc * sizeof(gfloat));

  dsp->uold = (gfloat **) g_malloc(nc * sizeof(gfloat *));
  for (i=0; i<nc; i++)
    dsp->uold[i] = (gfloat *) g_malloc0(nc * sizeof(gfloat));

  dsp->v0 = (gfloat **) g_malloc(nc * sizeof(gfloat *));
  for (i=0; i<nc; i++)
    dsp->v0[i] = (gfloat *) g_malloc0(nc * sizeof(gfloat));

  dsp->v1 = (gfloat **) g_malloc(nc * sizeof(gfloat *));
  for (i=0; i<nc; i++)
    dsp->v1[i] = (gfloat *) g_malloc0(nc * sizeof(gfloat));

  dsp->v = (gfloat **) g_malloc(nc * sizeof(gfloat *));
  for (i=0; i<nc; i++)
    dsp->v[i] = (gfloat *) g_malloc0(nc * sizeof(gfloat));

  dsp->uvevec = (gfloat **) g_malloc(nc * sizeof(gfloat *));
  for (i=0; i<nc; i++)
    dsp->uvevec[i] = (gfloat *) g_malloc0(nc * sizeof(gfloat));

  dsp->tv = (gfloat **) g_malloc(nc * sizeof(gfloat *));
  for (i=0; i<nc; i++)
    dsp->tv[i] = (gfloat *) g_malloc0(nc * sizeof(gfloat));

  dsp->tour_vars = (gint *) g_malloc0(nc * sizeof(gint));

  dsp->lambda = (gfloat *) g_malloc0(nc * sizeof(gfloat));
  dsp->tau = (gfloat *) g_malloc0(nc * sizeof(gfloat));
  dsp->tinc = (gfloat *) g_malloc0(nc * sizeof(gfloat));*/

}

/*-- eliminate the nc columns contained in *cols --*/
void
tour_realloc_down (gint nc, gint *cols, displayd *dsp, ggobid *gg)
{
  arrayf_delete_cols (&dsp->u0, nc, cols);
  arrayf_delete_cols (&dsp->u1, nc, cols);
  arrayf_delete_cols (&dsp->u, nc, cols);
  arrayf_delete_cols (&dsp->uold, nc, cols);
  arrayf_delete_cols (&dsp->v0, nc, cols);
  arrayf_delete_cols (&dsp->v1, nc, cols);
  arrayf_delete_cols (&dsp->v, nc, cols);
  arrayf_delete_cols (&dsp->uvevec, nc, cols);
  arrayf_delete_cols (&dsp->tv, nc, cols);

  vectori_delete_els (&dsp->tour_vars, nc, cols);
  vectorf_delete_els (&dsp->lambda, nc, cols);
  vectorf_delete_els (&dsp->tau, nc, cols);
  vectorf_delete_els (&dsp->tinc, nc, cols);
}

/*-- append columns for a total of nc columns --*/
void
tour_realloc_up (gint nc, displayd *dsp, ggobid *gg)
{
  arrayf_add_cols (&dsp->u0, nc);
  arrayf_add_cols (&dsp->u1, nc);
  arrayf_add_cols (&dsp->u, nc);
  arrayf_add_cols (&dsp->uold, nc);
  arrayf_add_cols (&dsp->v0, nc);
  arrayf_add_cols (&dsp->v1, nc);
  arrayf_add_cols (&dsp->v, nc);
  arrayf_add_cols (&dsp->uvevec, nc);
  arrayf_add_cols (&dsp->tv, nc);

  vectori_realloc (&dsp->tour_vars, nc);
  vectorf_realloc (&dsp->lambda, nc);
  vectorf_realloc (&dsp->tau, nc);
  vectorf_realloc (&dsp->tinc, nc);
}

void
free_tour(displayd *dsp)
{
  /*  gint k;*/
  /*  datad *d = dsp->d;*/
  /*  gint nc = d->ncols;*/

  /*  g_free((gpointer) dsp->tour_vars);
  g_free((gpointer) dsp->lambda);
  g_free((gpointer) dsp->tau);
  g_free((gpointer) dsp->tinc);*/

  vectori_free(&dsp->tour_vars);
  vectorf_free(&dsp->lambda);
  vectorf_free(&dsp->tau);
  vectorf_free(&dsp->tinc);

  arrayf_free(&dsp->u0, 0, 0);
  arrayf_free(&dsp->u1, 0, 0);
  arrayf_free(&dsp->u, 0, 0);
  arrayf_free(&dsp->uold, 0, 0);

  arrayf_free(&dsp->v0, 0, 0);
  arrayf_free(&dsp->v1, 0, 0);
  arrayf_free(&dsp->v, 0, 0);

  arrayf_free(&dsp->uvevec, 0, 0);
  arrayf_free(&dsp->tv, 0, 0);

  /*  for (k=0; k<nc; k++)
    g_free((gpointer) dsp->u[k]);
  g_free((gpointer) dsp->u);
  for (k=0; k<nc; k++)
    g_free((gpointer) dsp->u0[k]);
  g_free((gpointer) dsp->u0);
  for (k=0; k<nc; k++)
    g_free((gpointer) dsp->u1[k]);
  g_free((gpointer) dsp->u1);
  for (k=0; k<nc; k++)
    g_free((gpointer) dsp->uold[k]);
  g_free((gpointer) dsp->uold);

  for (k=0; k<nc; k++)
    g_free((gpointer) dsp->v0[k]);
  g_free((gpointer) dsp->v0);
  for (k=0; k<nc; k++)
    g_free((gpointer) dsp->v1[k]);
  g_free((gpointer) dsp->v1);
  for (k=0; k<nc; k++)
    g_free((gpointer) dsp->v[k]);
  g_free((gpointer) dsp->v);
  for (k=0; k<nc; k++)
    g_free((gpointer) dsp->uvevec[k]);
  g_free((gpointer) dsp->uvevec);
  for (k=0; k<nc; k++)
    g_free((gpointer) dsp->tv[k]);
  g_free((gpointer) dsp->tv);*/

}

void
zero_tau (displayd *dsp, ggobid *gg) {
  gint k;
  datad *d = dsp->d;

  for (k=0; k<d->ncols; k++) {
    dsp->lambda.vals[k]  = 0.0;
    dsp->tau.vals[k]  = 0.0;
    dsp->tinc.vals[k] = 0.0;
  }
}

void
zero_tinc(displayd *dsp, ggobid *gg) {
  datad *d = dsp->d;
  gint k;

  for (k=0; k<d->ncols; k++) {
    dsp->tinc.vals[k] = 0.0;
  }
}

/* This function initializes the tour variables - it should only be
   called more than once, when a new tour is started since a new
   subset of variable might be used, or when there is new data. */

void 
cpanel_tour_init (cpaneld *cpanel, ggobid *gg) {
    cpanel->tour_paused_p = false;
    cpanel->tour_local_scan_p = false;
    cpanel->tour_stepping_p = false;
    cpanel->tour_backtracking_p = false;
    cpanel->tour_step = TOURSTEP0;
    cpanel->tour_ls_dir = TOUR_LS_IN;
    cpanel->tour_path_len = 1.;
}

void 
display_tour_init (displayd *dsp, ggobid *gg) {
  gint i, j;
  datad *d = dsp->d;
  cpaneld *cpanel = &dsp->cpanel;
  gint nc = d->ncols;

  alloc_tour(dsp, gg);
 
    /* Initialize starting subset of active variables */
  dsp->ntour_vars = nc;
  for (j=0; j<nc; j++)
    dsp->tour_vars.vals[j] = j;
  /*  dsp->tour_vars.vals[0] = 0;
  dsp->tour_vars.vals[1] = 1;
  dsp->tour_vars.vals[2] = 2;*/

  /* declare starting base as first p chosen variables */
  for (i=0; i<nc; i++)
    for (j=0; j<nc; j++)
      dsp->u0.vals[i][j] = dsp->u1.vals[i][j] = dsp->u.vals[i][j] = 
        dsp->uold.vals[i][j] =
        dsp->v0.vals[i][j] = dsp->v1.vals[i][j] = 0.0;

  for (i=0; i<nc; i++)
  {
    dsp->u1.vals[i][dsp->tour_vars.vals[i]] =
      dsp->u0.vals[i][dsp->tour_vars.vals[i]] = 
      dsp->u.vals[i][dsp->tour_vars.vals[i]] =
      dsp->v0.vals[i][dsp->tour_vars.vals[i]] = 
      dsp->v1.vals[i][dsp->tour_vars.vals[i]] = 1.0;
  }

  /*  dsp->ts[0] = 0;
  dsp->ts[1] = M_PI_2;
  dsp->coss[0] = 1.0;
  dsp->coss[1] = 0.0;
  dsp->sins[1] = 1.0;
  dsp->sins[0] = 0.0;
  dsp->icoss[0] = PRECISION2;
  dsp->icoss[1] = 0;
  dsp->isins[1] = PRECISION2;
  dsp->isins[0] = 0;*/

  zero_tau(dsp, gg);
  dsp->dv = 1.0;
  dsp->delta = cpanel->tour_step*M_PI_2/10.0;
  dsp->tour_nsteps = 1; 
  dsp->tour_stepcntr = 1;

  dsp->tour_idled = 0;
  dsp->tour_get_new_target = true;
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
inner_prod(gfloat *x1, gfloat *x2, gint n)
{
  gdouble xip;
  gint j;

  xip = 0.;
  for (j=0; j<n; j++)
    xip = xip + (gdouble)x1[j]*(gdouble)x2[j];
  return((gfloat)xip);
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

/* checks columns of matrix are orthonormal */
gboolean checkcolson(gfloat **ut, gint nc, gint nd) {
  gint j, k;
  gfloat tol = 0.01;
  gboolean ok = true;

  for (j=0; j<nd; j++) {
    if (fabs(1.-calc_norm(ut[j], nc)) > tol) {
      ok = false;
      return(ok);
    }
  }

  for (j=0; j<nd-1; j++) {
    for (k=j+1; k<nd; k++) {
      if (fabs(inner_prod(ut[j],ut[k],nc)) > tol) {
        ok = false;
        return(ok);
      }
    }
  }

  return(ok);
}

/* matrix multiplication UV */
gboolean matmult_uv(gfloat **ut, gfloat **vt, gint ur, gint uc, 
  gint vr, gint vc, gfloat **ot) {
  gint i, j, k;
  gboolean ok = true;

  if (uc != vr) {
    ok = false;
    return(ok);
  }

  for (j=0; j<ur; j++) {
    for (k=0; k<vc; k++) {
      ot[k][j] = 0.0;
      for (i=0; i<uc; i++) {
        ot[k][j] += ut[i][j]*vt[k][i];
      }
    }
  }

  return(ok);
}

/* matrix multiplication U'V */
gboolean matmult_utv(gfloat **ut, gfloat **vt, gint ur, gint uc, 
  gint vr, gint vc, gfloat **ot) {
  gint i, j, k;
  gboolean ok = true;

  if (ur != vr) {
    ok = false;
    return(ok);
  }

  for (j=0; j<uc; j++) {
    for (k=0; k<vc; k++) {
      ot[k][j] = 0.0;
      for (i=0; i<ur; i++) {
        ot[k][j] += ut[j][i]*vt[k][i];
      }
    }
  }

  return(ok);
}

/* matrix multiplication UV */
gboolean matmult_uvt(gfloat **ut, gfloat **vt, gint ur, gint uc, 
  gint vr, gint vc, gfloat **ot) {
  gint i, j, k;
  gboolean ok = true;

  if (uc != vc) {
    ok = false;
    return(ok);
  }

  for (j=0; j<ur; j++) {
    for (k=0; k<vr; k++) {
      ot[k][j] = 0.0;
      for (i=0; i<uc; i++) {
        ot[k][j] += ut[i][j]*vt[i][k];
      }
    }
  }

  return(ok);
}

/* copy matrix */
void copy_mat(gfloat **ot, gfloat **it, gint nr, gint nc) {
  gint j, k;

  for (j=0; j<nr; j++)
    for (k=0; k<nc; k++)
      ot[k][j] = it[k][j];
}

/* orthonormalize x2 on x2, just by diagonals should be enough for this
   tour code */
void
matgram_schmidt(gfloat **x1, gfloat **x2, gint nr, gint nc)
{
  gint j, k;
  gfloat ip;

  for (j=0; j<nc; j++) {
    ip = inner_prod(x1[j], x2[j], nr);
    for (k=0; k<nr; k++)
      x2[j][k] = x2[j][k] - ip*x1[j][k];
    norm(x2[j], nr);
  }

}

void
eigen_clear (displayd *dsp)
{
  datad *d = dsp->d;
  gint nc = d->ncols;
  gint j, k;

  for (j=0; j<nc; j++) {
    for (k=0; k<nc; k++) {
      dsp->v0.vals[j][k] = 0.;
      dsp->v1.vals[j][k] = 0.;
    }
    dsp->lambda.vals[j] = 0.;
    dsp->tau.vals[j] = 0.;
    dsp->tinc.vals[j] = 0.;
  }

}

/* u0 = starting projection
 * u1 = target projection
 * u = interpolated projection
 * nc = num vars
 * nd = proj dim
 */
void path(displayd *dsp, gint nd) {

  datad *d = dsp->d;
  gint nc = d->ncols;
  gint i, j, k, rank;
  gdouble tol = 0.01;
  gdouble tmpd1, tmpd2, tmpd;
  gboolean doit = true;
  paird *pairs = (paird *) g_malloc (nd * sizeof (paird));
  gfloat *e = (gfloat *) g_malloc (nd * sizeof (gfloat));
  gint dI; /* dimension of intersection of base pair */
  gfloat **ptinc = (gfloat **) g_malloc (2 * sizeof (gfloat *));

  /*  for (i=0; i<d->ncols; i++)
    printf("%f ",dsp->u0.vals[0][i]);
  printf("\n");

  printf("u1 \n");
  for (i=0; i<d->ncols; i++)
    printf("%f ",dsp->u1[0][i]);
  printf("\n");*/

  /* 2 is hard-wired because it relates to cos, sin
                         and nothing else. */
  for (i=0; i<2; i++) 
    ptinc[i] = (gfloat *) g_malloc (nd * sizeof (gfloat));
    
  /* Check that u0 and u1 are both orthonormal. */
  if (!checkcolson(dsp->u0.vals, nc, nd)) {
    printf("Columns of u0 are not orthonormal");
    doit = false;
  }
  if (!checkcolson(dsp->u1.vals, nc, nd)) {
    printf("Columns of u1 are not orthonormal");
    doit = false;
  }

  /* Do SVD of u0'u1: span(u0,u1).*/
  if (doit) {
    if (!matmult_utv(dsp->u0.vals, dsp->u1.vals, nc, nd, nc, nd, dsp->tv.vals))
      printf("#cols != #rows in the two matrices");
      
      dsvd(dsp->tv.vals, nd, nd, dsp->lambda.vals, dsp->v1.vals);

      copy_mat(dsp->v0.vals, dsp->tv.vals, nd, nd);

      /* we want eigenvalues in order from largest to smallest, ie
         smallest angle to largest angle */
      /* only do this if nd > 2 otherwise it is easy */
      if (nd > 2) {
        /*-- obtain ranks to use in sorting eigenvals and eigenvec --*/
        for (i=0; i<nd; i++) {
          pairs[i].f = (gfloat) dsp->lambda.vals[i];
          pairs[i].indx = i;
        }
        qsort ((gchar *) pairs, (size_t) dsp->lambda.vals, sizeof (paird), pcompare);

       /*-- sort the eigenvalues and eigenvectors into temporary arrays --*/
       for (i=0; i<nd; i++) {
         k = (nd - i) - 1;  /*-- to reverse the order --*/
         rank = pairs[i].indx;
         e[k] = dsp->lambda.vals[rank];
         for (j=0; j<nd; j++)
           dsp->tv.vals[j][k] = dsp->v0.vals[j][rank];
       }

       /*-- copy the sorted eigenvalues and eigenvectors back --*/
       for (i=0; i<nd; i++) {
         dsp->lambda.vals[i] = e[i];
         for (j=0; j<nd; j++)
           dsp->v0.vals[j][i] = dsp->tv.vals[j][i];
       }
  
       /* need to do v1 too */
       for (i=0; i<nd; i++) {
         k = (nd - i) - 1;  /*-- to reverse the order --*/
         rank = pairs[i].indx;
         for (j=0; j<nd; j++)
           dsp->tv.vals[j][k] = dsp->v1.vals[j][rank];
       }
       for (i=0; i<nd; i++) {
         for (j=0; j<nd; j++)
           dsp->v1.vals[j][i] = dsp->tv.vals[j][i];
       }
     }
     else if (nd == 2) {
       if (dsp->lambda.vals[1] > dsp->lambda.vals[0]) {
         e[0] = dsp->lambda.vals[1];
         dsp->lambda.vals[1] = dsp->lambda.vals[0];
         dsp->lambda.vals[0] = e[0];
         for (j=0; j<nd; j++) {
           dsp->tv.vals[j][1] = dsp->v0.vals[j][0];
           dsp->v0.vals[j][0] = dsp->v0.vals[j][1];
           dsp->v0.vals[j][1] = dsp->tv.vals[j][1];
           dsp->tv.vals[j][1] = dsp->v1.vals[j][0];
           dsp->v1.vals[j][0] = dsp->v1.vals[j][1];
           dsp->v1.vals[j][1] = dsp->tv.vals[j][1];
	 }

       }
     }

     /* copy the eigenvectors into a permanent structure. need this
        for reprojection */
     copy_mat(dsp->uvevec.vals, dsp->v0.vals, nd, nd);

/*      SingularValueDecomposition svd = temp.svd();
      Va = svd.getU();
      Vz = svd.getV();
      lambda = svd.getSingularValues();*/
     /* Returns the sv's in order largest to smallest.*/

      /*  Check span of <Fa,Fz>
       If dI=ndim we should stop here, and set Ft to be Fa but this is
       equivalent to setting the lambda's to be 1.0 at this stage.*/
      dI = 0;
      for (i=0; i<nd; i++) {
	if (dsp->lambda.vals[i] > 1.0-tol) {
	  dI++;
	  dsp->lambda.vals[i] = 1.0;
	}
      }
    
      /*  Compute principal angles */
      for (i=0; i<nd; i++)
	dsp->tau.vals[i] = (gfloat) acos((gdouble) dsp->lambda.vals[i]);
      
      /*  Calculate principal directions */
      if (nd > dI) {
        copy_mat(dsp->tv.vals, dsp->v0.vals, nc, nd);
        if (!matmult_uv(dsp->u0.vals, dsp->tv.vals, nc, nd, nd, nd, dsp->v0.vals))
          printf("Could not multiply u and v, cols!=rows \n");
        copy_mat(dsp->tv.vals, dsp->v1.vals, nc, nd);
        if (!matmult_uv(dsp->u1.vals, dsp->tv.vals, nc, nd, nd, nd, dsp->v1.vals))
          printf("Could not multiply u and v, cols!=rows \n");
	/*  Orthonormalize v1 on v0*/
        matgram_schmidt(dsp->v0.vals, dsp->v1.vals, nc, nd);

      }
      else {
        copy_mat(dsp->v0.vals, dsp->u0.vals, nc, nd);
        copy_mat(dsp->v1.vals, dsp->u0.vals, nc, nd);
	for (i=0; i<nd; i++)
	  dsp->tau.vals[i] = 0.0;
      }

      /* Construct current basis*/
      for (i=0; i<nd; i++)
	dsp->tinc.vals[i]=0.0;
      for (i=0; i<nd; i++) {
	ptinc[0][i] = (gfloat) cos((gdouble) dsp->tinc.vals[i]);
	ptinc[1][i] = (gfloat) sin((gdouble) dsp->tinc.vals[i]);
      }

      for (i=0; i<nd; i++) {
	tmpd1 = ptinc[0][i];
	tmpd2 = ptinc[1][i];
	for (j=0; j<nc; j++) {
	  tmpd = dsp->v0.vals[i][j]*tmpd1 + dsp->v1.vals[i][j]*tmpd2;
          dsp->v.vals[i][j] = tmpd;
	}
      }

      matmult_uvt(dsp->v.vals, dsp->uvevec.vals, nc, nd, nd, nd, dsp->u.vals);

      /* orthonormal to correct round-off errors */
      for (i=0; i<nd; i++)
        norm(dsp->u.vals[i], nc); 

      for (k=0; k<nd-1; k++)
        for (j=k+1; j<nd; j++)
          gram_schmidt(dsp->u.vals[k], dsp->u.vals[j], nc);

      /* Calculate Euclidean norm of principal angles.*/
      dsp->dv = 0.0;
      for (i=0; i<nd; i++)
	dsp->dv += (dsp->tau.vals[i]*dsp->tau.vals[i]);
      dsp->dv = (gfloat)sqrt((gdouble)dsp->dv);

      /* Reset increment counters.*/
      dsp->tour_nsteps = (gint) floor((gdouble)(dsp->dv/dsp->delta))+1;
      /*      for (i=1; i<nd; i++) {
	if ((gint) floor((gdouble)(dG/delta)) < nsteps) 
	  nsteps = (gint) floor((gdouble)(dG/delta));
      }*/
      dsp->tour_stepcntr = 0;
    }
    else {
      for (i=0; i<nd; i++)
	dsp->tau.vals[i] = 0.0;
      copy_mat(dsp->v0.vals, dsp->u0.vals, nc, nd);
      copy_mat(dsp->v1.vals, dsp->u0.vals, nc, nd);
      dsp->tour_nsteps = 0;
    }

/* free temporary arrays */
  g_free ((gpointer) pairs);
  for (j=0; j<2; j++)
    g_free (ptinc[j]);
  g_free (ptinc);
  g_free (e);

}

/* Generate the interpolation frame. No preprojection is done */
void tour_reproject(displayd *dsp, gint nd)
{
  datad *d = dsp->d;
  gint nc = d->ncols;
  gint i, j, k;
  gdouble tmpd1, tmpd2, tmpd;
  gfloat **ptinc = (gfloat **) g_malloc (2 * sizeof (gfloat *));

  for (i=0; i<2; i++)
    ptinc[i] = (gfloat *) g_malloc (nd * sizeof (gfloat));

  for (i=0; i<nd; i++) {
    ptinc[0][i] = (gfloat) cos((gdouble) dsp->tinc.vals[i]);
    ptinc[1][i] = (gfloat) sin((gdouble) dsp->tinc.vals[i]);
  }

  for (i=0; i<nd; i++) {
    tmpd1 = ptinc[0][i];
    tmpd2 = ptinc[1][i];
    for (j=0; j<nc; j++) {
      tmpd = dsp->v0.vals[i][j]*tmpd1 + dsp->v1.vals[i][j]*tmpd2;
      dsp->v.vals[i][j] = tmpd;
    }
  }

  matmult_uvt(dsp->v.vals, dsp->uvevec.vals, nc, nd, nd, nd, dsp->u.vals);

  /* orthonormal to correct round-off errors */
  for (i=0; i<nd; i++)
    norm(dsp->u.vals[i], nc); 

  for (k=0; k<nd-1; k++)
    for (j=k+1; j<nd; j++)
      gram_schmidt(dsp->u.vals[k], dsp->u.vals[j], nc);

  /*    printf("v, u \n");
    for (i=0; i<d->ncols; i++) {
      for (j=0; j<2; j++)
        printf("%f %f ",dsp->v.vals[j][i],dsp->u.vals[j][i]);
      printf("\n");
    }*/

}

/* this routine increments the interpolation */
void
increment_tour(displayd *dsp, gint nd)
{
  int i;
  gboolean attheend = false;

  dsp->tour_stepcntr++;

  /*  printf("tinc ");
  for (i=0; i<nd; i++)
    printf("%f ",dsp->tinc[i]);
  printf("\n");*/

  for (i=0; i<nd; i++) 
    if (dsp->tinc.vals[i] > dsp->tau.vals[i]) {
      attheend = true;
      dsp->tour_nsteps = dsp->tour_stepcntr;
    }

  if (attheend || dsp->tour_nsteps == 0 || 
      dsp->tour_nsteps == dsp->tour_stepcntr) {
    for (i=0; i<nd; i++)
      dsp->tinc.vals[i] = dsp->tau.vals[i];
  }
  else {
    for (i=0; i<nd; i++)
      dsp->tinc.vals[i] += dsp->delta*dsp->tau.vals[i]/dsp->dv;
  }
}

gboolean
reached_target(displayd *dsp) {
  gboolean arewethereyet = false;

  if (dsp->tour_nsteps == 0 || dsp->tour_stepcntr == dsp->tour_nsteps)
    arewethereyet = true;

  return(arewethereyet);
}

void
do_last_increment(displayd *dsp, gint nd)
{
  int j;

  for (j=0; j<nd; j++)
    dsp->tinc.vals[j] = dsp->tau.vals[j];

}

void speed_set (gint slidepos, ggobid *gg) {

  displayd *dsp = gg->current_display; 
  cpaneld *cpanel = &dsp->cpanel;
  gfloat fracpath;

  if (slidepos < 5)
  {
    /*   cpanel->is_tour_paused = true;*/
    cpanel->tour_step = 0.0;
    /* probably need to turn off thread. */
  }
  else
  {
     /*    if (!cpanel->is_tour_paused) {
	       cpanel->tour_paused = false;
     }*/

    /*
     * To cause tour to speed up wildly at the right of the
     * scrollbar range.
    */
    if (slidepos < 50)
      cpanel->tour_step = ((float) slidepos - 5.) / 2000. ;
    else if ((slidepos >= 50) && (slidepos < 90))
      cpanel->tour_step = pow((double)(slidepos-50)/100.,(double)1.5) + 0.0225;
    else
      /*      cpanel->tour_step = pow((double)(slidepos-50)/100.,(double)10) + 0.1868;*/
      cpanel->tour_step = sqrt((double)(slidepos-50)) + 0.1868;
  }
  /*  dsp->delta = cpanel->tour_step/dsp->dv;*/
  dsp->delta = cpanel->tour_step*M_PI_2/10.0;

  fracpath = dsp->tour_stepcntr/dsp->tour_nsteps;

  dsp->tour_nsteps = (gint) floor((gdouble)(dsp->dv/dsp->delta))+1;
  dsp->tour_stepcntr = (gint) floor(fracpath*dsp->tour_nsteps);
}

void
gt_basis (displayd *dsp, ggobid *gg, gint nd)
/*
 * Generate d random p dimensional vectors to form new ending basis
*/
{
  datad *d = dsp->d;
  gint j, k, check = 1, nvals = dsp->ntour_vars*nd;
  gdouble frunif[2];
  gdouble r, fac, frnorm[2];

/*
 * Method suggested by Press, Flannery, Teukolsky, and Vetterling (1986)
 * "Numerical Recipes" p.202-3, for generating random normal variates .
*/

  /* Zero out u1 before filling; this might fix a bug we are
     encountering with returning from a receive tour.
  */
  for (j=0; j<d->ncols; j++)
    for (k=0; k<nd; k++)
      dsp->u1.vals[k][j] = 0.0 ;

  if (dsp->ntour_vars > 2) {
      for (j=0; j<nvals/2+1; j++) {
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
          if (nd == 1) {
            if (2*j <= dsp->ntour_vars) {
              dsp->u1.vals[0][dsp->tour_vars.vals[2*j]] = (gfloat) frnorm[0];
              if (2*j+1 < dsp->ntour_vars) 
                dsp->u1.vals[0][dsp->tour_vars.vals[2*j+1]] = (gfloat) frnorm[1];
	    }
	  }
          else if (nd == 2) {
            if (j <= dsp->ntour_vars) {
              dsp->u1.vals[0][dsp->tour_vars.vals[j]] = (gfloat) frnorm[0];
              dsp->u1.vals[1][dsp->tour_vars.vals[j]] = (gfloat) frnorm[1];
	    }
	  }
      }
    for (k=0; k<nd; k++)
      norm(dsp->u1.vals[k], d->ncols);

    /*
     * Orthogonalize the basis on the first using Gram-Schmidt
    */
    if (nd > 1) {
      for (k=0; k<nd-1; k++)
        for (j=k+1; j<nd; j++)
          gram_schmidt(dsp->u1.vals[k], dsp->u1.vals[j], d->ncols);
    }
  }
  else
  {
    for (k=0; k<nd; k++) 
      dsp->u1.vals[k][dsp->tour_vars.vals[k]] = 1.;
  }
}
