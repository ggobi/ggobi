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
#include <malloc.h>

#include "vars.h"
#include "externs.h"

#include "tour_pp.h"
#include "tour1d_pp.h"

gfloat randomval, nrand;
gint nset;

/* reset pp variables */
void reset_pp(datad *d, gint nprev, gint b, ggobid *gg, void *data)
{
  displayd *dsp = gg->current_display; 

  if (dsp->t1d_window != NULL && GTK_WIDGET_VISIBLE (dsp->t1d_window)) {
    free_optimize0_p(&dsp->t1d_pp_op);
    alloc_optimize0_p(&dsp->t1d_pp_op, d->nrows_in_plot, dsp->t1d.nactive, 
      1);
    t1d_pp_reinit(gg);
  }
  if (dsp->t2d_window != NULL && GTK_WIDGET_VISIBLE (dsp->t2d_window)) {
    free_optimize0_p(&dsp->t2d_pp_op);
    alloc_optimize0_p(&dsp->t2d_pp_op, d->nrows_in_plot, dsp->t2d.nactive, 
      2);
    t2d_pp_reinit(gg);
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
assume an error has occured during computation of the index. 

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
  gfloat *ip = g_malloc (proj->ncols*sizeof(gfloat)), norm;

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
                gint (*index) (array_f*, void*, gfloat*),
                void *param)
{ gfloat index_work;
/*  array_f proj_work, pdata, *proj;*/
  array_f proj_work, *proj;
  int i,j, m, k;

  proj = &(op->proj_best);
  arrayf_init_null (&proj_work);
  arrayf_alloc_zero (&proj_work, proj->nrows, proj->ncols);
  /*  arrayf_init_null (&pdata);
      arrayf_alloc_zero (&pdata, op->data.nrows, proj->ncols);*/

  g_printerr("temp start 2 %f\n",op->temp_start);
  g_printerr("cooling 2%f\n",op->cooling);

  /*  op->temp_start     =  1; * make this an interactive parameter */
  op->temp_end       =  0.001;
  /*  op->cooling        =  0.99; * make this an interactive parameter */
  /* is equivalent to log(temp_end/temp_start)/log(cooling) projections */
  op->heating        =  1;
  op->restart        =  1;
  op->success        =  0;
  op->temp           =  0.1; /*1.0;*/
  op->maxproj        =  op->restart*(1+log(op->temp_end/op->temp_start)/
    log(op->cooling)); /* :) */

  /* This adds random noise to existing projection and orthonormalize, 
     if the current projection is null */
  if (iszero(proj))
  { /* sprintf (msg, "zero projection matrix"); print(); */
    normal_fill (proj, 1.0, proj);
    orthonormal (proj);
  }

  /* calculate projected data */
  for (i=0; i<op->data.nrows; i++)
  { 
    for (j=0; j<proj->nrows; j++)
    { 
      op->pdata.vals[i][j] = 0;
      for (m=0; m<op->data.ncols; m++)
        op->pdata.vals[i][j] += op->data.vals[i][m]*proj->vals[j][m];
    }
  }
  /* do index calculation, functions return -1 if a problem, which
     is then passed back through optimize0 to tour1d_run */
  /*  if (index (&op->pdata, param, &op->index_best)) return(-1);*/
  if (index (&op->pdata, param, &index_work)) return(-1);

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
      if (index (&op->pdata, param, &index_work)) return(-1);

      if (index_work > op->index_best)
      { /* sprintf (msg, "Success %f", index_work); print(); */
        op->success++;
        /*printf ("Success %f\n", index_work); */
    /*        arrayf_copy (&proj_work, proj); Sigbert's code, I think 
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

  P = (gint *) malloc(n*sizeof(gint));
  inv = (gdouble *) malloc(n*n*sizeof(gdouble));
  d = ludcmp(a,n,P);
 
  b = (gdouble *) malloc(n*sizeof(gdouble));
  for(i=0; i<n; i++)
  {  
    for(j=0; j<n; j++)
    {  
      if(i == j) b[j] = 1.0; else b[j] = 0.0;
    }
    d=solve(a,b,n,P);
     for(j=0; j<n; j++)
       inv[j*n+i] = b[j];
  }
  memcpy(a,inv,n*n*sizeof(gdouble));

  free(P);
  free(inv);
}    

gdouble solve(gdouble *a,gdouble *b,gint n,gint *Pivot) 
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


gdouble ludcmp(gdouble *a,gint n,gint *Pivot) 
{ 
    gint i,j,k,ier;
    gdouble *s,det,temp,c;
    det=1;
    s = (gdouble *) malloc(n*sizeof(gdouble));
    for(i=0;i<n; i++)
    {  s[i] = a[i*n+1];
       for(j=1; j<n; j++)
          if(s[i] < a[i*n+j]) s[i] = a[i*n+j];
    }
    for(k=0;k<n-1; k++)
    {  for(i=k; i<n; i++)
       {   temp = fabs(a[i*n+k]/s[i]);
           if(i==k) { c=temp; Pivot[k]=i;}
           else if(c <temp) {c = temp; Pivot[k]=i;}
       }  
        /* If all elements of a row(or column) of A are zero, |A| = 0 */
       if(c==0) 
       {   det=0;
           return(det);
       }
       if(Pivot[k]!=k)
       {   det*=-1; 
           for(j=k; j<n; j++)
           {   temp =a[k*n+j]; 
               a[k*n+j]=a[Pivot[k]*n+j]; 
               a[Pivot[k]*n+j]=temp;
            }       
           temp = s[k];
           s[k] = s[Pivot[k]];   
           s[Pivot[k]]=temp;
       }
       for(i=k+1; i<n; i++)
       {   temp =a[i*n+k]/a[k*n+k];
           a[i*n+k] = temp;
           for(j=k+1; j<n; j++)
              a[i*n+j] -= temp*a[k*n+j];
       }
       det *= a[k*n+k];
    }
    k = n-1;
    det *= a[(n-1)*n+(n-1)];
    ier=0; 
    free(s);
    return(det);
}                               
  
