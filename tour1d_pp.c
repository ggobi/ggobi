/* tour1d_pp.c */
/* Copyright (C) 2001 Dianne Cook and Sigbert Klinke

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

/* Start of inclusion of Sigbert's tour1d_pp.c */

/*#include "eispack.h"*/
#include "tour1d_pp.h"
#include "tour_pp.h"

/*static gchar msg[1024];*/

/*-- projection pursuit indices --*/
#define PCA            0
#define LDA            1
#define CART_GINI      2
#define CART_ENTROPY   3
#define CART_VAR       4
#define SUBD           5

/*void print()
{ FILE *f = fopen ("dump", "a");
  if (f)
  { fprintf (f, "%s\n", msg);
    fclose(f);
  }
}*/

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
  /*  sprintf (msg, "PCA-Index=%f", *val); print();*/
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
  /*gint ierr;*/ /* eispack */
  if (p==2)
  { lp = 0.5*(*(cov+0) + *(cov+3));
    lq = *(cov+0)* *(cov+3) - *(cov+1)* *(cov+2);
    ew[0] = lp - sqrt(lp*lp-lq);
    ew[1] = lp + sqrt(lp*lp-lq);          
  }
  else
    {}
  /* i want to avoid using eispack */
    /*    rs_ (&p, &p, cov, ew, &matz, ev, fv1, fv2, &ierr);*/
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
        /* printf ("cov=%f,%f,%f,%f\n", cov[0], cov[1], cov[2], cov[3]); */
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
  gint i, j, k, lda, n, job;
  /*gint info, inert[3];*/  /*eispack*/
  /*gfloat detw[2], detwb[2];*/

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
  /* i want to avoid using eispack */
  /*  dsifa_ (dp->a, &lda, &n, dp->kpvt, &info);
      dsidi_ (dp->a, &lda, &n, dp->kpvt, detw, inert, dp->work, &job);*/
  
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
  /* i want to avoid using eispack */
  /*  dsifa_ (dp->a, &lda, &n, dp->kpvt, &info);
      dsidi_ (dp->a, &lda, &n, dp->kpvt, detwb, inert, dp->work, &job);*/
  
  /*  *val = -detw[0]/detwb[0]*pow(10,detw[1]-detwb[1]);*/

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

void t1d_clear_pppixmap(ggobid *gg)
{
  displayd *dsp = gg->current_display;
  gint margin=10;
  gint wid = dsp->t1d_ppda->allocation.width, 
    hgt = dsp->t1d_ppda->allocation.height;

  /* clear the pixmap */
  gdk_gc_set_foreground (gg->plot_GC, &gg->bg_color);
  gdk_draw_rectangle (dsp->t1d_pp_pixmap, gg->plot_GC,
                      true, 0, 0, wid, hgt);

  gdk_gc_set_foreground (gg->plot_GC, &gg->accent_color);
  gdk_draw_line (dsp->t1d_pp_pixmap, gg->plot_GC,
    margin, hgt - margin,
    wid - margin, hgt - margin);
  gdk_draw_line (dsp->t1d_pp_pixmap, gg->plot_GC,
    margin, hgt - margin, margin, margin);

  gdk_draw_pixmap (dsp->t1d_ppda->window, gg->plot_GC, dsp->t1d_pp_pixmap,
                   0, 0, 0, 0,
                   wid, hgt);
}

void t1d_clear_ppda(ggobid *gg)
{
  displayd *dsp = gg->current_display;
  gint i;

  /* clear the ppindx matrix */
  dsp->t1d_ppindx_count = 0;
  dsp->t1d_indx_min=1000.;
  dsp->t1d_indx_max=-1000.;
  for (i=0; i<100; i++) 
  {
    dsp->t1d_ppindx_mat[i] = 0.0;
  }

  t1d_clear_pppixmap(gg);
}

void t1d_ppdraw_all(gint wid, gint hgt, gint margin, ggobid *gg)
{
  displayd *dsp = gg->current_display;
  /*gint xpos, ypos, xstrt, ystrt;*/
  GdkPoint pptrace[100];
  gint i;

  t1d_clear_pppixmap(gg);

  for (i=0; i<dsp->t1d_ppindx_count; i++) 
  {
    pptrace[i].x = margin+i*2;
    pptrace[i].y = hgt-margin-(gint)((gfloat)((dsp->t1d_ppindx_mat[i]-
      dsp->t1d_indx_min)/(gfloat) (dsp->t1d_indx_max-dsp->t1d_indx_min)) * 
      (gfloat) (hgt - 2*margin));
  }
  gdk_draw_lines (dsp->t1d_pp_pixmap, gg->plot_GC,
    pptrace, dsp->t1d_ppindx_count);

  gdk_draw_pixmap (dsp->t1d_ppda->window, gg->plot_GC, dsp->t1d_pp_pixmap,
    0, 0, 0, 0, wid, hgt);

}

/* This is the pp index plot drawing routine */ 
void t1d_ppdraw(gfloat pp_indx_val, ggobid *gg)
{
  displayd *dsp = gg->current_display;
  gint margin=10;
  gint wid = dsp->t1d_ppda->allocation.width, 
    hgt = dsp->t1d_ppda->allocation.height;
  gint j;
  static gboolean init = true;
  gchar *label = g_strdup("PP index: (0.0) 0.0000 (0.0)");

  if (init) {
    t1d_clear_ppda(gg);
    init = false;
  }

  dsp->t1d_ppindx_mat[dsp->t1d_ppindx_count] = pp_indx_val;

  if (dsp->t1d_indx_min > pp_indx_val)
      dsp->t1d_indx_min = pp_indx_val;
  if (dsp->t1d_indx_max < pp_indx_val)
    dsp->t1d_indx_max = pp_indx_val;

  if (dsp->t1d_indx_min == dsp->t1d_indx_max) dsp->t1d_indx_min *= 0.9999;

  label = g_strdup_printf ("PP index: (%3.1f) %5.3f (%3.1f)",
    dsp->t1d_indx_min, dsp->t1d_ppindx_mat[dsp->t1d_ppindx_count], 
    dsp->t1d_indx_max);
  gtk_label_set_text(GTK_LABEL(dsp->t1d_pplabel),label);

  gdk_gc_set_foreground (gg->plot_GC, &gg->accent_color);
  if (dsp->t1d_ppindx_count == 0) 
  {
    dsp->t1d_ppindx_count++;
  }
  else if (dsp->t1d_ppindx_count > 0 && dsp->t1d_ppindx_count < 80) {
    t1d_ppdraw_all(wid, hgt, margin, gg);
    dsp->t1d_ppindx_count++;
  }
  else if (dsp->t1d_ppindx_count >= 80) 
  {
    /* cycle values back into array */
    for (j=0; j<=dsp->t1d_ppindx_count; j++)
      dsp->t1d_ppindx_mat[j] = dsp->t1d_ppindx_mat[j+1];
    t1d_ppdraw_all(wid, hgt, margin, gg);
  }

  g_free (label);
}

/********************************************************************

                             INDEX CALCULATION

The index function has to be defined as

     gint index (array_f *pdata, void *param, gfloat *val)

with   

Input:  pdata   projected data
        param   additional parameters for the index 
                (will not be touched by the optimization routine)

Output: val     the index-value
        
This function should simply calculate the index value for a provided
projection.

*********************************************************************/


gfloat t1d_calc_indx (array_f data, array_f proj, 
                gint *rows, gint nrows, 
                gint ncols,
                gint (*index) (array_f*, void*, gfloat*),
                void *param)
{ 
  gfloat indexval;
  array_f pdata;
  gint i, j, m;

  arrayf_init_null (&pdata);
  arrayf_alloc_zero (&pdata, nrows, 1);

  /* fill projected data array */
  for (m=0; m<nrows; m++)
  { 
    i = rows[m];
    pdata.vals[i][0] = 0.0;
    for (j=0; j<ncols; j++)
    { 
      pdata.vals[i][0] += data.vals[i][j]*proj.vals[0][j];
    }
  }

  index (&pdata, param, &indexval);
  arrayf_free (&pdata, 0, 0);

  return(indexval);
}

gboolean t1d_switch_index(gint indxtype, gint basismeth, ggobid *gg)
{
  displayd *dsp = gg->current_display; 
  datad *d = dsp->d;
  gint kout, nrows = d->nrows_in_plot, pdim = 1;
  subd_param sp; 
  discriminant_param dp;
  cartgini_param cgp;
  cartentropy_param cep;
  cartvariance_param cvp;
  gfloat *gdata;
  gint i, j;

  gdata  = malloc (nrows*sizeof(gfloat));

  for (i=0; i<d->nrows_in_plot; i++)
    for (j=0; j<dsp->t1d.nvars; j++)
      dsp->t1d_pp_op.data.vals[i][j] = 
        d->tform.vals[d->rows_in_plot[i]][dsp->t1d.vars.els[j]];

  for (j=0; j<dsp->t1d.nvars; j++)
    dsp->t1d_pp_op.proj_best.vals[j][0] = 
      dsp->t1d.u.vals[0][dsp->t1d.vars.els[j]];

  if (d->clusterid.els==NULL) printf ("No cluster information found\n");
    for (i=0; i<nrows; i++)
    { 
      if (d->clusterid.els!=NULL)
        gdata[i] = d->clusterid.els[d->rows_in_plot[i]];
      else
        gdata[i] = 0;
    }

  switch (indxtype)
  { 
    case PCA: 
      dsp->t1d.ppval = t1d_calc_indx (d->tform, 
        dsp->t1d.u, d->rows_in_plot, d->nrows, d->ncols, pca, NULL);
      if (basismeth == 1)
        kout = optimize0 (&dsp->t1d_pp_op, pca, &cvp);
      break;
    case LDA: 
      alloc_discriminant_p (&dp, gdata, nrows, pdim);
      dsp->t1d.ppval = t1d_calc_indx (d->tform, 
        dsp->t1d.u, d->rows_in_plot, d->nrows, d->ncols, 
        discriminant, &dp);
      if (basismeth == 1)
        kout = optimize0 (&dsp->t1d_pp_op, discriminant, &dp);
      free_discriminant_p (&dp);
      break;
    case CART_GINI: 
      alloc_cartgini_p (&cgp, nrows, gdata);
      dsp->t1d.ppval = t1d_calc_indx (d->tform, 
        dsp->t1d.u, d->rows_in_plot, d->nrows, d->ncols,
        cartgini, &cgp);
      if (basismeth == 1)
        kout = optimize0 (&dsp->t1d_pp_op, cartgini, &cgp);
      free_cartgini_p (&cgp);
      break;
    case CART_ENTROPY: 
      alloc_cartentropy_p (&cep, nrows, gdata);
      dsp->t1d.ppval = t1d_calc_indx (d->tform, 
        dsp->t1d.u, d->rows_in_plot, d->nrows, d->ncols, 
        cartentropy, &cep);
      if (basismeth == 1)
        kout = optimize0 (&dsp->t1d_pp_op, cartentropy, &cep);
      free_cartentropy_p (&cep);
      break;
    case CART_VAR: 
      alloc_cartvariance_p (&cvp, nrows, gdata);
      dsp->t1d.ppval = t1d_calc_indx (d->tform, 
        dsp->t1d.u, d->rows_in_plot, d->nrows, d->ncols, 
        cartvariance, &cvp);
      if (basismeth == 1)
        kout = optimize0 (&dsp->t1d_pp_op, cartentropy, &cep);
      free_cartvariance_p (&cvp);
      break;
    case SUBD: 
      alloc_subd_p (&sp, nrows, pdim);
      dsp->t1d.ppval  = t1d_calc_indx (d->tform, dsp->t1d.u, 
        d->rows_in_plot, d->nrows, d->ncols, subd, &sp);
      if (basismeth == 1)
        kout  = optimize0 (&dsp->t1d_pp_op, subd, &sp);
      free_subd_p (&sp);
      break;
    default: 
      free (gdata);
      return(true);
      break;
  }
  free (gdata);
  return(false);
}

#undef SUBD           
#undef LDA            
#undef CART_GINI      
#undef CART_ENTROPY   
#undef CART_VAR       
#undef PCA            
