/* tour1d.c */

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

/* Start of inclusion of Sigbert's tour1d_pp.c */

#include "eispack.h"
#include "tour1d_pp.h"


char msg[1024];

void print()
{ FILE *f = fopen ("dump", "a");
  if (f)
  { fprintf (f, "%s\n", msg);
    fclose(f);
  }
}

/********************************************************************

Index          : PCA-d
Transformation : -
Purpose        : computes the trace of the cov matrix of pdata
Note           : Modifies pdata !

*********************************************************************/

void center (array_f *data)
{ gint i, j;
  gfloat mean;
  for (i=0; i<data->ncols; i++)
  { mean = 0.0;
    for (j=0; j<data->nrows; j++)
      mean += data->vals[j][i];
    mean = mean/data->nrows;
    for (j=0; j<data->nrows; j++)
      data->vals[j][i] -= mean;
  }
}

gint pca (array_f *pdata, void *param, gfloat *val)
{ gint i, j;

  center (pdata);
  /* dump (pdata, 6, 1); */

  *val = 0.0;
  for (i=0; i<pdata->ncols; i++)
  { for (j=0; j<pdata->nrows; j++)
      *val += pdata->vals[j][i]*pdata->vals[j][i];
  }
  *val /= (pdata->nrows-1);
  sprintf (msg, "PCA-Index=%f", *val); print();
  return (0);
}

/********************************************************************

Index          : SUB-d
Transformation : Data should be standardized
Purpose        : Looks for d-dimensional structure in the projected data

                 For this purpose a local PCA with k neighbours
                 (k = min_neighbour+1, min_neighbour+1+neighbour_step,
                      min_neighbour+1+2*neighbour_step,.., max_neighbour)
                 is computed. We average the maximal explained variance
                 for the datapoints 1, 1+data_step, 1+2*data_step, ...
Note           : Requires eispack.c
*********************************************************************/

gint alloc_subd_p (subd_param *sp, gint nrows, gint ncols)
{
  /* Some default values */
  sp->min_neighbour  = 10;
  sp->max_neighbour  = 30;
  sp->dim            =  1;
  sp->data_step      =  1;
  sp->neighbour_step =  1;

  /* initialize temporary space */
  sp->dist  = malloc (nrows*sizeof(gfloat));
  sp->index = malloc (nrows*sizeof(gint));
  sp->nmean = malloc (ncols*sizeof(gfloat));
  sp->mean  = malloc (ncols*sizeof(gfloat));
  sp->ew    = malloc (ncols*sizeof(gfloat));
  sp->ev    = malloc (ncols*ncols*sizeof(gfloat));
  sp->fv1   = malloc (ncols*sizeof(gfloat));
  sp->fv2   = malloc (ncols*sizeof(gfloat));
  sp->cov   = malloc (ncols*ncols*sizeof(gfloat));

  return 0;
}

gint free_subd_p (subd_param *sp)
{
  free(sp->dist);
  free(sp->index);
  free(sp->nmean);
  free(sp->mean);
  free(sp->ew);
  free(sp->ev);
  free(sp->fv1);
  free(sp->fv2);
  free(sp->cov);

  return 0;
}

gfloat *base;

int smallest (const void *left, const void *right)
{ gfloat l = base[*((gint *) left)], r = base[*((gint *) right)];
  if (l<r) return (-1);
  if (l>r) return (1);
  return (0);
}

void distance (array_f *pdata, gint i, gfloat *dist)
{ gint j, k;
  for (j=0; j<pdata->nrows; j++)
  { dist[j]  = 0;
    for (k=0; k<pdata->ncols; k++)
      dist[j] += (pdata->vals[i][k] - pdata->vals[j][k])
	        *(pdata->vals[i][k] - pdata->vals[j][k]);
  }
}

void mean_min_neighbour (array_f *pdata, gint *index, int min_neighbour, gfloat *nmean)
{ gint j, k;
  for (k = 0; k<pdata->ncols; k++) nmean[k] = 0;
  for (j = 0; j<min_neighbour; j++)
  { for (k = 0; k<pdata->ncols; k++)
      nmean[k] += pdata->vals[index[j]][k];
  }
}

void covariance (array_f *pdata, gint *index, int j, gfloat *mean, gfloat *cov)
{ gint k, p, q;
  for (p=0; p<pdata->ncols; p++)      
  { for (q=0; q<=p; q++) 
    { *(cov+p*pdata->ncols+q) = 0.0;
      for (k=0; k<=j; k++)
      { *(cov+p*pdata->ncols+q) += (pdata->vals[index[k]][p]-mean[p])
	                          *(pdata->vals[index[k]][q]-mean[q]);
      }
      *(cov+p*pdata->ncols+q) /= j;
      *(cov+q*pdata->ncols+p) = *(cov+p*pdata->ncols+q);
    }
  }
}

gfloat variance_explained (gfloat *ew, gint d, gint p)
{ gfloat ewsum = 0, dimsum = 0;
  gint k;
  for (k=0; k<p; k++)      
  { ewsum += ew[k];
    if (k>=p-d) dimsum += ew[k];
  }
  return (dimsum/ewsum);
}

void eigenvalues (gfloat *cov, gint p, gfloat *ew, 
                  gint matz, gfloat *ev, gfloat *fv1, gfloat *fv2)
{ gfloat lp, lq;
  gint ierr;
  if (p==2)
  { lp = 0.5*(*(cov+0) + *(cov+3));
    lq = *(cov+0)* *(cov+3) - *(cov+1)* *(cov+2);
    ew[0] = lp - sqrt(lp*lp-lq);
    ew[1] = lp + sqrt(lp*lp-lq);          
  }
  else
    rs_ (&p, &p, cov, ew, &matz, ev, fv1, fv2, &ierr);
}      

gint subd (array_f *pdata, void *param, gfloat *val)
{ subd_param *sp = (subd_param *) param;
  gfloat varexp, dimmax;
  gint i, j, k, matz = 0, nused;

  *val  = 0;

  nused = 0;
  for (i=0; i<pdata->nrows; i+= sp->data_step, nused++)
  { 
    distance (pdata, i, sp->dist);

    for (j=0; j<pdata->nrows; j++) sp->index[j] = j;
    base = sp->dist;
    qsort (sp->index, pdata->nrows, sizeof(gint), smallest);

    mean_min_neighbour (pdata, sp->index, sp->min_neighbour, sp->nmean);

    dimmax = 0;
    for (j = sp->min_neighbour; j<sp->max_neighbour; j++)
    { 
      for (k = 0; k<pdata->ncols; k++)
        sp->nmean[k] += pdata->vals[sp->index[j]][k]; 
      if (((j-sp->min_neighbour)%sp->neighbour_step)==0)
      {
        for (k = 0; k<pdata->ncols; k++) sp->mean[k] = sp->nmean[k]/(j+1);
        covariance (pdata, sp->index, j, sp->mean, sp->cov);
	/*	printf ("cov=%f,%f,%f,%f\n", cov[0], cov[1], cov[2], cov[3]); */
        eigenvalues (sp->cov, pdata->ncols, sp->ew, matz, sp->ev, sp->fv1, sp->fv2);
	/* printf ("ew[0]=%f ew[1]=%f\n", ew[0], ew[1]); */
        varexp = variance_explained (sp->ew, sp->dim, pdata->ncols);
	/* printf ("varexp=%f\n", varexp); */
        dimmax = (varexp>dimmax ? varexp : dimmax);
      }
    }
    *val += dimmax;
  }
  *val /= nused;
  return (0);
}

/********************************************************************

Index          : Discriminant
Transformation : -
Purpose        : Looks for the best projection to discriminate
                 between groups.
*********************************************************************/

gint zero (gfloat *ptr, gint length)
{ gint i;
  for (i=0; i<length; i++)
    ptr[i] = 0;
  return (0);
}

gint compute_groups (gint *group, gint *ngroup, gint *groups, gint nrows, gfloat *gdata)
{ gint i, j, *groupval;

  /* initialize data */
  groupval = malloc (nrows*sizeof(gint));

  *groups = 0;
  for (i=0; i<nrows; i++)
  { for (j=0; j<*groups; j++)
    { if (groupval[j]==gdata[i])
      { ngroup[j]++;
        break;
      }
    }
    if (j==*groups )
    { groupval[j]  = gdata[i];
      ngroup[j] = 1;
      (*groups)++;
    }
  }

  for (i=0; i<nrows; i++)
  { for (j=0; j<*groups; j++)
    { if (groupval[j]==gdata[i])
        group[i] = j;
    }
  }

  return ((*groups==1) || (*groups==nrows));
}

gint alloc_discriminant_p (discriminant_param *dp, gfloat *gdata, gint nrows, gint ncols)
{
  dp->group    = malloc (nrows*sizeof(gint));
  dp->ngroup   = malloc (nrows*sizeof(gint));

  if (compute_groups (dp->group, dp->ngroup, &(dp->groups), nrows, gdata)) return (1);

  /* initialize temporary space */
  dp->cov      = malloc (ncols*ncols*sizeof(gfloat));
  dp->a        = malloc (ncols*ncols*sizeof(gfloat));
  dp->mean     = malloc (nrows*ncols*sizeof(gfloat));
  dp->ovmean   = malloc (ncols*sizeof(gfloat));
  dp->kpvt     = malloc (ncols*sizeof(gint));
  dp->work     = malloc (nrows*sizeof(gint));

  return 0;
}

gint free_discriminant_p (discriminant_param *dp)
{ free(dp->group);
  free(dp->ngroup);
  free(dp->cov);
  free(dp->a);
  free(dp->mean);
  free(dp->ovmean);
  free(dp->kpvt);
  free(dp->work);

  return 0;
}

gint discriminant (array_f *pdata, void *param, gfloat *val)
{ discriminant_param *dp = (discriminant_param *) param;
  gint i, j, k, lda, n, info, inert[3], job;
  gfloat detw[2], detwb[2];

  /* Compute means */

  zero (dp->mean, dp->groups*pdata->ncols);
  zero (dp->ovmean, pdata->ncols);
  zero (dp->cov, pdata->ncols*pdata->ncols);

  for (i=0; i<pdata->nrows; i++)
  { for (k=0; k<pdata->ncols; k++)
    { dp->mean[k*pdata->nrows+dp->group[i]] += pdata->vals[i][k];  
      dp->ovmean[k] += pdata->vals[i][k];
    }
  }

  for (k=0; k<pdata->ncols; k++)
  { for (i=0; i<dp->groups; i++)
    { dp->mean[k*pdata->nrows+i] /= dp->ngroup[i];
    /*     sprintf (msg, "mean[%i,%i]=%f", i, k, dp->mean[k*pdata->nrows+i]); print();   */
    }
    dp->ovmean[k] /= pdata->nrows;
  }

  /* Compute W */

  for (i=0; i<pdata->nrows; i++)
  { for (j=0; j<pdata->ncols; j++)
    { for (k=0; k<=pdata->ncols; k++)
      { dp->cov[k*pdata->ncols+j] += 
        (pdata->vals[i][j]-dp->mean[j*pdata->nrows+dp->group[i]])*
	(pdata->vals[i][k]-dp->mean[k*pdata->nrows+dp->group[i]])/
         dp->ngroup[dp->group[i]];
      }
      dp->cov[j*pdata->ncols+k] = dp->cov[k*pdata->ncols+j];
    }
  }

  lda = pdata->ncols;
  n   = pdata->ncols;
  job = 10;

  memcpy (dp->a, dp->cov, pdata->ncols*pdata->ncols*sizeof(gfloat));
  dsifa_ (dp->a, &lda, &n, dp->kpvt, &info);
  dsidi_ (dp->a, &lda, &n, dp->kpvt, detw, inert, dp->work, &job);
  
  /* Compute W+B */

  for (i=0; i<dp->groups; i++)
  { for (j=0; j<pdata->ncols; j++)
    { for (k=0; k<pdata->ncols; k++)
      { dp->cov[k*pdata->ncols+j] += 
          (dp->mean[k*pdata->nrows+i]-dp->ovmean[k])*
          (dp->mean[j*pdata->nrows+i]-dp->ovmean[j])/
           dp->groups;
      }
    }
  }

  memcpy (dp->a, dp->cov, pdata->ncols*pdata->ncols*sizeof(gfloat));
  dsifa_ (dp->a, &lda, &n, dp->kpvt, &info);
  dsidi_ (dp->a, &lda, &n, dp->kpvt, detwb, inert, dp->work, &job);
  
  *val = -detw[0]/detwb[0]*pow(10,detw[1]-detwb[1]);

  printf ("Index=%f\n", *val);

/*  sprintf (msg, "index=%f\n", *val); print(); */
  return (0);
}

/********************************************************************

Index          : CartGini, CartEntropy, CartVariance
Transformation : -
Purpose        : Looks for the best split in 1d-projected data.

*********************************************************************/

gint alloc_cartgini_p (cartgini_param *dp, gint nrows, gfloat *gdata)
{ /* initialize data */

  dp->group    = malloc (nrows*sizeof(gint));
  dp->ngroup   = malloc (nrows*sizeof(gint));
  if (compute_groups (dp->group, dp->ngroup, &(dp->groups), nrows, gdata)) return (1);

  /* initialize temporary space */
  dp->x        = malloc (nrows*sizeof(gfloat));
  dp->nright   = malloc (nrows*sizeof(gint));
  dp->index    = malloc (nrows*sizeof(gint));

  return 0;
}

gint free_cartgini_p (cartgini_param *dp)
{ free (dp->ngroup);
  free (dp->group);
  free (dp->x);
  free (dp->index);
  free (dp->nright);

  return 0;
}

gint cartgini (array_f *pdata, void *param, gfloat *val)
{ cartgini_param *dp = (cartgini_param *) param;
  gint i, k;
  gfloat dev, prob;
 
  if (pdata->ncols!=1) return(-1);
  for (i=0; i<pdata->nrows; i++) 
  { dp->x[i] = pdata->vals[i][0];
    dp->index[i] = i;
  }

  base = dp->x;
  qsort (dp->index, pdata->nrows, sizeof(gint), smallest);
 
  *val = 2;
  for (i=0; i<dp->groups; i++)
  { dp->nright[i] = 0;
    *val -= (dp->ngroup[i]/pdata->nrows)*(dp->ngroup[i]/pdata->nrows);
  }
  
  for (i=0; i<pdata->nrows-1; i++)
  { (dp->nright[dp->group[dp->index[i]]])++;
    dev = 2;
    for (k=0; k<dp->groups; k++)
    { prob = ((gfloat) dp->nright[k])/((gfloat) (i+1));
      dev -= prob*prob;
      prob = ((gfloat) (dp->ngroup[k]-dp->nright[k]))/((gfloat) (pdata->nrows-i-1));
      dev -= prob*prob;
    }
    if (dev<*val) *val = dev;
  }

  *val *= -1;
/*  sprintf (msg, "Index=%f", *val);print();              */

  return(0);
}

gint alloc_cartentropy_p (cartentropy_param *dp, gint nrows, gfloat *gdata)
{ /* initialize data */
  dp->group    = malloc (nrows*sizeof(gint));
  dp->ngroup   = malloc (nrows*sizeof(gint));
  if (compute_groups (dp->group, dp->ngroup, &(dp->groups), nrows, gdata)) return (1);

  /* initialize temporary space */
  dp->x        = malloc (nrows*sizeof(gfloat));
  dp->nright   = malloc (nrows*sizeof(gint));
  dp->index    = malloc (nrows*sizeof(gint));

  return 0;
}

gint free_cartentropy_p (cartentropy_param *dp)
{ free (dp->ngroup);
  free (dp->group);
  free (dp->x);
  free (dp->index);
  free (dp->nright);

  return 0;
}

gint cartentropy (array_f *pdata, void *param, gfloat *val)
{ cartentropy_param *dp = (cartentropy_param *) param;
  gint i, k;
  gfloat dev, prob;
 
  if (pdata->ncols!=1) return(-1);
  for (i=0; i<pdata->nrows; i++) 
  { dp->x[i] = pdata->vals[i][0];
    dp->index[i] = i;
  }

  base = dp->x;
  qsort (dp->index, pdata->nrows, sizeof(gint), smallest);
 
  *val = 0;
  for (i=0; i<dp->groups; i++)
  { dp->nright[i] = 0;
    prob  = dp->ngroup[i]/pdata->nrows;
    if (prob>0) *val += prob*log(prob);
  }
  
  for (i=0; i<pdata->nrows-1; i++)
  { (dp->nright[dp->group[dp->index[i]]])++;
    dev = 0;
    for (k=0; k<dp->groups; k++)
    { prob = ((gfloat) dp->nright[k])/((gfloat) (i+1));
      if (prob>0) dev += prob*log(prob);
      prob = ((gfloat) (dp->ngroup[k]-dp->nright[k]))/((gfloat) (pdata->nrows-i-1));
      if (prob>0) dev += prob*log(prob);
    }
    if (dev<*val) *val = dev;
  }

  *val *= -1;
/*  sprintf (msg, "Index=%f", *val); print(); */
  return(0);
}

gint alloc_cartvariance_p (cartvariance_param *dp, gint nrows, gfloat *gdata)
{ gint i;
  /* initialize data */

  dp->y = malloc (nrows*sizeof(gfloat));

  for (i=0; i<nrows; i++)
    dp->y[i] = gdata[i];

  /* initialize temporary space */
  dp->x        = malloc (nrows*sizeof(gfloat));
  dp->index    = malloc (nrows*sizeof(gint));

  return 0;
}

gint free_cartvariance_p (cartvariance_param *dp)
{ free (dp->y);
  free (dp->x);
  free (dp->index);

  return 0;
}

gint cartvariance (array_f *pdata, void *param, gfloat *val)
{ cartvariance_param *dp = (cartvariance_param *) param;
  gint i, j;
  gfloat mul, mur, dev;
 
  if (pdata->ncols!=1) return(-1);

  for (i=0; i<pdata->nrows; i++) 
  { dp->x[i] = pdata->vals[i][0];
    dp->index[i] = i;
  }

  base = dp->x;
  qsort (dp->index, pdata->nrows, sizeof(gint), smallest);
 
  mur = 0;
  for (i=0; i<pdata->nrows; i++)
    mur += dp->y[i];
  mur /= pdata->nrows;
  *val = 0;
  for (i=0; i<pdata->nrows; i++)
    *val += (dp->y[i]-mur)*(dp->y[i]-mur);
  
  for (i=1; i<pdata->nrows; i++)
  { mur = mul = 0;
    for (j = 0; j<i; j++)
      mul += dp->y[dp->index[j]];
    mul /= i;
    for (j = i; j<pdata->nrows; j++)
      mur += dp->y[dp->index[j]];
    mur /= (pdata->nrows-i);
    dev = 0;
    for (j = 0; j<i; j++)
      dev += (dp->y[dp->index[j]]-mul)*(dp->y[dp->index[j]]-mul);
    for (j = i; j<pdata->nrows; j++)
      dev += (dp->y[dp->index[j]]-mur)*(dp->y[dp->index[j]]-mur);
    if (dev<*val) *val = dev;
  }
  
/*  sprintf (msg, "Index=%f", *val); print();  */
  return(0);
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

gint alloc_optimize0_p (optimize0_param *op, gint nrows, gint ncols)
{
  op->temp_start     =  1;
  op->temp_end       =  0.001;
  op->cooling        =  0.99;
  /* is equivalent to log(temp_end/temp_start)/log(cooling) projections */
  op->heating        =  1;
  op->restart        =  1;
  op->success        =  0;
  op->temp           =  1;
  op->maxproj        =  op->restart*(1+log(op->temp_end/op->temp_start)/log(op->cooling)); /* :) */
  arrayf_null (&op->proj_best);
  arrayf_alloc_zero (&op->proj_best, nrows, ncols);
  arrayf_null (&op->data);

  return 0;
}

gint free_optimize0_p (optimize0_param *op)
{ arrayf_free (&op->proj_best, 0, 0);
  arrayf_free (&op->data, 0, 0);

  return 0;
}

gboolean iszero (array_f *data)
{ gfloat sum = 0;
  gint i, j;
  for (i=0; i<data->nrows; i++)
  { for (j=0; j<data->ncols; j++)
      sum += fabs(data->vals[i][j]);
  }
  return (sum<1e-6);
}

gfloat randomval, nrand;
gint nset;

void initrandom(gfloat start)
{ randomval = floor (fmod (fabs(start), 62748517.0));
  nset   = 0;
}

gfloat uniformrandom()
{ randomval = fmod (27132.0 * randomval + 7.0, 62748517.0);
  return (randomval / 62748517.0);
}

gfloat normalrandom()
{ gfloat x, y, r;
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
{ int i, j;
  for (i=0; i<data->nrows; i++)
  { for (j=0; j<data->ncols; j++)
      data->vals[i][j] = base->vals[i][j]+delta*normalrandom();
  }
}

void orthonormal (array_f *proj)
{ gint i, j, k;
  gfloat *ip = malloc (proj->ncols*sizeof(gfloat)), norm;
  for (i=0; i<proj->ncols; i++)
  { /* Compute inner product between p_i and all p_j */
    for (j=0; j<i; j++)
    { ip[j] = 0;
      for (k=0; k<proj->nrows; k++)
        ip[j] += proj->vals[k][j]*proj->vals[k][i];
    }
    /* Subtract now all vectors from p_i */
    for (j=0; j<i; j++)
    { for (k=0; k<proj->nrows; k++)
        proj->vals[k][i] -= ip[j]*proj->vals[k][j];
    }
    /* Finally norm vector p_i */
    norm = 0.0;
    for (k=0; k<proj->nrows; k++)
      norm += (proj->vals[k][i]*proj->vals[k][i]);
    norm = sqrt(norm);
    for (k=0; k<proj->nrows; k++)
      proj->vals[k][i] /= norm;
  }
}

gint optimize0 (optimize0_param *op,
                gint (*index) (array_f*, void*, gfloat*),
                void *param)
{ gfloat index_work;
  array_f proj_work, pdata, *proj;
  int i,j, m, k;

  proj = &(op->proj_best);
  arrayf_null (&proj_work);
  arrayf_alloc_zero (&proj_work, proj->nrows, proj->ncols);
  arrayf_null (&pdata);
  arrayf_alloc_zero (&pdata, op->data.nrows, proj->ncols);

  if (iszero(proj))
  { /* sprintf (msg, "zero projection matrix"); print(); */
    normal_fill (proj, 1.0, proj);
  }
  orthonormal (proj);

  for (i=0; i<op->data.nrows; i++)
  { for (j=0; j<proj->ncols; j++)
    { pdata.vals[i][j] = 0;
      for (m=0; m<op->data.ncols; m++)
        pdata.vals[i][j] += op->data.vals[i][m]*proj->vals[m][j];
    }
  }
  if (index (&pdata, param, &op->index_best)) return(-1);

  arrayf_copy (proj, &proj_work);

  op->success = k = 0;
  while (op->restart>0)
  { /* sprintf (msg, "Restart %i", op->restart); print(); */
    while (op->temp>op->temp_end)
    { /* sprintf (msg, "Iteration %i", k); print(); */
      normal_fill (&proj_work, op->temp, proj);
      orthonormal (&proj_work);                              
      op->temp *= op->cooling;

      for (i=0; i<op->data.nrows; i++)
      { for (j=0; j<proj->ncols; j++)
        { pdata.vals[i][j] = 0;
          for (m=0; m<op->data.ncols; m++)
            pdata.vals[i][j] += op->data.vals[i][m]*proj_work.vals[m][j];
        }
      }

      if (index (&pdata, param, &index_work)) return(-1);
      if (index_work>op->index_best)
      { /* sprintf (msg, "Success %f", index_work); print(); */
        op->success++;
        /*printf ("Success %f\n", index_work); */
        arrayf_copy (&proj_work, proj);
        op->index_best = index_work;
        op->temp *= op->heating;
      }
      k++; 
      if (k>=op->maxproj) 
      { printf ("Best =%f\n", op->index_best);
        for (i=0; i<proj->nrows; i++)        
	{ for (j=0; j<proj->ncols; j++)
  	    printf ("%+5.3f ", proj->vals[i][j]);
  	  printf ("\n");
        }
        printf ("\n");
        return(k);
      }
    }
    op->temp = op->temp_start;
    op->restart--;
  } 

  printf ("Best =%f\n", op->index_best);

  return (k);
}

/* End of inclusion of Sigbert's tour1d_pp.c */

/* This function interacts with control  buttons in ggobi */
void t1d_optimz(gint optimz_on, gboolean *nt, gint *bm) {
  gboolean new_target = *nt;
  gint bas_meth = *bm;

  if (optimz_on) 
    bas_meth = 1;
  else
    bas_meth = 0;

  new_target = true;

  *nt = new_target;
  *bm = bas_meth;
}
