/* tour.c */

#include "stdlib.h"

#include <gtk/gtk.h>
#ifdef USE_STRINGS_H
#include <strings.h>
#endif
#include <math.h>
#include <stdlib.h>
/*#include <time.h>
  #include <sys/time.h>*/
#include <unistd.h>

#include "vars.h"
#include "externs.h"

void
zero_tau (vector_f tau, gint projdim) {

  gint k;

  for (k=0; k<projdim; k++) 
    tau.els[k] = 0.0;
}

void
zero_tinc(vector_f tinc, gint projdim) {
  gint k;

  for (k=0; k<projdim; k++) 
    tinc.els[k] = 0.0;
}

void
zero_lambda(vector_f lambda, gint projdim) {
  gint k;

  for (k=0; k<projdim; k++) 
    lambda.els[k] = 0.0;
}

void
norm(gdouble *x, gint n)
{
  gint j;
  gdouble xn = 0;

  for (j=0; j<n; j++)
    xn = xn + (x[j]*x[j]);
  xn = sqrt(xn);
  for (j=0; j<n; j++)
    x[j] = x[j]/xn;
}

gdouble
calc_norm(gdouble *x, gint n)
{
  gint j;
  gdouble xn = 0;

  for (j=0; j<n; j++)
    xn = xn + (x[j]*x[j]);
  xn = sqrt(xn);

  return(xn);
}

gdouble
inner_prod(gdouble *x1, gdouble *x2, gint n)
{
  gdouble xip;
  gint j;

  xip = 0.;
  for (j=0; j<n; j++)
    xip = xip + x1[j]*x2[j];
  return(xip);
}

/* orthonormalizes vector 2 on vector */
void
gram_schmidt(gdouble *x1, gdouble *x2, gint n)
{
  gint j;
  gdouble ip;
  gdouble tol=0.99;

  ip = inner_prod(x1, x2, n);

  if (fabs(ip) < tol) { /*  If the two vectors are not orthogonal already */
    for (j=0; j<n; j++)
      x2[j] = x2[j] - ip*x1[j];
    norm(x2, n);
  }

}

/* checks columns of matrix are orthonormal */
gboolean checkcolson(gdouble **ut, gint datadim, gint projdim) {
  gint j, k;
  gdouble tol = 0.01;
  gboolean ok = true;/* true means cols are o.n. */

  for (j=0; j<projdim; j++) {
    if (fabs(1.-calc_norm(ut[j], datadim)) > tol) {
      ok = false;
      return(ok);
    }
  }

  for (j=0; j<projdim-1; j++) {
    for (k=j+1; k<projdim; k++) {
      if (fabs(inner_prod(ut[j],ut[k],datadim)) > tol) {
        ok = false;
        return(ok);
      }
    }
  }

  return(ok);
}

gboolean checkequiv(gdouble **Fa, gdouble **Fz, gint datadim, gint projdim) {
  gint j;
  gdouble ftmp, tol = 0.0001;
  gboolean ok = true; /* false = the two are the same */

  for (j=0; j<projdim; j++) {
    ftmp = inner_prod(Fa[j], Fz[j], datadim);
    /*    printf("checkequiv %f \n",ftmp);*/
    /* if ftmp is close to zero it says the4 two vectors are close
       to being identical */
    if (fabs(1.-ftmp) < tol) {
      ok = false;
      return(ok);
    }
  }

  return(ok);
}

/* matrix multiplication UV */
gboolean matmult_uv(gdouble **ut, gdouble **vt, gint ur, gint uc, 
  gint vr, gint vc, gdouble **ot) {
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
        ot[k][j] += (ut[i][j]*vt[k][i]);
      }
    }
  }

  return(ok);
}

/* matrix multiplication U'V */
gboolean matmult_utv(gdouble **ut, gdouble **vt, gint ur, gint uc, 
  gint vr, gint vc, gdouble **ot) {
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
        ot[k][j] += (ut[j][i]*vt[k][i]);
      }
    }
  }

  return(ok);
}

/* matrix multiplication UV */
gboolean matmult_uvt(gdouble **ut, gdouble **vt, gint ur, gint uc, 
  gint vr, gint vc, gdouble **ot) {
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
        ot[k][j] += (ut[i][j]*vt[i][k]);
      }
    }
  }

  return(ok);
}

/* copy matrix ot=out matrix, it=in matrix */
void copy_mat(gdouble **ot, gdouble **it, gint nr, gint datadim) {
  gint j, k;

  for (j=0; j<nr; j++)
    for (k=0; k<datadim; k++)
      ot[k][j] = it[k][j];
}

/* orthonormalize x2 on x1, just by diagonals should be enough for this
   tour code */
void
matgram_schmidt(gdouble **x1, gdouble **x2, gint nr, gint datadim)
{
  gint j, k;
  gdouble ip;

  for (j=0; j<datadim; j++) {
    norm(x1[j], nr);
    norm(x2[j], nr);
    ip = inner_prod(x1[j], x2[j], nr);
    for (k=0; k<nr; k++)
      x2[j][k] = x2[j][k] - ip*x1[j][k];
    norm(x2[j], nr);
  }

}

void
eigen_clear (array_d Ga, array_d Gz, vector_f lambda, vector_f tau, 
  vector_f tinc, gint datadim)
{
  /*  datad *d = dsp->d;
  gint datadim = d->ncols;*/
  gint j, k;

  for (j=0; j<datadim; j++) {
    for (k=0; k<datadim; k++) {
      Ga.vals[j][k] = 0.;
      Gz.vals[j][k] = 0.;
    }
    lambda.els[j] = 0.;
    tau.els[j] = 0.;
    tinc.els[j] = 0.;
  }

}

/* Fa = starting projection
 * Fz = target projection
 * F = interpolated projection
 * datadim = num vars
 * projdim = proj dim
 */
gint path(array_d Fa, array_d Fz, array_d F, gint datadim, gint projdim, array_d Ga,
  array_d Gz, array_d G, vector_f lambda, array_d tv, array_d Va, array_d Vz,
  vector_f tau, vector_f tinc, gint *ns, gint *stcn, gfloat *pdist_az, 
  gfloat *ptang, gfloat step) 
{
  gint i, j, k;
  gdouble tol = 0.01;
  gdouble tmpd1 = 0.0, tmpd2 = 0.0, tmpd =0.0;
  gboolean doit = true;
  paird *pairs = (paird *) g_malloc (projdim * sizeof (paird));
  gfloat *e = (gfloat *) g_malloc (projdim * sizeof (gfloat));
  gint dI; /* dimension of intersection of base pair */
  gfloat **ptinc = (gfloat **) g_malloc (2 * sizeof (gfloat *));

  gfloat dist_az = *pdist_az;
  gfloat tang = *ptang;
  gint nsteps = *ns;
  gint stepcntr = *stcn;

  zero_tau(tau, projdim);
  zero_tinc(tinc, projdim);
  zero_lambda(lambda, projdim);
  for (i=0; i<projdim; i++)
    for (j=0; j<datadim; j++)
    {
      Ga.vals[i][j] = 0.0;
      Gz.vals[i][j] = 0.0;
      G.vals[i][j] = 0.0;
      Va.vals[i][j] = 0.0;
    }
  dist_az = 0.0;
  tang = 0.0;
  
  /* 2 is hard-wired because it relates to cos, sin
                         and nothing else. */
  for (i=0; i<2; i++) 
    ptinc[i] = (gfloat *) g_malloc (projdim * sizeof (gfloat));
    
  /* Check that Fa and Fz are both orthonormal. */
  if (!checkcolson(Fa.vals, datadim, projdim)) {
g_printerr("Columns of Fa are not orthonormal: get new Fa\n");
g_printerr ("Fa: ");
for (i=0; i<datadim; i++) g_printerr ("%f ", Fa.vals[0][i]);
g_printerr ("\n    ");
for (i=0; i<datadim; i++) g_printerr ("%f ", Fa.vals[1][i]);
g_printerr ("\n");
    return(1);
  }
  if (!checkcolson(Fz.vals, datadim, projdim)) {
g_printerr("Columns of Fz are not orthonormal: generating new Fz\n");
    return(2);
  }

  /* Check that Fa and Fz are the same */
  if (!checkequiv(Fa.vals, Fz.vals, datadim, projdim)) {
g_printerr("Fa equiv Fz: generating random Fz\n");
    return(3);
  }

  /* Do SVD of Fa'Fz: span(Fa,Fz).*/
  if (doit) {
    
    if (!matmult_utv(Fa.vals, Fz.vals, datadim, projdim, datadim, 
      projdim, tv.vals))
      printf("#cols != #rows in the two matrices");
      
    /* tv comes in as a projdimxprojdim asymmetric matrix */
      dsvd(tv.vals, projdim, projdim, lambda.els, Va.vals);
      /* tv gets overwritten with the left-hand reduction, u,
           which is then stored in Va,
         Vz gets the right-hand reduction, v */

      /* dec: switched order of Va and Vz,
         assuming Vz to be transpose, so forcing
         a transpose of this matrix to be used. Also
         it turns out that Va comes back as a transpose.
         Very strange, but this is what it takes to 
         get results consistent with R coding, and
         that is clearly correct. */

      for (i=0; i<projdim; i++)
        for (j=0; j<projdim; j++)
          Vz.vals[i][j] = tv.vals[j][i];

      for (i=0; i<projdim; i++)
        for (j=0; j<projdim; j++)
          tv.vals[i][j] = Va.vals[j][i];

      for (i=0; i<projdim; i++)
        for (j=0; j<projdim; j++)
	Va.vals[i][j] = tv.vals[i][j];

      /*  Check span of <Fa,Fz>
       If dimension of the intersection is equal to dimension of proj,
       dI=ndim, we should stop here, and set Ft to be Fa but this is
       equivalent to setting the lambda's to be 1.0 at this stage.*/
      dI = 0;
      for (i=0; i<projdim; i++) {
        if (lambda.els[i] > 1.0-tol) {
          dI++;
          lambda.els[i] = 1.0;
        }
      }

      /*  Compute principal angles */
      for (i=0; i<projdim; i++) {
        tau.els[i] = (gfloat) acos((gdouble) lambda.els[i]);
      }

      /*  Calculate principal directions */
      if (projdim > dI) { /* Span is ok - proceed */

        /* Rotate Fa to get Ga */
        for (i=0; i<datadim; i++)
          for (j=0; j<projdim; j++)
            tv.vals[j][i] = 0.0;
        arrayd_copy(&Va, &tv);

        if (!matmult_uv(Fa.vals, tv.vals, datadim, projdim, projdim, 
          projdim, Ga.vals))
          printf("Could not multiply u and v, cols!=rows \n");
        for (j=0; j<projdim; j++)
          norm(Ga.vals[j], datadim);
        for (i=0; i<projdim-1; i++) {
          for (j=i+1; j<projdim; j++)
            gram_schmidt(Ga.vals[i], Ga.vals[j], datadim);
	}
        
        /* Rotate Fz to get Gz */
        for (i=0; i<datadim; i++)
          for (j=0; j<projdim; j++)
            tv.vals[j][i] = 0.0;
        arrayd_copy(&Vz, &tv);

        if (!matmult_uv(Fz.vals, tv.vals, datadim, projdim, projdim, 
          projdim, Gz.vals))
            printf("Could not multiply u and v, cols!=rows \n");

        for (j=0; j<projdim; j++)
          norm(Gz.vals[j], datadim);
        for (i=0; i<projdim-1; i++) {
          for (j=i+1; j<projdim; j++)
            gram_schmidt(Gz.vals[i], Gz.vals[j], datadim);
	}

        /* orthonormalize Gz on Ga to make a frame of rotation */
        for (i=0; i<projdim; i++)
          gram_schmidt(Ga.vals[i], Gz.vals[i], datadim);
        for (j=0; j<projdim; j++)
          norm(Gz.vals[j], datadim);
        for (i=0; i<projdim-1; i++) {
          for (j=i+1; j<projdim; j++)
            gram_schmidt(Gz.vals[i], Gz.vals[j], datadim);
	}

      }
      /*      else { * Span not ok, cannot do interp path, so reinitialize *
        arrayd_copy(&Fa, &Ga);
        arrayd_copy(&Fa, &Gz);
        for (i=0; i<projdim; i++)
          tau.els[i] = 0.0;
        * Need to clean this up - It seems this never occurs *
	} Don't think this is needed */

      /* Construct current basis*/
      for (i=0; i<projdim; i++)
        tinc.els[i]=0.0;
      for (i=0; i<projdim; i++) {
        ptinc[0][i] = (gfloat) cos((gdouble) tinc.els[i]);
        ptinc[1][i] = (gfloat) sin((gdouble) tinc.els[i]);
      }

      for (i=0; i<projdim; i++) {
        tmpd1 = ptinc[0][i];
        tmpd2 = ptinc[1][i];
        for (j=0; j<datadim; j++) {
          tmpd = Ga.vals[i][j]*tmpd1 + Gz.vals[i][j]*tmpd2;
          G.vals[i][j] = tmpd;
        }
      }

      /* rotate in space of plane to match Fa basis */
      matmult_uvt(G.vals, Va.vals, datadim, projdim, projdim, projdim, F.vals);

      /* orthonormal to correct round-off errors */
      for (i=0; i<projdim; i++)
        norm(F.vals[i], datadim); 

      for (k=0; k<projdim-1; k++)
        for (j=k+1; j<projdim; j++)
          gram_schmidt(F.vals[k], F.vals[j], datadim);

      /* Calculate Euclidean norm of principal angles.*/
      tmpd = 0.0;
      for (i=0; i<projdim; i++)
        tmpd += ((gdouble)tau.els[i]*(gdouble)tau.els[i]);
      dist_az = (gfloat)sqrt(tmpd);

      if (dist_az < 0.0001) {
	/*        printf("returning before standardizing tau's\n");
        for (i=0; i<projdim; i++) 
          printf("tau %d %f ",i,tau.els[i]);
	  printf("\n");*/
	/*        zero_tau(tau, projdim);
        zero_tinc(tinc, projdim);
        zero_lambda(lambda, projdim);
        for (i=0; i<projdim; i++)
          for (j=0; j<datadim; j++)
          {
            Ga.vals[i][j] = 0.0;
            Gz.vals[i][j] = 0.0;
            G.vals[i][j] = 0.0;
            Va.vals[i][j] = 0.0;
            Fz.vals[i][j] = 0.0;
	    }
        dist_az = 0.0;
        tang = 0.0;
        *pdist_az = dist_az;
        *ptang = tang;*/
        arrayd_copy(&Fa, &F);
        return(3);
      }
      
      for (i=0; i<projdim; i++) {
        if (tau.els[i] > tol) {
          tau.els[i] /= dist_az;
	}
        else 
          tau.els[i] = 0.0;
      }

      *pdist_az = dist_az;
      *ptang = tang;
  }
  else {
    arrayd_copy(&Fa, &F);
    arrayd_copy(&Fa, &Ga);
    arrayd_copy(&Fz, &Gz);
    arrayd_copy(&Fz, &Va);
    arrayd_copy(&Fa, &G);

    *pdist_az = dist_az;
    *ptang = tang;
    *ns = nsteps;
    *stcn = stepcntr;

  }

  /*  printf("dist_az %f \n",dist_az);*/
/* free temporary arrays */
  g_free ((gpointer) pairs);
  for (j=0; j<2; j++)
    g_free (ptinc[j]);
  g_free (ptinc);
  g_free (e);

  return(0);
} /* path */

/* Generate the interpolation frame. No preprojection is done */
void tour_reproject(vector_f tinc, array_d G, array_d Ga, array_d Gz, 
  array_d F, array_d Va, gint datadim, gint projdim)
{
  gint i, j, k;
  gdouble tmpd1, tmpd2, tmpd;
  gfloat **ptinc = (gfloat **) g_malloc (2 * sizeof (gfloat *));

  for (i=0; i<2; i++)
    ptinc[i] = (gfloat *) g_malloc (projdim * sizeof (gfloat));

  for (i=0; i<projdim; i++) {
    ptinc[0][i] = (gfloat) cos((gdouble) tinc.els[i]);
    ptinc[1][i] = (gfloat) sin((gdouble) tinc.els[i]);
  }

  for (i=0; i<projdim; i++) {
    tmpd1 = ptinc[0][i];
    tmpd2 = ptinc[1][i];
    for (j=0; j<datadim; j++) {
      tmpd = Ga.vals[i][j]*tmpd1 + Gz.vals[i][j]*tmpd2;
      G.vals[i][j] = tmpd;
    }
  }

  /* rotate in space of plane to match Fa basis */
  matmult_uvt(G.vals, Va.vals, datadim, projdim, projdim, projdim, F.vals);

  /* orthonormalize to correct round-off errors */
  for (i=0; i<projdim; i++)
    norm(F.vals[i], datadim); 

  for (k=0; k<projdim; k++)
    for (j=k+1; j<projdim; j++)
      gram_schmidt(F.vals[k], F.vals[j], datadim);

  for (j=0; j<2; j++)
    g_free (ptinc[j]);
  g_free (ptinc);

}

/* this routine increments the interpolation */
void
increment_tour(vector_f tinc, vector_f tau, gint *ns, gint *stcn, 
  gfloat dist_az, gfloat delta, gfloat *ptang, gint projdim)
{
  int i;
  gboolean attheend = false;
  gfloat tang = *ptang;
  gint nsteps = *ns;
  gint stepcntr = *stcn;
  /*  time_t bt;
  struct tm *nowtm;
  struct timeval tv; 

  bt = time(NULL);
  nowtm = localtime(&bt);
  gettimeofday(&tv,NULL);*/

  tang += delta;

  if (tang >= dist_az)
      attheend = true;

  if (!attheend) {
    for (i=0; i<projdim; i++)
      tinc.els[i] = (tang*tau.els[i]);
  }

  *ptang = tang;
  *ns = nsteps;
  *stcn = stepcntr;
}

gboolean
reached_target(gint nsteps, gint stepcntr, gfloat tang, gfloat dist_az, 
  gint basmeth, 
  gfloat *indxval, gfloat *oindxval) 
{
  gboolean arewethereyet = false;

  if (basmeth == 0) {
    if (tang >= dist_az)
      arewethereyet = true;
  }
  else if (basmeth == 1) {
    if (*indxval < *oindxval)
    {
      arewethereyet = true;
      *indxval = *oindxval;
    }
    else
      *oindxval = *indxval;
  }

  return(arewethereyet);
}

gboolean
reached_target2(vector_f tinc, vector_f tau, gint basmeth, 
  gfloat *indxval, gfloat *oindxval, gint projdim) 
{
  gboolean arewethereyet = false;
  gfloat tol=0.01;
  gint i;

  if (basmeth == 1) {
    if (*indxval < *oindxval)
    {
      arewethereyet = true;
      *indxval = *oindxval;
    }
    else
      *oindxval = *indxval;
  }
  else {
    for (i=0; i<projdim; i++) 
    if (fabs(tinc.els[i]-tau.els[i]) < tol) 
      arewethereyet = true;
  }

  return(arewethereyet);
}

void
do_last_increment(vector_f tinc, vector_f tau, gfloat dist_az, gint projdim)
{
  int j;

  for (j=0; j<projdim; j++)
    tinc.els[j] = tau.els[j]*dist_az;

}

void speed_set (gint slidepos, gfloat *st, gfloat *dlt,  
  gint *ns, gint *stcn) {

  gfloat step = *st;
  gfloat delta = *dlt;
  gint nsteps = *ns;
  gint stepcntr = *stcn;

  if (slidepos < 5)
  {
    step = 0.0;
    delta = 0.0;
  }
  else
  {
    /*
     * To cause tour to speed up wildly at the right of the
     * scrollbar range.
    */
    if (slidepos < 50)
      step = ((gfloat) slidepos - 5.) / 2000.;
    else if ((slidepos >= 50))/* && (slidepos < 90))*/
      step = (gfloat) pow((double)(slidepos-50)/100.,(gdouble)1.5) + 0.0225;

    delta = (step*M_PI_2)/(10.0);

  }

  *st = step;
  *dlt = delta;
  *ns = nsteps;
  *stcn = stepcntr;
}

void
gt_basis (array_d Fz, gint nactive, vector_i active_vars, 
  gint datadim, gint projdim)
/*
 * Generate d random p dimensional vectors to form new ending basis
*/
{
  gint i, j, k, check = 1, nvals = nactive*projdim, ntimes;
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

  /* Zero out Fz before filling; this might fix a bug we are
     encountering with returning from a receive tour.
  */
  for (j=0; j<datadim; j++)
    for (k=0; k<projdim; k++)
      Fz.vals[k][j] = 0.0 ;

  if (nactive > projdim) {
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
      if (projdim == 1) {
        if (oddno && j == ntimes-1) {
          Fz.vals[0][active_vars.els[2*j]] = frnorm[0];
        }
        else {
          Fz.vals[0][active_vars.els[2*j]] = frnorm[0];
          Fz.vals[0][active_vars.els[2*j+1]] = frnorm[1];
        }
      }
      else if (projdim == 2) {
        Fz.vals[0][active_vars.els[j]] = frnorm[0];
        Fz.vals[1][active_vars.els[j]] = frnorm[1];
      }
    }
    for (k=0; k<projdim; k++)
      norm(Fz.vals[k], datadim);

    /*
     * Orthogonalize the basis on the first using Gram-Schmidt
    */
    if (projdim > 1) {
      for (k=0; k<projdim-1; k++)
        for (j=k+1; j<projdim; j++)
          gram_schmidt(Fz.vals[k], Fz.vals[j], datadim);
    }
  }
  else /* if there is only one variable */
  {
    for (i=0; i<projdim; i++)
      Fz.vals[i][active_vars.els[i]] = 1.;
  }

}

