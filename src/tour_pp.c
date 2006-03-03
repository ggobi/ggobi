/* tour_pp.c */
/* Copyright (C) 2001 Dianne Cook and Sigbert Klinke and Eun-Kyung Lee

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

The authors can be contacted at the following email addresses:
    dicook@iastate.edu    sigbert@wiwi.hu-berlin.de
*/

#include <gtk/gtk.h>
#ifdef USE_STRINGS_H
#include <strings.h>
#endif

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if !defined __APPLE__ && !defined __GNUC__
#include <malloc.h>
#endif

#include "vars.h"
#include "externs.h"

#include "tour_pp.h"
#include "tour1d_pp.h"
#include "tour2d_pp.h"

gfloat randomval, nrand;
gint nset;

/* reset pp variables */
void reset_pp(GGobiData *d, gint nprev, gint b, ggobid *gg, void *data)
{
  displayd *dsp;
  GList *l;
  for (l=gg->displays; l; l=l->next) {
    dsp = (displayd *) l->data;
    if (dsp->t1d_window != NULL && GTK_WIDGET_VISIBLE (dsp->t1d_window)) {
      free_optimize0_p(&dsp->t1d_pp_op);
      alloc_optimize0_p(&dsp->t1d_pp_op, d->nrows_in_plot, dsp->t1d.nactive, 
        1);
      t1d_pp_reinit(dsp, gg);
    }
    if (dsp->t2d_window != NULL && GTK_WIDGET_VISIBLE (dsp->t2d_window)) {
      free_optimize0_p(&dsp->t2d_pp_op);
      alloc_optimize0_p(&dsp->t2d_pp_op, d->nrows_in_plot, dsp->t2d.nactive, 
        2);
      t2d_pp_reinit(dsp, gg);
    }
  }
}

/********************************************************************

                         OPTIMIZATION

The index function has to be defined as

     gint index (array_f *pdata, void *param, gfloat *val)

with   

Input:  pdata   projected data
        param   additional parameters for the index 
                (will not be touched by the optimization routine)

Output: val     the index-value
        
The return value should be zero, otherwise the optimization routine
assumes an error has occurred during computation of the index. 

*********************************************************************/

gint alloc_optimize0_p (optimize0_param *op, gint nrows, gint ncols, gint ndim)
{
  arrayf_init_null (&op->proj_best);
  /*  arrayf_alloc_zero (&op->proj_best, ncols, ndim); *nrows, ncols);*/
  arrayf_alloc_zero (&op->proj_best, ndim, ncols); /*nrows, ncols);*/
  arrayf_init_null (&op->data);
  arrayf_alloc_zero (&op->data, nrows, ncols); 
  arrayf_init_null (&op->pdata);
  arrayf_alloc_zero (&op->pdata, nrows, ndim); 

  return 0;
}

gint free_optimize0_p (optimize0_param *op)
{ 
  arrayf_free (&op->proj_best, 0, 0);
  arrayf_free (&op->data, 0, 0);
  arrayf_free(&op->pdata, 0, 0);

  return 0;
}

gint realloc_optimize0_p (optimize0_param *op, gint ncols, vector_i pcols)
{
  gint i, ncolsdel;
  gint *cols;/* = g_malloc (ncols*sizeof(gint));*/

/* pdata doesn't need to be reallocated, since it doesn't depend on ncols */
  if (op->proj_best.ncols < ncols) {
    arrayf_add_cols(&op->proj_best, ncols);
    arrayf_add_cols(&op->data, ncols);
  }
  else {
    ncolsdel = op->proj_best.ncols - ncols;
    cols = g_malloc (ncolsdel*sizeof(gint));
    for (i=0; i<ncolsdel; i++)
      cols[i] = ncols-1-i;

    arrayf_delete_cols(&op->proj_best, ncolsdel, cols);
    arrayf_delete_cols(&op->data, ncolsdel, cols);
    g_free (cols);
  }

  return 0;
}

gboolean iszero (array_f *data)
{ 
  gfloat sum = 0;
  gint i, j;

  for (i=0; i<data->nrows; i++)
  { for (j=0; j<data->ncols; j++)
      sum += fabs(data->vals[i][j]);
  }
  return (sum<1e-6);
}

void initrandom(gfloat start)
{ 
  randomval = floor (fmod (fabs(start), 62748517.0));
  nset   = 0;
}

gfloat uniformrandom()
{ 
  randomval = fmod (27132.0 * randomval + 7.0, 62748517.0);
  return (randomval / 62748517.0);
}

gfloat normalrandom()
{ 
  gfloat x, y, r;
  if (nset) { nset = 0; return(nrand); }
  do
  { x = 2.0*uniformrandom()-1.0;
    y = 2.0*uniformrandom()-1.0;
    r = x*x+y*y;
  }
  while (r>=1.0);
  r = sqrt(-2.0*log(r)/r);
  nrand = x*r;
  nset  = 1;
  return(y*r);
}

void normal_fill (array_f *data, gfloat delta, array_f *base)
{ 
  int i, j;
  for (i=0; i<data->nrows; i++)
  { for (j=0; j<data->ncols; j++)
      data->vals[i][j] = base->vals[i][j]+delta*normalrandom();
  }
}

void orthonormal (array_f *proj)
{ 
  gint i, j, k;
  gfloat *ip = g_malloc (proj->ncols*sizeof(gfloat));
  gfloat norm;

  /* First norm vector p_i */
  for (i=0; i<proj->nrows; i++)
  { 
    norm = 0.0;
    for (k=0; k<proj->ncols; k++)
      norm += (proj->vals[i][k]*proj->vals[i][k]);
    norm = sqrt(norm);
    for (k=0; k<proj->ncols; k++)
      proj->vals[i][k] /= norm;
  }

  for (i=0; i<proj->nrows; i++)
  { /* Compute inner product between p_i and all p_j */
    for (j=0; j<i; j++)
    { ip[j] = 0;
      for (k=0; k<proj->ncols; k++)
        ip[j] += proj->vals[j][k]*proj->vals[i][k];
    }
    /* Subtract now all vectors from p_i */
    for (j=0; j<i; j++)
    { for (k=0; k<proj->ncols; k++)
        proj->vals[i][k] -= ip[j]*proj->vals[j][k];
    }
    /* Finally norm vector p_i */
    norm = 0.0;
    for (k=0; k<proj->ncols; k++)
      norm += (proj->vals[i][k]*proj->vals[i][k]);
    norm = sqrt(norm);
    for (k=0; k<proj->ncols; k++)
      proj->vals[i][k] /= norm;
  }
  g_free(ip);
}

gint optimize0 (optimize0_param *op,
                Tour_PPIndex_f index,
                void *param)
{ 
  gfloat index_work = 0.0;
/*  array_f proj_work, pdata, *proj;*/
  array_f proj_work, *proj;
  int i,j, m, k;

  proj = &(op->proj_best);
  arrayf_init_null (&proj_work);
  arrayf_alloc_zero (&proj_work, proj->nrows, proj->ncols);
  /*  arrayf_init_null (&pdata);
      arrayf_alloc_zero (&pdata, op->data.nrows, proj->ncols);*/

  /*  op->temp_start     =  1; * make this an interactive parameter */
  op->temp_end       =  0.001;
  /*  op->cooling        =  0.99; * make this an interactive parameter */
  /* is equivalent to log(temp_end/temp_start)/log(cooling) projections */
  op->heating        =  1;
  op->restart        =  1;
  op->success        =  0;
  op->temp           =  op->temp_start;/*0.1; 1.0;*/
  op->maxproj        =  op->restart*(1+log(op->temp_end/op->temp_start)/
    log(op->cooling)); /* :) */

  /*g_printerr("NEW OPTIMIZATION\n");
    g_printerr ("index_work %f index_best %f \n",index_work, op->index_best);*/
  /* This adds random noise to existing projection and orthonormalize, 
     if the current projection is null */
  if (iszero(proj))
  { /* sprintf (msg, "zero projection matrix"); print(); */
    normal_fill (proj, 1.0, proj);
    orthonormal (proj);
  }

  /*g_printerr("nrows %d ncols %d\n",op->data.nrows, op->data.ncols);

g_printerr ("proj: ");
for (i=0; i<proj->ncols; i++) g_printerr ("%f ", proj->vals[0][i]);
g_printerr ("\n");
for (i=0; i<proj->ncols; i++) g_printerr ("%f ", proj->vals[1][i]);
g_printerr ("\n");*/
  /* calculate projected data */ /* this is generated outside this function */
  /*  for (i=0; i<op->data.nrows; i++)
  { 
    for (j=0; j<proj->nrows; j++)
    { 
      op->pdata.vals[i][j] = 0;
      for (m=0; m<op->data.ncols; m++)
        op->pdata.vals[i][j] += op->data.vals[i][m]*proj->vals[j][m];
    }
    }*/
  /* do index calculation, functions return -1 if a problem, which
     is then passed back through optimize0 to tour1d_run */
  if (index (&op->pdata, param, &op->index_best, NULL)) return(-1);
  /*g_printerr ("index_work %f index_best %f \n",index_work, op->index_best);*/
/*  if (index (&op->pdata, param, &index_work)) return(-1);*/

  /* fill proj_work */
  arrayf_copy (proj, &proj_work);

  op->success = k = 0;
  while (op->restart > 0)
  { /* sprintf (msg, "Restart %i", op->restart); print(); */
    while (op->temp > op->temp_end)
    { /* sprintf (msg, "Iteration %i", k); print(); */
      /* add some randomness to current projection */
      normal_fill (&proj_work, op->temp, proj);
      orthonormal (&proj_work);                              
      op->temp *= op->cooling;

      /* calc projected data */
      for (i=0; i<op->data.nrows; i++)
      { 
        for (j=0; j<proj->nrows; j++)
        { 
          op->pdata.vals[i][j] = 0;
          for (m=0; m<op->data.ncols; m++)
            op->pdata.vals[i][j] += op->data.vals[i][m]*proj_work.vals[j][m];
        }
      }

      /* Calculate pp index for current projection */       
      if (index (&op->pdata, param, &index_work, NULL)) return(-1);

      /*g_printerr ("index_work %f temp %f \n",index_work, op->temp);
g_printerr ("proj_work: ");
for (i=0; i<proj->ncols; i++) g_printerr ("%f ", proj_work.vals[0][i]);
g_printerr ("\n    ");
g_printerr ("index_best %f temp %f \n", op->index_best, op->temp);
g_printerr ("proj_best: ");
for (i=0; i<proj->ncols; i++) g_printerr ("%f ", op->proj_best.vals[0][i]);
g_printerr ("\n");
      */
      if (index_work > op->index_best)
      { /* sprintf (msg, "Success %f", index_work); print(); */
        op->success++;
        /*printf ("Success %f\n", index_work); */
        arrayf_copy (&proj_work, proj); /*Sigbert's code, I think 
               this should be saving it into the best proj rather than work*/
        arrayf_copy (&proj_work, &op->proj_best);
        op->index_best = index_work;
        op->temp *= op->heating;
      }
      k++; 
      if (k >= op->maxproj) 
      { 
    /*        printf("A #iter = %d \n",k);
        printf ("Best = %f\n", op->index_best);
        for (i=0; i<proj->nrows; i++)        
    { for (j=0; j<proj->ncols; j++)
          printf ("%+5.3f ", proj->vals[i][j]);
        printf ("\n");
        }
        printf ("\n");*/
        return(k);
      }
    }
    op->temp = op->temp_start;
    op->restart--;
  } 

  /*  printf("B #iter = %d \n",k);
  printf ("Best = %f\n", op->index_best);
  for (i=0; i<proj->nrows; i++)        
  { 
    for (j=0; j<proj->ncols; j++)
      printf ("%+5.3f ", proj->vals[i][j]);
     printf ("\n");
  }
  printf ("\n");
  */
  return (k);
}

/********************************************************************
      OPTIMIZATION BY DERIVATIVES
********************************************************************/

void
pp_deriv_optimization()
{
/*
  int i, j;
  float tmpf1, tmpf2;
  float eps = 0.5;
*/

  /* calc index first */

  /* calc derivs */
  /*  if (xg->pp_index_btn == HERMITE_BTN)
      hermite_deriv1(xg->tform2, X, xg->u[0], xg->u[1], dIhat,
        xg->nlinkable_in_plot, xg->rows_in_plot,
        xg->ncols_used, xg->tour_vars, xg->numvars_t, lJ);

  else if (xg->pp_index_btn == CENTRAL_MASS_BTN)
      central_mass_deriv(xg->tform2, X,
        xg->u[0], xg->u[1], dIhat,
        xg->nlinkable_in_plot, xg->rows_in_plot,
        xg->ncols_used, xg->numvars_t, xg->tour_vars);
  */
/* update xg->u1 by the direction vectors */
  /*  if (derivs_equal_zero(xg))
  {
    stop_tour_proc(xg);
    write_msg_in_pp_window();
  }
  else
  {
    for (i=0; i<xg->numvars_t; i++)
      a[i] = dIhat[0][xg->tour_vars[i]];
    tmpf1 = calc_norm(a, xg->numvars_t);
    tmpf1 *= tmpf1;
    for (i=0; i<xg->numvars_t; i++)
      a[i] = dIhat[1][xg->tour_vars[i]];
    tmpf2 = calc_norm(a, xg->numvars_t);
    tmpf2 *= tmpf2;
    tmpf1 = sqrt((double) (tmpf1+tmpf2));
    for (j=0; j<2; j++)
      for (i=0; i<xg->numvars_t; i++)
        dIhat[j][xg->tour_vars[i]] *= (eps/tmpf1);
    for (i=0; i<2; i++)
      for (j=0; j<xg->numvars_t; j++)
        xg->u1[i][xg->tour_vars[j]] = xg->u[i][xg->tour_vars[j]]
          + dIhat[i][xg->tour_vars[j]];

    norm(xg->u1[0], xg->ncols_used);
    norm(xg->u1[1], xg->ncols_used);
    for (i=0; i<2; i++)
      for (j=0; j<xg->numvars_t; j++)
        xg->tv[i][j] = xg->u1[i][xg->tour_vars[j]];
    gram_schmidt(xg->tv[0], xg->tv[1],xg->numvars_t);
    for (i=0; i<2; i++)
      for (j=0; j<xg->numvars_t; j++)
        xg->u1[i][xg->tour_vars[j]] = xg->tv[i][j];

    init_basis(xg);
    }*/
}

/*****************************************************/
/*               Utility Routines                    */
/*                                                   */
/* Reference : An Introduction to Numerical Analysis */
/*             - Kendall E. Atkinson                 */
/*             (p 449 - 450)                         */
/*****************************************************/

void inverse(gdouble *a, gint n)
{
  gdouble *b,*inv,d;
  gint *P,i,j;

  P = (gint *) g_malloc(n*sizeof(gint));
  inv = (gdouble *) g_malloc(n*n*sizeof(gdouble));
  d = ludcmp(a,n,P);
 
  b = (gdouble *) g_malloc(n*sizeof(gdouble));
  for(i=0; i<n; i++)
  {  
    for(j=0; j<n; j++)
    {  
      if(i == j) b[j] = 1.0; else b[j] = 0.0;
    }
    d=tour_pp_solve(a,b,n,P);
     for(j=0; j<n; j++)
       inv[j*n+i] = b[j];
  }
  memcpy(a,inv,n*n*sizeof(gdouble));

  g_free(P);
  g_free(inv);
  g_free(b);
}    

gdouble
tour_pp_solve(gdouble *a,gdouble *b,gint n,gint *Pivot) 
{
  gint i,j,k;
  gdouble temp;
  
  for(k=0; k<(n-1); k++)
  {  if(Pivot[k] != k)
     {  temp = b[Pivot[k]];
        b[Pivot[k]] = b[k];
        b[k] = temp;
     }
     for(i=(k+1);i<n; i++)
        b[i] -= a[i*n+k]*b[k];
  }
  b[n-1] /= a[n*n-1];
  for(i=(n-2); i>=0; i--)
  {  temp=0;
     for(j=(i+1); j<n; j++)
       temp += a[i*n+j]*b[j];
       b[i] = (b[i] -temp)/a[i*n+i];
  }
  return(0);
}


gdouble ludcmp(gdouble *a, gint n, gint *Pivot) 
{ 
  gint i,j,k,ier;
  gdouble *s,det,temp,c;
  det=1;

  s = (gdouble *) g_malloc(n*sizeof(gdouble));

  for (i=0;i<n; i++) {  
    s[i] = a[i*n+1];
    for(j=1; j<n; j++)
      if(s[i] < a[i*n+j]) s[i] = a[i*n+j];
  }
  for (k=0;k<n-1; k++) {  
    for(i=k; i<n; i++) {   
      temp = fabs(a[i*n+k]/s[i]);
      if(i==k) { 
        c=temp; Pivot[k]=i;
      }
      else if(c <temp) {
        c = temp; Pivot[k]=i;
      }
    }  
    /* If all elements of a row(or column) of A are zero, |A| = 0 */
    if(c==0) {   
      det=0;
      return(det);
    }
    if(Pivot[k]!=k) {   
      det*=-1; 
      for(j=k; j<n; j++) {   
        temp = a[k*n+j]; 
        a[k*n+j]=a[Pivot[k]*n+j]; 
        a[Pivot[k]*n+j]=temp;
      }
      temp = s[k];
      s[k] = s[Pivot[k]];   
      s[Pivot[k]]=temp;
    }
    for(i=k+1; i<n; i++) {   
      temp = a[i*n+k]/a[k*n+k];
      a[i*n+k] = temp;
      for(j=k+1; j<n; j++)
      a[i*n+j] -= temp*a[k*n+j];
    }
    det *= a[k*n+k];
  }
  k = n-1;
  det *= a[(n-1)*n+(n-1)];
  ier=0; 

  g_free(s);

  return(det);
}                               
  
/********************************************************************
             Arbitrary dimensional pp indices
*********************************************************************/

gint alloc_pp(pp_param *pp, gint nrows, gint ncols, gint ndim)
{
  gint nr = nrows;/* number of cases */
  gint nc = MAX(ncols,2); /* ncols = number of active vars */
  gint nd = MAX(ndim,2); /* projection dimension */
  gint ncolors = 50; /* This is a guess at the max num of colors -
                        it really shouldn't be hard-coded */

  vectori_alloc_zero(&pp->group, nr);
  vectori_alloc_zero(&pp->ngroup, nr);

  arrayd_alloc_zero(&pp->cov, nd, nd);
  arrayd_alloc_zero(&pp->tcov, nd, nd); /* temporary usage in lda */
  arrayd_alloc_zero(&pp->mean, ncolors, nd); /* means for each group */
  vectord_alloc_zero(&pp->ovmean, nc); /* mean for each projection */

  vectori_alloc_zero(&pp->index, nr);/* used in gini/entropy */
  vectori_alloc_zero(&pp->nright, nr);/* used in gini/entropy */
  vectord_alloc_zero(&pp->x, nr); /* used in gini/entropy */

  return 0;
}

gint free_pp (pp_param *pp)
{
  vectori_free(&pp->group);
  vectori_free(&pp->ngroup);

  arrayd_free(&pp->cov, 0, 0);
  arrayd_free(&pp->tcov, 0, 0);
  arrayd_free(&pp->mean, 0, 0);
  vectord_free(&pp->ovmean);

  vectori_free(&pp->index);
  vectori_free(&pp->nright);
  vectord_free(&pp->x);

  return 0;
}

/********************************************************************

Index          : Holes
Transformation : -
Purpose        : Looks for the projection with no data in center.
*********************************************************************/

gint holes_raw(array_f *pdata, void *param, gfloat *val, gpointer unused)
{ 
  pp_param *pp = (pp_param *) param;
  int i, p, n, k,j;
  gdouble tmp,x1,x2;
  gdouble *cov;
  gdouble acoefs;
  gdouble tol = 0.0001;

  p = pdata->ncols; 
  n = pdata->nrows;
  cov = (gdouble *) g_malloc(p*p*sizeof(gdouble));
  zero(cov,p*p);

  for(j=0; j<p; j++) {
    pp->ovmean.els[j] = 0.0;
    for(i=0; i<n; i++) 
      pp->ovmean.els[j] += pdata->vals[i][j];
    pp->ovmean.els[j] /= ((gdouble)n);
  }

  for (j=0; j<p; j++) { 
    for (k=0; k<=j; k++) { 
      pp->cov.vals[k][j] = 0.0;
      for (i=0; i<n; i++) 
        pp->cov.vals[k][j] += (((pdata->vals[i][j])-pp->ovmean.els[j])*
         ((pdata->vals[i][k])-(pp->ovmean.els[k])));
      pp->cov.vals[k][j] /= ((double)(n-1)); 
      if (j != k)
        pp->cov.vals[j][k] = pp->cov.vals[k][j];
      }
  }

  /*  g_printerr("cov %f %f %f %f\n",pp->cov.vals[0][0],
      pp->cov.vals[0][1],pp->cov.vals[1][0],pp->cov.vals[1][1]);*/
  if (p>1) {
    for (i=0; i<p; i++)
      for (j=0; j<p; j++)
        cov[i*p+j] = pp->cov.vals[i][j];
    inverse(cov, p);
    for (i=0; i<p; i++)
      for (j=0; j<p; j++)
        pp->cov.vals[i][j] = cov[i*p+j];
  }
  else {
    if (pp->cov.vals[0][0] > tol)
      pp->cov.vals[0][0] = 1./pp->cov.vals[0][0];
    else
      pp->cov.vals[0][0] = 10000.0;
  }

  acoefs = 0.0;
  for (i=0; i<n; i++) { 
    tmp = 0.0;  
    for (j=0; j<p; j++) {   
      x1 = pdata->vals[i][j]-pp->ovmean.els[j];
      for (k=0; k<p; k++) {  
        x2 = pdata->vals[i][k]-pp->ovmean.els[k]; 
        tmp += (x1*x2*pp->cov.vals[j][k]);
      }
    } 
    acoefs += exp(-tmp/2.0);
  }
  *val = (1.0-acoefs/(gdouble)n)/(gdouble) (1.0-exp(-p/2.0));

  g_free(cov);

  return(0);
}

/********************************************************************

Index          : Central Mass
Transformation : -
Purpose        : Looks for the projection with lots of data in center.
*********************************************************************/

gint central_mass_raw(array_f *pdata, void *param, gfloat *val, gpointer unused)
{ 
  pp_param *pp = (pp_param *) param;
  int i, p, n,k,j;
  gdouble tmp,x1,x2;
  gdouble *cov;
  gdouble acoefs;
  gdouble tol = 0.0001;

  p = pdata->ncols; 
  n = pdata->nrows;
  cov = (gdouble *) g_malloc(p*p*sizeof(gdouble));
  zero(cov,p*p);

  for(j=0; j<p; j++) {
    pp->ovmean.els[j] = 0.0;
    for(i=0; i<n; i++) 
      pp->ovmean.els[j] += pdata->vals[i][j];
    pp->ovmean.els[j] /= ((gdouble)n);
  }

  for (j=0; j<p; j++) { 
    for (k=0; k<=j; k++) { 
      pp->cov.vals[k][j] = 0.0;
      for (i=0; i<n; i++) 
        pp->cov.vals[k][j] += (((pdata->vals[i][j])-pp->ovmean.els[j])*
         ((pdata->vals[i][k])-(pp->ovmean.els[k])));
      pp->cov.vals[k][j] /= ((double)(n-1)); 
      if (j != k)
        pp->cov.vals[j][k] = pp->cov.vals[k][j];
      }
  }

  if (p>1) {
    for (i=0; i<p; i++)
      for (j=0; j<p; j++)
        cov[i*p+j] = pp->cov.vals[i][j];
    inverse(cov, p);
    for (i=0; i<p; i++)
      for (j=0; j<p; j++)
        pp->cov.vals[i][j] = cov[i*p+j];
  }
  else {
    if (pp->cov.vals[0][0] > tol)
      pp->cov.vals[0][0] = 1./pp->cov.vals[0][0];
    else
      pp->cov.vals[0][0] = 10000.0;
  }

  acoefs = 0.0;
  for (i=0; i<n; i++) { 
    tmp = 0.0;  
    for (j=0; j<p; j++) {   
      x1 = pdata->vals[i][j]-pp->ovmean.els[j];
      for (k=0; k<p; k++) {  
        x2 = pdata->vals[i][k]-pp->ovmean.els[k]; 
        tmp += (x1*x2*pp->cov.vals[j][k]);
      }
    } 
    acoefs += exp(-tmp/2.0);
  }
  *val = (acoefs/n-exp(-p/2.0))/((gdouble) (1-exp(-p/2.0)));

  g_free(cov);
  return(0);
}

/********************************************************************

Index          : Discriminant
Transformation : -
Purpose        : Looks for the best projection to discriminate
                 between groups.
*********************************************************************/

void zero (gdouble *ptr, gint length)
{ 
  gint i;

  for (i=0; i<length; i++)
    ptr[i] = 0.0;
}

void zero_int(gint *mem, int size)
{
  gint i;
  for(i=0; i<size; i++)
  mem[i] = 0;
}
  
gint compute_groups (vector_i group, vector_i ngroup, gint *numgroups, 
  gint nrows, gfloat *gdata)
{ 
  gint i, j, *groupval;

  /* initialize data */
  groupval = g_malloc (nrows*sizeof(gint));

  *numgroups = 0;
  for (i=0; i<nrows; i++)
  { for (j=0; j<*numgroups; j++)
    { if (groupval[j]==gdata[i])
      { ngroup.els[j]++;
        break;
      }
    }
    if (j==*numgroups )
    { groupval[j]  = gdata[i];
      ngroup.els[j] = 1;
      (*numgroups)++;
    }
  }

  for (i=0; i<nrows; i++)
  { for (j=0; j<*numgroups; j++)
    { if (groupval[j]==gdata[i])
        group.els[i] = j;
    }
  }

  g_free(groupval);

  return ((*numgroups==1) || (*numgroups==nrows));
}

gint discriminant (array_f *pdata, void *param, gfloat *val, gpointer unused)
{ 
  pp_param *pp = (pp_param *) param;
  gint i, j, k, l;
  gint n, p;
  gdouble det;
  gint *Pv; /* dummy structure for pivot in ludcmp - not used */
  gdouble *cov; /* need to get rid of this variable */

  n = pdata->nrows;
  p = pdata->ncols;

  Pv = (gint *) g_malloc(p*sizeof(gint));
  cov = (gdouble *) g_malloc(p*p*sizeof(gdouble));

  /* Compute means */
  for (k=0; k<p; k++) {
    for (l=0; l<pp->numgroups; l++)
      pp->mean.vals[l][k] = 0.0;
    pp->ovmean.els[k] = 0.0;
  }
  for (k=0; k<p; k++) {
    for (i=0; i<n; i++)
    { 
      pp->mean.vals[pp->group.els[i]][k] += (gdouble) pdata->vals[i][k]; 
      pp->ovmean.els[k] += (gdouble) pdata->vals[i][k];
    }
  }

  for (k=0; k<p; k++)
  { 
    for (i=0; i<pp->numgroups; i++)
    { 
      pp->mean.vals[i][k] /= (gdouble) pp->ngroup.els[i];
    }
    pp->ovmean.els[k] /= (gdouble) n;
  }

  /* Compute W */
  for (j=0; j<p; j++)
    for (k=0; k<p; k++)
      pp->cov.vals[j][k] = 0.0;
  for (i=0; i<n; i++)
  { 
    for (j=0; j<p; j++)
    { 
      for (k=0; k<=j; k++)
      { 
        pp->cov.vals[k][j] += 
          ((gdouble) pdata->vals[i][j]-pp->mean.vals[pp->group.els[i]][j])*
          ((gdouble) pdata->vals[i][k]-pp->mean.vals[pp->group.els[i]][k]);
        pp->cov.vals[j][k] = pp->cov.vals[k][j];
      }
    }
  }

  if (p>1) {
    for (i=0; i<p; i++)
      for (j=0; j<p; j++)
        cov[i*p+j] = pp->cov.vals[i][j];
    det = ludcmp(cov, p, Pv); 
    for (i=0; i<p; i++)
      for (j=0; j<p; j++)
        pp->cov.vals[i][j] = cov[i*p+j];
  }
  else
    det = fabs((gdouble) pp->cov.vals[0][0]);
    *val = det;

  /* Compute B */
  /*  for (j=0; j<p; j++)
    for (k=0; k<p; k++)
      pp->cov.vals[j][k] = 0.0;
  for (j=0; j<p; j++) 
  {	
    for(k=0; k<p; k++)
    {
      for (i=0; i< pp->numgroups; i++)	
        pp->cov.vals[j][k] += (pp->mean.vals[i][j]-pp->ovmean.els[j])*
          (pp->mean.vals[i][k]-pp->ovmean.els[k])*(gdouble)(pp->ngroup.els[i]);
    }
  }

  if (p>1) {
    for (i=0; i<p; i++)
      for (j=0; j<p; j++)
        cov[i*p+j] = pp->cov.vals[i][j];
    det = ludcmp(cov, p, Pv); 
    for (i=0; i<p; i++)
      for (j=0; j<p; j++)
        pp->cov.vals[i][j] = cov[i*p+j];
  }
  else
    det = fabs((gdouble) pp->cov.vals[0][0]);
    *val = det;*/

  /* Compute W+B */
  for (j=0; j<p; j++)
    for (k=0; k<p; k++)
      pp->cov.vals[j][k] = 0.0;
  for (i=0; i<n; i++)
  { 
    for (j=0; j<p; j++)
    { 
      for (k=0; k<=j; k++)
      { 
        pp->cov.vals[k][j] += 
          ((gdouble) pdata->vals[i][j]-pp->ovmean.els[j])*
          ((gdouble) pdata->vals[i][k]-pp->ovmean.els[k]);
        pp->cov.vals[j][k] = pp->cov.vals[k][j];
      }
    }
  }

  if (p>1) {
    for (i=0; i<p; i++)
      for (j=0; j<p; j++)
        cov[i*p+j] = pp->cov.vals[i][j];
    det = ludcmp(cov, p, Pv); 
    for (i=0; i<p; i++)
      for (j=0; j<p; j++)
        pp->cov.vals[i][j] = cov[i*p+j];
  }
  else
    det = fabs((gdouble) pp->cov.vals[0][0]);

  *val = 1.0-*val/det; /*1-W/(W+B)*/
/*  *val = *val/det; B/(W+B) */

  g_free(Pv);
  g_free(cov);

  return (0);
}

/********************************************************************

Index          : Gini, Entropy, Variance
Transformation : -
Purpose        : Looks for the best split in 1d-projected data.

*********************************************************************/

void swap_group(array_f *pdata, gint *group, int i, int j)
{
  int temp1,k; 
  double temp2;

  temp1 = group[i];
  group[i] = group[j];
  group[j] = temp1;
  for(k=0; k<pdata->ncols; k++)
  { temp2 = pdata->vals[i][k];   
        pdata->vals[i][k] = pdata->vals[j][k];
        pdata->vals[j][k] = temp2;
  }

}

void sort_group(array_f *pdata, gint *group, int left, int right)
{
  int i, last;
                                
  if(left >= right) return;
  swap_group(pdata, group, left, (left+right)/2);
  last = left;   
  for(i=left+1; i<=right; i++)
    if(group[i] < group[left])
      swap_group(pdata, group, ++last,i);
  swap_group(pdata, group, left, last);
  sort_group(pdata, group, left, last-1);
  sort_group(pdata, group, last+1,right);
}       

void swap_data(double *x, int *index,int i, int j)
{
  int temp1; double temp2;
  
  temp1 = index[i];
  index[i] = index[j];
  index[j] = temp1;
  temp2 = x[i];
  x[i]= x[j];
  x[j] = temp2;
}

void sort_data(double *x, int *index,int left, int right)
{
  int i, last;
                                
  if(left >= right) return;
  swap_data(x,index,left,(left+right)/2);
  last = left;   
  for(i=left+1; i<=right; i++)
    if(x[i] < x[left])
      swap_data(x,index,++last,i);
  swap_data(x,index, left, last);
  sort_data(x,index, left, last-1);
  sort_data(x,index,last+1,right);
}       

void countgroup(int *group, int *gps, int n)
{
  int temp,i;
  int groups = *gps;

  temp = group[0]; 
  groups=1; 

  for(i=1; i<n; i++) 
    if (group[i] != temp) 
      (groups)++; 
  temp = group[i];

  *gps = groups;
}

void countngroup(int *group, int *ngroup, int n)
{
  int temp,i,j;

  temp= group[0]; 
  ngroup[0] = 1; 
  j=0; 
  for(i=1; i<n; i++) 
  {	
    if (group[i] != temp) 
      temp = group[i]; j++;
    (ngroup[j]) ++; 
  } 

}

gint cartgini (array_f *pdata, void *param, gfloat *val, gpointer unused)
{ 
  pp_param *pp = (pp_param *) param;
  gint i, k, n, p, g = pp->numgroups, left, right, l;
  gfloat dev, prob, maxindex, index;

  n = pdata->nrows;
  p = pdata->ncols;

/* Sort pdata by group */ 
  right = pdata->nrows-1;
  left = 0;
  zero_int(pp->index.els,n);
  for (i=0; i<n; i++)
    pp->index.els[i] = pp->group.els[i];
  sort_group(pdata,pp->index.els,left,right);

/* data relocation and make index */ 
  zero(pp->x.els,n);

/* Calculate Gini index in each coordinate 
             and find minimum              */

  for (l=0; l<p; l++)
  {
    for (i=0; i<n; i++) { 
      pp->x.els[i] = pdata->vals[i][l];
      pp->index.els[i] = pp->group.els[i];
    }

    left=0;
    right=n-1;
    sort_data(pp->x.els, pp->index.els, left, right) ;

 /* Calculate gini index */
    zero_int(pp->nright.els,g);
    index = 1;
    for (i=0; i<g; i++) { 
      pp->nright.els[i] = 0;
      index -= (((gdouble)pp->ngroup.els[i])/((gdouble)n))*
        (((gdouble)pp->ngroup.els[i])/((gdouble)n));
    }
    for (i=0; i<n-1; i++)  {
      (pp->nright.els[pp->index.els[i]])++;
      dev=1;
      for (k=0; k<g; k++) {
        prob = ((gdouble) pp->nright.els[k])/((gdouble)(i+1));
        dev -= prob*prob*((gdouble)(i+1)/(gdouble)n);
        prob = ((gdouble) (pp->ngroup.els[k]-pp->nright.els[k]))/
          ((gdouble)(n-i-1));
        dev -= prob*prob*((gdouble)(n-i-1)/(gdouble)n);
      }
      if (dev<index) index = dev;
    }
    if(l==0) maxindex = index; /* index is between 0 and 1 - need max */
    else if(maxindex < index) maxindex = index;
  } 
  *val = 1-maxindex ;
  return(0);
}

gint cartentropy (array_f *pdata, void *param, gfloat *val, gpointer unused)
{ 
  pp_param *pp = (pp_param *) param;
  gint i, k, n, p, g = pp->numgroups, left, right,l;
  gfloat dev, prob, maxindex, index;

  n = pdata->nrows;
  p = pdata->ncols;

/* Sort pdata by group */ 
  right = pdata->nrows-1;
  left = 0;
  zero_int(pp->index.els,n);
  for (i=0; i<n; i++)
    pp->index.els[i] = pp->group.els[i];
  sort_group(pdata,pp->index.els,left,right);

/* data relocation and make index */ 
  zero(pp->x.els,n);

/* Calculate index in each coordinate and find minimum  */
  for(l=0; l<p; l++)
  {
    for (i=0; i<n; i++) { 
      pp->x.els[i] = pdata->vals[i][l];
      pp->index.els[i] = pp->group.els[i];
    }

    left=0;
    right=n-1;
    sort_data(pp->x.els, pp->index.els,left,right) ;

 /* Calculate index */
    zero_int(pp->nright.els,g);
    index = 0;
    for (i=0; i<g; i++) { 
      pp->nright.els[i] = 0;
      index -= (((gdouble)pp->ngroup.els[i])/((gdouble)n))*
        log(((gdouble)pp->ngroup.els[i])/((gdouble)n));
    }
    for (i=0; i<n-1; i++)  {
      (pp->nright.els[pp->index.els[i]])++;
      dev=0;
      for (k=0; k<g; k++) {
        prob = ((double) pp->nright.els[k])/((double)(i+1));
        if (prob > 0)
          dev -= prob*log(prob)*((gdouble)(i+1)/(gdouble)n);
        prob = ((double) (pp->ngroup.els[k]-pp->nright.els[k]))/
          ((double)(n-i-1));
        if (prob > 0)
          dev -= prob*log(prob)*((gdouble)(n-i-1)/(gdouble)n);
      }
      if (dev<index) index = dev;
    }
    if(l==0) maxindex=index;
    else if(maxindex < index) maxindex = index;
  } 
  *val = 1-maxindex/log(g) ;
  return(0);

}

