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
zero_tau (vector_f tau, gint nd) {

  gint k;

  for (k=0; k<nd; k++) 
    tau.els[k] = 0.0;
}

void
zero_tinc(vector_f tinc, gint nd) {
  gint k;

  for (k=0; k<nd; k++) 
    tinc.els[k] = 0.0;
}

void
zero_lambda(vector_f lambda, gint nd) {
  gint k;

  for (k=0; k<nd; k++) 
    lambda.els[k] = 0.0;
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

/* orthonormalizes vector 2 on vector */
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

/* checks columns of matrix are orthonormal */
gboolean checkequiv(gfloat **u0, gfloat **u1, gint nc, gint nd) {
  gint j;
  gfloat tol = 0.0001;
  gboolean ok = true;

  for (j=0; j<nd; j++) {
    if (fabs(1.-inner_prod(u0[j], u1[j], nc)) < tol) {
      ok = false;
      /*      printf("checkequiv %f \n",inner_prod(u0[j], u1[j], nc));*/
      return(ok);
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

/* copy matrix ot=out matrix, it=in matrix */
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
eigen_clear (array_f v0, array_f v1, vector_f lambda, vector_f tau, 
  vector_f tinc, gint nc)
{
  /*  datad *d = dsp->d;
  gint nc = d->ncols;*/
  gint j, k;

  for (j=0; j<nc; j++) {
    for (k=0; k<nc; k++) {
      v0.vals[j][k] = 0.;
      v1.vals[j][k] = 0.;
    }
    lambda.els[j] = 0.;
    tau.els[j] = 0.;
    tinc.els[j] = 0.;
  }

}

/* u0 = starting projection
 * u1 = target projection
 * u = interpolated projection
 * nc = num vars
 * nd = proj dim
 */
/*void path(displayd *dsp, gint nd) {*/
void path(array_f u0, array_f u1, array_f u, gint nc, gint nd, array_f v0,
  array_f v1, array_f v, vector_f lambda, array_f tv, array_f uvevec, 
  vector_f tau, vector_f tinc, gint *ns, gint *stcn, gfloat *pdv, 
  gfloat delta) {

  /*  datad *d = dsp->d;
  gint nc = d->ncols;*/
  gint i, j, k, rank;
  gdouble tol = 0.0001;
  gdouble tmpd1 = 0.0, tmpd2 = 0.0, tmpd =0.0;
  gboolean doit = true;
  paird *pairs = (paird *) g_malloc (nd * sizeof (paird));
  gfloat *e = (gfloat *) g_malloc (nd * sizeof (gfloat));
  gint dI; /* dimension of intersection of base pair */
  gfloat **ptinc = (gfloat **) g_malloc (2 * sizeof (gfloat *));

  gfloat dv = *pdv;
  gint nsteps = *ns;
  gint stepcntr = *stcn;

  zero_tau(tau, nd);
  zero_tinc(tinc, nd);
  zero_lambda(lambda, nd);
  
  /* 2 is hard-wired because it relates to cos, sin
                         and nothing else. */
  for (i=0; i<2; i++) 
    ptinc[i] = (gfloat *) g_malloc (nd * sizeof (gfloat));
    
  /* Check that u0 and u1 are both orthonormal. */
  if (!checkcolson(u0.vals, nc, nd)) {
    printf("Columns of u0 are not orthonormal");
    doit = false;
  }
  if (!checkcolson(u1.vals, nc, nd)) {
    printf("Columns of u1 are not orthonormal");
    doit = false;
  }

  /* Check that u0 and u1 are the same */
  if (!checkequiv(u0.vals, u1.vals, nc, nd)) {
    doit = false;
  }

  /* Do SVD of u0'u1: span(u0,u1).*/
  if (doit) {
    if (!matmult_utv(u0.vals, u1.vals, nc, nd, nc, nd, tv.vals))
      printf("#cols != #rows in the two matrices");
      
      dsvd(tv.vals, nd, nd, lambda.els, v1.vals);

      copy_mat(v0.vals, tv.vals, nd, nd);

      /* we want eigenvalues in order from largest to smallest, ie
         smallest angle to largest angle */
      /* only do this if nd > 2 otherwise it is easy */
      if (nd > 2) {
        /*-- obtain ranks to use in sorting eigenvals and eigenvec --*/
        for (i=0; i<nd; i++) {
          pairs[i].f = (gfloat) lambda.els[i];
          pairs[i].indx = i;
        }
        qsort ((gchar *) pairs, (size_t) lambda.els, sizeof (paird), pcompare);

       /*-- sort the eigenvalues and eigenvectors into temporary arrays --*/
       for (i=0; i<nd; i++) {
         k = (nd - i) - 1;  /*-- to reverse the order --*/
         rank = pairs[i].indx;
         e[k] = lambda.els[rank];
         for (j=0; j<nd; j++)
           tv.vals[j][k] = v0.vals[j][rank];
       }

       /*-- copy the sorted eigenvalues and eigenvectors back --*/
       for (i=0; i<nd; i++) {
         lambda.els[i] = e[i];
         for (j=0; j<nd; j++)
           v0.vals[j][i] = tv.vals[j][i];
       }
  
       /* need to do v1 too */
       for (i=0; i<nd; i++) {
         k = (nd - i) - 1;  /*-- to reverse the order --*/
         rank = pairs[i].indx;
         for (j=0; j<nd; j++)
           tv.vals[j][k] = v1.vals[j][rank];
       }
       for (i=0; i<nd; i++) {
         for (j=0; j<nd; j++)
           v1.vals[j][i] = tv.vals[j][i];
       }
     }
     else if (nd == 2) {
       if (lambda.els[1] > lambda.els[0]) {
         e[0] = lambda.els[1];
         lambda.els[1] = lambda.els[0];
         lambda.els[0] = e[0];

         for (j=0; j<nd; j++) {
           tv.vals[j][1] = v0.vals[j][0];
           v0.vals[j][0] = v0.vals[j][1];
           v0.vals[j][1] = tv.vals[j][1];
           tv.vals[j][1] = v1.vals[j][0];
           v1.vals[j][0] = v1.vals[j][1];
           v1.vals[j][1] = tv.vals[j][1];
	 }

       }
     }

     /* copy the eigenvectors into a permanent structure. need this
        for reprojection */
     copy_mat(uvevec.vals, v0.vals, nd, nd);

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
	if (lambda.els[i] > 1.0-tol) {
	  dI++;
	  lambda.els[i] = 1.0;
	}
      }
    
      /*  Compute principal angles */
      for (i=0; i<nd; i++)
	tau.els[i] = (gfloat) acos((gdouble) lambda.els[i]);
      
      /*  Calculate principal directions */
      if (nd > dI) {
        copy_mat(tv.vals, v0.vals, nc, nd);
        if (!matmult_uv(u0.vals, tv.vals, nc, nd, nd, nd, v0.vals))
          printf("Could not multiply u and v, cols!=rows \n");
        copy_mat(tv.vals, v1.vals, nc, nd);
        if (!matmult_uv(u1.vals, tv.vals, nc, nd, nd, nd, v1.vals))
          printf("Could not multiply u and v, cols!=rows \n");
	/*  Orthonormalize v1 on v0*/
        matgram_schmidt(v0.vals, v1.vals, nc, nd);

      }
      else {
        copy_mat(v0.vals, u0.vals, nc, nd);
        copy_mat(v1.vals, u0.vals, nc, nd);
	for (i=0; i<nd; i++)
	  tau.els[i] = 0.0;
      }

      /* Construct current basis*/
      for (i=0; i<nd; i++)
	tinc.els[i]=0.0;
      for (i=0; i<nd; i++) {
	ptinc[0][i] = (gfloat) cos((gdouble) tinc.els[i]);
	ptinc[1][i] = (gfloat) sin((gdouble) tinc.els[i]);
      }

      for (i=0; i<nd; i++) {
	tmpd1 = ptinc[0][i];
	tmpd2 = ptinc[1][i];
	for (j=0; j<nc; j++) {
	  tmpd = v0.vals[i][j]*tmpd1 + v1.vals[i][j]*tmpd2;
          v.vals[i][j] = tmpd;
	}
      }

      matmult_uvt(v.vals, uvevec.vals, nc, nd, nd, nd, u.vals);

      /* orthonormal to correct round-off errors */
      for (i=0; i<nd; i++)
        norm(u.vals[i], nc); 

      for (k=0; k<nd-1; k++)
        for (j=k+1; j<nd; j++)
          gram_schmidt(u.vals[k], u.vals[j], nc);

      /* Calculate Euclidean norm of principal angles.*/
      dv = 0.0;
      for (i=0; i<nd; i++)
	dv += (tau.els[i]*tau.els[i]);
      dv = (gfloat)sqrt((gdouble)dv);
      *pdv = dv;

      /* Reset increment counters.*/
      nsteps = (gint) floor((gdouble)(dv/delta))+1;
      /*      for (i=1; i<nd; i++) {
	if ((gint) floor((gdouble)(dG/delta)) < nsteps) 
	  nsteps = (gint) floor((gdouble)(dG/delta));
      }*/
      stepcntr = 0;
      *ns = nsteps;
      *stcn = stepcntr;
    }
    else {
      zero_tau(tau, nd);
      zero_tinc(tau, nd);
      zero_lambda(tau, nd);
      copy_mat(u.vals, u0.vals, nc, nd);
      copy_mat(v0.vals, u0.vals, nc, nd);
      copy_mat(v1.vals, u1.vals, nc, nd);
      copy_mat(uvevec.vals, u1.vals, nc, nd);
      copy_mat(v.vals, u0.vals, nc, nd);
      stepcntr = 0;
      nsteps = 0;
      dv = 0.0;
      *pdv = dv;
      *ns = nsteps;
      *stcn = stepcntr;

    }

/* free temporary arrays */
  g_free ((gpointer) pairs);
  for (j=0; j<2; j++)
    g_free (ptinc[j]);
  g_free (ptinc);
  g_free (e);

}

/* Generate the interpolation frame. No preprojection is done */
/*void tour_reproject(displayd *dsp, gint nd)*/
void tour_reproject(vector_f tinc, array_f v, array_f v0, array_f v1, 
  array_f u, array_f uvevec, gint nc, gint nd)
{
  /*  datad *d = dsp->d;
  gint nc = d->ncols;*/
  gint i, j, k;
  gdouble tmpd1, tmpd2, tmpd;
  gfloat **ptinc = (gfloat **) g_malloc (2 * sizeof (gfloat *));

  for (i=0; i<2; i++)
    ptinc[i] = (gfloat *) g_malloc (nd * sizeof (gfloat));

  for (i=0; i<nd; i++) {
    ptinc[0][i] = (gfloat) cos((gdouble) tinc.els[i]);
    ptinc[1][i] = (gfloat) sin((gdouble) tinc.els[i]);
  }

  for (i=0; i<nd; i++) {
    tmpd1 = ptinc[0][i];
    tmpd2 = ptinc[1][i];
    for (j=0; j<nc; j++) {
      tmpd = v0.vals[i][j]*tmpd1 + v1.vals[i][j]*tmpd2;
      v.vals[i][j] = tmpd;
    }
  }

  matmult_uvt(v.vals, uvevec.vals, nc, nd, nd, nd, u.vals);

  /* orthonormal to correct round-off errors */
  for (i=0; i<nd; i++)
    norm(u.vals[i], nc); 

  for (k=0; k<nd-1; k++)
    for (j=k+1; j<nd; j++)
      gram_schmidt(u.vals[k], u.vals[j], nc);

  for (j=0; j<2; j++)
    g_free (ptinc[j]);
  g_free (ptinc);

}

/* this routine increments the interpolation */
void
/*increment_tour(displayd *dsp, gint nd)*/
increment_tour(vector_f tinc, vector_f tau, gint *ns, gint *stcn, 
  gfloat dv, gfloat delta, gint nd)
{
  int i;
  gboolean attheend = false;

  gint nsteps = *ns;
  gint stepcntr = *stcn;

  stepcntr++;

  /*  printf("tinc ");
  for (i=0; i<nd; i++)
    printf("%f ",tinc[i]);
  printf("\n");*/

  for (i=0; i<nd; i++) 
    if (tinc.els[i] > tau.els[i]) {
      attheend = true;
      nsteps = stepcntr;
    }

  if (attheend || nsteps == 0 || 
      nsteps == stepcntr) {
    for (i=0; i<nd; i++)
      tinc.els[i] = tau.els[i];
  }
  else {
    for (i=0; i<nd; i++)
      tinc.els[i] += delta*tau.els[i]/dv;
  }

  *ns = nsteps;
  *stcn = stepcntr;
}

gboolean
reached_target(gint nsteps, gint stepcntr) {
  gboolean arewethereyet = false;

  if (nsteps == 0 || stepcntr == nsteps)
    arewethereyet = true;

  return(arewethereyet);
}

void
/*do_last_increment(displayd *dsp, gint nd)*/
do_last_increment(vector_f tinc, vector_f tau, gint nd)
{
  int j;

  for (j=0; j<nd; j++)
    tinc.els[j] = tau.els[j];

}

void speed_set (gint slidepos, gfloat *st, gfloat *dlt, gfloat dv, 
  gint *ns, gint *stcn) {

  /*  displayd *dsp = gg->current_display; 
  cpaneld *cpanel = &dsp->cpanel;*/
  gfloat fracpath;

  gfloat step = *st;
  gfloat delta = *dlt;
  gint nsteps = *ns;
  gint stepcntr = *stcn;

  if (slidepos < 5)
  {
    step = 0.0;
    delta = 0.0;
    nsteps = 0;
    stepcntr = 0;
  }
  else
  {
    /*
     * To cause tour to speed up wildly at the right of the
     * scrollbar range.
    */
    if (slidepos < 50)
      step = ((gfloat) slidepos - 5.) / 2000. ;
    else if ((slidepos >= 50) && (slidepos < 90))
      step = (gfloat) pow((double)(slidepos-50)/100.,(gdouble)1.5) + 0.0225;
    else
      step = (gfloat) sqrt((double)(slidepos-50)) + 0.1868;

    delta = step*M_PI_2/10.0;
    if (nsteps > 0)
      fracpath = stepcntr/nsteps;
    else 
      fracpath = 1.0;

    nsteps = (gint) floor((gdouble)(dv/delta))+1;
    stepcntr = (gint) floor(fracpath*nsteps);

  }

  /*    printf("slidepos: %d; nsteps: %d; stepcntr: %d; delta: %f; dv: %f\n",slidepos,nsteps,stepcntr, delta,dv);*/

  *st = step;
  *dlt = delta;
  *ns = nsteps;
  *stcn = stepcntr;
}

void
gt_basis (array_f u1, gint nvars, vector_i vars, gint nc, gint nd)
/*
 * Generate d random p dimensional vectors to form new ending basis
*/
{
  gint i, j, k, check = 1, nvals = nvars*nd, ntimes;
  gdouble frunif[2];
  gdouble r, fac, frnorm[2];
  gboolean oddno;

  if ((nvals % 2) == 1) 
    oddno = true;
  else  
    oddno=false;

  if (oddno)
    ntimes = nvals/2+1;
  else
    ntimes = nvals/2;
/*
 * Method suggested by Press, Flannery, Teukolsky, and Vetterling (1986)
 * "Numerical Recipes" p.202-3, for generating random normal variates .
*/

  /* Zero out u1 before filling; this might fix a bug we are
     encountering with returning from a receive tour.
  */
  for (j=0; j<nc; j++)
    for (k=0; k<nd; k++)
      u1.vals[k][j] = 0.0 ;

  if (nvars > nd) {
      for (j=0; j<ntimes; j++) {
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
            if (oddno && j == ntimes-1) {
              u1.vals[0][vars.els[2*j]] = (gfloat) frnorm[0];
	    }
            else {
              u1.vals[0][vars.els[2*j]] = (gfloat) frnorm[0];
              u1.vals[0][vars.els[2*j+1]] = (gfloat) frnorm[1];
	    }
	  }
          else if (nd == 2) {
              u1.vals[0][vars.els[j]] = (gfloat) frnorm[0];
              u1.vals[1][vars.els[j]] = (gfloat) frnorm[1];
	  }
      }
    for (k=0; k<nd; k++)
      norm(u1.vals[k], nc);

    /*
     * Orthogonalize the basis on the first using Gram-Schmidt
    */
    if (nd > 1) {
      for (k=0; k<nd-1; k++)
        for (j=k+1; j<nd; j++)
          gram_schmidt(u1.vals[k], u1.vals[j], nc);
    }
  }
  else /* if there is only one variable */
  {
    for (i=0; i<nd; i++)
      u1.vals[i][vars.els[i]] = 1.;
  }
}

