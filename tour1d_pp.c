/* tour1d_pp.c */
/* Copyright (C) 2001, 2002 Dianne Cook and Sigbert Klinke and Eun-Kyung Lee

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
#ifndef Darwin
#include <malloc.h>
#endif

#include "vars.h"
#include "externs.h"

#include "tour1d_pp.h"
#include "tour_pp.h"

/*static gchar msg[1024];*/

/*-- projection pursuit indices --*/
#define HOLES          0
#define CENTRAL_MASS   1
#define PCA            2
#define LDA            3
#define CGINI          4
#define CENTROPY       5
#define CART_VAR       6
#define SUBD           7

#define EXPMINUS1 0.3678794411714423
#define ONEMINUSEXPMINUS1 0.63212056

/* void print()
{ FILE *f = fopen ("dump", "a");
  if (f)
  { fprintf (f, "%s\n", msg);
    fclose(f);
  }
}*/

void t1d_pptemp_set(gfloat slidepos, displayd *dsp, ggobid *gg) {
  dsp->t1d_pp_op.temp_start = slidepos;
}

void t1d_ppcool_set(gfloat slidepos, displayd *dsp, ggobid *gg) {
  dsp->t1d_pp_op.cooling = slidepos;
}

/* void
alloc_holes1d_p(holes_param *hp, gint nrows)
{
  hp->h0 = (gfloat *) g_malloc(
    (guint) nrows*sizeof(gfloat *));
}

void
free_holes1d_p(holes_param *hp)
{
  g_free(hp->h0);
}
*/

/***************************************************/
/*  1D Holes index for raw data                    */
/***************************************************/

/* gint holes1d_raw1(array_f *pdata, void *param, gfloat *val)
{  
   gint i, n=pdata->nrows;
   gfloat m1, x1, temp;
   gfloat var, acoefs;

   m1=0;
   for(i=0; i<n; i++)
     m1 += pdata->vals[i][0];
   m1 /= n;

   var = 0;
   for(i=0; i<n; i++)
     var += (pdata->vals[i][0]-m1)*(pdata->vals[i][0]-m1)/(n-1);

   acoefs=0.;

   for(i=0; i<n; i++)
   {  
     x1 = pdata->vals[i][0]-m1; 
     temp = x1*x1/var;
     acoefs +=exp(-temp/2);
   }

   *val = (1.-acoefs/n)/(gfloat) ONEMINUSEXPMINUS1;
   return(0);
}
*/
/**********************************************************/
/*  1D Central Mass index for raw data                    */
/**********************************************************/

/* gint central_mass1d_raw1(array_f *pdata, void *param, gfloat *val)
{
   gint i, n=pdata->nrows;
   gfloat m1, x1, temp;
   gfloat var, acoefs;

   m1=0;
   for(i=0; i<n; i++)
     m1 += pdata->vals[i][0];
   m1 /= n;

   var = 0;
   for(i=0; i<n; i++)
     var += (pdata->vals[i][0]-m1)*(pdata->vals[i][0]-m1)/(n-1);

   acoefs=0.;

   for(i=0; i<n; i++)
   {  
     x1 = pdata->vals[i][0]-m1; 
     temp = x1*x1/var;
     acoefs +=exp(-temp/2);
   }

   *val = (acoefs/n-(gfloat)EXPMINUS1)/(gfloat) ONEMINUSEXPMINUS1;
   return(0);
}
*/
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

  *val = 0.0;
  for (i=0; i<pdata->ncols; i++)
  { for (j=0; j<pdata->nrows; j++)
      *val += pdata->vals[j][i]*pdata->vals[j][i];
  }
  *val /= (pdata->nrows-1);

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

/*gint alloc_subd_p (subd_param *sp, gint nrows, gint ncols)
{
  * Some default values *
  sp->min_neighbour  = 10;
  sp->max_neighbour  = 30;
  sp->dim            =  1;
  sp->data_step      =  1;
  sp->neighbour_step =  1;

  * initialize temporary space *
  sp->dist  = g_malloc (nrows*sizeof(gfloat));
  sp->index = g_malloc (nrows*sizeof(gint));
  sp->nmean = g_malloc (ncols*sizeof(gfloat));
  sp->mean  = g_malloc (ncols*sizeof(gfloat));
  sp->ew    = g_malloc (ncols*sizeof(gfloat));
  sp->ev    = g_malloc (ncols*ncols*sizeof(gfloat));
  sp->fv1   = g_malloc (ncols*sizeof(gfloat));
  sp->fv2   = g_malloc (ncols*sizeof(gfloat));
  sp->cov   = g_malloc ((ncols+ncols)*sizeof(gfloat));

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

  if (p==2)
  { lp = 0.5*(*(cov+0) + *(cov+3));
    lq = *(cov+0)* *(cov+3) - *(cov+1)* *(cov+2);
    ew[0] = lp - sqrt(lp*lp-lq);
    ew[1] = lp + sqrt(lp*lp-lq);          
  }
  else
    {}
    *    rs_ (&p, &p, cov, ew, &matz, ev, fv1, fv2, &ierr);*
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
        eigenvalues (sp->cov, pdata->ncols, sp->ew, matz, sp->ev, sp->fv1, sp->fv2);
        varexp = variance_explained (sp->ew, sp->dim, pdata->ncols);
        dimmax = (varexp>dimmax ? varexp : dimmax);
      }
    }
    *val += dimmax;
  }
  *val /= nused;
  return (0);
}
*/
/*gint cartgini (array_f *pdata, void *param, gfloat *val)
{ 
  cartgini_param *dp = (cartgini_param *) param;
  gint i, k, n, p, g = dp->groups, left, right;
  gfloat dev, prob;

  n = pdata->nrows;
  p = pdata->ncols;
  if (p != 1) 
    return(-1);

* Sort pdata by group *
  right = pdata->nrows-1;
  left = 0;
  sort_group(pdata,dp->group,left,right);

* data relocation and make index *
  zero(dp->x,n);
  zero_int(dp->index,n);

  for (i=0; i<n; i++) {	
    dp->x[i] = pdata->vals[i][0];
    dp->index[i] = dp->group[i];
  }

  left=0;
  right=n-1;
  sort_data(dp->x, dp->index,left,right) ;

 * Calculate gini index *
  zero_int(dp->nright,g);
  *val = 1;
  for (i=0; i<g; i++) {	
    dp->nright[i] = 0;
    *val -= (((gdouble)dp->ngroup[i])/((gdouble)n))*
      (((gdouble)dp->ngroup[i])/((gdouble)n));
  }
  for (i=0; i<n-1; i++)  {
    (dp->nright[(dp->index[i])])++;
    dev=2;
    for (k=0; k<g; k++) {
      prob = ((double) dp->nright[k])/((double)(i+1));
      dev -= prob*prob;
      prob = ((double) (dp->ngroup[k]-dp->nright[k]))/((double)(n-i-1));
      dev -= prob*prob;
    }
    if (dev<*val) *val = dev;
  } 
  *val = (1-*val);

  return(0);
}*/


/*gint cartentropy (array_f *pdata, void *param, gfloat *val)
{ 
  cartentropy_param *dp = (cartentropy_param *) param;

  gint i, k, n, p, g = dp->groups, left, right;
  gfloat dev, prob;

  n = pdata->nrows;
  p = pdata->ncols;
  if (p != 1) 
    return(-1);

* Sort pdata by group *
  right = pdata->nrows-1;
  left = 0;
  sort_group(pdata,dp->group,left,right);

* data relocation and make index *
  zero(dp->x,n);
  zero_int(dp->index,n);

  for (i=0; i<n; i++) {	
    dp->x[i] = pdata->vals[i][0];
    dp->index[i] = dp->group[i];
  }

  left=0;
  right=n-1;
  sort_data(dp->x, dp->index,left,right) ;

* Calculate entropy index *
  zero_int(dp->nright,g);
  *val = 0;
  for (i=0; i<dp->groups; i++)
  { dp->nright[i] = 0;
    prob  = dp->ngroup[i]/pdata->nrows;
    if (prob>0) *val += prob*log(prob);
  }
  
  for (i=0; i<pdata->nrows-1; i++)
  { (dp->nright[dp->index[i]])++;
    dev = 0;
    for (k=0; k<dp->groups; k++)
    { prob = ((gdouble) dp->nright[k])/((gdouble) (i+1));
      if (prob>0) dev += prob*log(prob);
      prob = ((gdouble) (dp->ngroup[k]-dp->nright[k]))/
        ((gdouble) (pdata->nrows-i-1));
      if (prob>0) dev += prob*log(prob);
    }
    if (dev<*val) *val = dev;
  }

  *val = (1-*val);

  return(0);
}
*/
/*gint alloc_cartvariance_p (cartvariance_param *dp, gint nrows, gfloat *gdata)
{ gint i;
  * initialize data *

  dp->y = g_malloc (nrows*sizeof(gfloat));

  for (i=0; i<nrows; i++)
    dp->y[i] = gdata[i];

  * initialize temporary space *
  dp->x        = g_malloc (nrows*sizeof(gfloat));
  dp->index    = g_malloc (nrows*sizeof(gint));

  return 0;
}

gint free_cartvariance_p (cartvariance_param *dp)
{ g_free (dp->y);
  g_free (dp->x);
  g_free (dp->index);

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
  
  return(0);
}
*/

/* This function interacts with control  buttons in ggobi */
void t1d_optimz(gint optimz_on, gboolean *nt, gint *bm, displayd *dsp) {
  gboolean new_target = *nt;
  gint bas_meth = *bm;
  gint i, j;

  if (optimz_on) {
    for (i=0; i<1; i++)
      for (j=0; j<dsp->t1d.nactive; j++)
        dsp->t1d_pp_op.proj_best.vals[i][j] = 
          dsp->t1d.F.vals[i][dsp->t1d.active_vars.els[j]];
    /*    dsp->t1d.ppval = dsp->t1d_indx_min;*/
    dsp->t1d_pp_op.index_best = 0.0;
    bas_meth = 1;
  }
  else {
    bas_meth = 0;
  }

  new_target = true;

  *nt = new_target;
  *bm = bas_meth;
}

void t1d_clear_pppixmap(displayd *dsp, ggobid *gg)
{
  colorschemed *scheme = gg->activeColorScheme;
  gint margin=10;
  gint wid = dsp->t1d_ppda->allocation.width, 
    hgt = dsp->t1d_ppda->allocation.height;

  /* clear the pixmap */
  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_bg);
  gdk_draw_rectangle (dsp->t1d_pp_pixmap, gg->plot_GC,
                      true, 0, 0, wid, hgt);

  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
  gdk_draw_line (dsp->t1d_pp_pixmap, gg->plot_GC,
    margin, hgt - margin,
    wid - margin, hgt - margin);
  gdk_draw_line (dsp->t1d_pp_pixmap, gg->plot_GC,
    margin, hgt - margin, margin, margin);

  gdk_draw_pixmap (dsp->t1d_ppda->window, gg->plot_GC, dsp->t1d_pp_pixmap,
                   0, 0, 0, 0,
                   wid, hgt);
}

void t1d_clear_ppda(displayd *dsp, ggobid *gg)
{
  gint i;

  /* clear the ppindx matrix */
  dsp->t1d_ppindx_count = 0;
  dsp->t1d_indx_min=1000.;
  dsp->t1d_indx_max=-1000.;
  for (i=0; i<100; i++) 
  {
    dsp->t1d_ppindx_mat[i] = 0.0;
  }

  t1d_clear_pppixmap(dsp, gg);
}

void t1d_ppdraw_all(gint wid, gint hgt, gint margin, displayd *dsp, ggobid *gg)
{
  /*gint xpos, ypos, xstrt, ystrt;*/
  GdkPoint pptrace[100];
  gint i;

  t1d_clear_pppixmap(dsp, gg);

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

/* This is writes text to the pp window to in form the
user that optimize is finding a new maximum */ 
void t1d_ppdraw_think(displayd *dsp, ggobid *gg)
{
  splotd *sp = (splotd *) g_list_nth_data (dsp->splots, 0);
  colorschemed *scheme = gg->activeColorScheme;
  GtkStyle *style = gtk_widget_get_style (sp->da);
  gchar *varlab;
  gint lbearing, rbearing, width, ascent, descent;
  gint wid = dsp->t1d_ppda->allocation.width, 
    hgt = dsp->t1d_ppda->allocation.height;

  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
  varlab = g_strdup_printf("Thinking...");
  gdk_text_extents (
#if GTK_MAJOR_VERSION == 2
    gtk_style_get_font (style),
#else
    style->font,
#endif
    varlab, strlen (varlab),
    &lbearing, &rbearing, &width, &ascent, &descent);
    gdk_draw_string (dsp->t1d_pp_pixmap,
#if GTK_MAJOR_VERSION == 2
    gtk_style_get_font (style),
#else
      style->font,
#endif
      gg->plot_GC, 10, 10, varlab);
    g_free (varlab);
  gdk_draw_pixmap (dsp->t1d_ppda->window, gg->plot_GC, dsp->t1d_pp_pixmap,
    0, 0, 0, 0, wid, hgt);
}

/* This is the pp index plot drawing routine */ 
void t1d_ppdraw(gfloat pp_indx_val, displayd *dsp, ggobid *gg)
{
  colorschemed *scheme = gg->activeColorScheme;
  gint margin=10;
  gint wid = dsp->t1d_ppda->allocation.width, 
    hgt = dsp->t1d_ppda->allocation.height;
  gint j;
  static gboolean init = true;
  gchar *label = g_strdup("PP index: (0.0) 0.0000 (0.0)");

  if (init) {
    t1d_clear_ppda(dsp, gg);
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
 
    gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
    if (dsp->t1d_ppindx_count == 0) 
    {
      dsp->t1d_ppindx_count++;
    }
    else if (dsp->t1d_ppindx_count > 0 && dsp->t1d_ppindx_count < 80) {
      t1d_ppdraw_all(wid, hgt, margin, dsp, gg);
      dsp->t1d_ppindx_count++;
    }
    else if (dsp->t1d_ppindx_count >= 80) 
    {
      /* cycle values back into array */
      for (j=0; j<=dsp->t1d_ppindx_count; j++)
        dsp->t1d_ppindx_mat[j] = dsp->t1d_ppindx_mat[j+1];
      t1d_ppdraw_all(wid, hgt, margin, dsp, gg);
    }
  g_free (label);
}

void t1d_pp_reinit(displayd *dsp, ggobid *gg)
{
  gint i, j;
  gchar *label = g_strdup("PP index: (0.0) 0.0000 (0.0)");

  for (i=0; i<dsp->t1d_pp_op.proj_best.nrows; i++)
    for (j=0; j<dsp->t1d_pp_op.proj_best.ncols; j++)
      dsp->t1d_pp_op.proj_best.vals[i][j] = 0.;
  dsp->t1d.ppval = 0.0;
  dsp->t1d.oppval = -1.0;
  dsp->t1d_pp_op.index_best = 0.0;
  label = g_strdup_printf ("PP index: (%3.1f) %5.3f (%3.1f)",
  dsp->t1d_indx_min, dsp->t1d_ppindx_mat[dsp->t1d_ppindx_count], 
  dsp->t1d_indx_max);
  gtk_label_set_text(GTK_LABEL(dsp->t1d_pplabel),label);

  t1d_clear_ppda(dsp, gg);
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

gfloat t1d_calc_indx (array_f pd, 
                Tour_PPIndex_f index,
                void *param)
{ 
  gfloat indexval;

  index (&pd, param, &indexval);

  return(indexval);
}

gboolean t1d_switch_index(gint indxtype, gint basismeth, displayd *dsp,
  ggobid *gg)
{
  datad *d = dsp->d;
  gint kout, nrows = d->nrows_in_plot;
  gfloat *gdata;
  gint i, j;

  if (d->nrows_in_plot == 1)  /* can't do pp on no data! */
    return(false);

  for (i=0; i<d->nrows_in_plot; i++)
    for (j=0; j<dsp->t1d.nactive; j++)
      dsp->t1d_pp_op.data.vals[i][j] = 
        d->tform.vals[d->rows_in_plot.els[i]][dsp->t1d.active_vars.els[j]];

  for (j=0; j<dsp->t1d.nactive; j++)
    dsp->t1d_pp_op.proj_best.vals[0][j] = 
      dsp->t1d.F.vals[0][dsp->t1d.active_vars.els[j]];

  for (i=0; i<d->nrows_in_plot; i++) {
    dsp->t1d_pp_op.pdata.vals[i][0] = 
        (d->tform.vals[d->rows_in_plot.els[i]][dsp->t1d.active_vars.els[0]]*
        dsp->t1d.F.vals[0][dsp->t1d.active_vars.els[0]]);
    for (j=1; j<dsp->t1d.nactive; j++)
      dsp->t1d_pp_op.pdata.vals[i][0] += 
        (d->tform.vals[d->rows_in_plot.els[i]][dsp->t1d.active_vars.els[j]]*
        dsp->t1d.F.vals[0][dsp->t1d.active_vars.els[j]]);
  }

  gdata  = g_malloc (nrows*sizeof(gfloat));
  if (d->clusterid.els==NULL) printf ("No cluster information found\n");
  for (i=0; i<nrows; i++)
  { 
    if (d->clusterid.els!=NULL)
      gdata[i] = d->clusterid.els[d->rows_in_plot.els[i]];
    else
      gdata[i] = 0;
  }

  switch (indxtype)
  { 
    case HOLES: 
      dsp->t1d.ppval = t1d_calc_indx (dsp->t1d_pp_op.pdata, 
        holes_raw, &dsp->t1d_pp_param);
      if (basismeth == 1)
        kout = optimize0 (&dsp->t1d_pp_op, holes_raw, &dsp->t1d_pp_param);
      break;
    case CENTRAL_MASS: 
      dsp->t1d.ppval = t1d_calc_indx (dsp->t1d_pp_op.pdata, 
        central_mass_raw, &dsp->t1d_pp_param);
      if (basismeth == 1)
        kout = optimize0 (&dsp->t1d_pp_op, central_mass_raw, 
          &dsp->t1d_pp_param);
      break;
    case PCA: 
      dsp->t1d.ppval = t1d_calc_indx (dsp->t1d_pp_op.pdata, 
        pca, NULL);
      if (basismeth == 1)
        kout = optimize0 (&dsp->t1d_pp_op, pca, NULL);
      break;
    case LDA:
      if (!compute_groups (dsp->t1d_pp_param.group, dsp->t1d_pp_param.ngroup, 
          &dsp->t1d_pp_param.numgroups, nrows, gdata)) {
        dsp->t1d.ppval = t1d_calc_indx (dsp->t1d_pp_op.pdata, 
          discriminant, &dsp->t1d_pp_param);
        if (basismeth == 1)
          kout = optimize0 (&dsp->t1d_pp_op, discriminant, &dsp->t1d_pp_param);
      }
      break;
    case CGINI: 
      if (!compute_groups (dsp->t1d_pp_param.group, dsp->t1d_pp_param.ngroup, 
          &dsp->t1d_pp_param.numgroups, nrows, gdata)) {
        dsp->t1d.ppval = t1d_calc_indx (dsp->t1d_pp_op.pdata, 
          cartgini, &dsp->t1d_pp_param);
        if (basismeth == 1)
          kout = optimize0 (&dsp->t1d_pp_op, cartgini, &dsp->t1d_pp_param);
      }
      break;
   case CENTROPY: 
      if (!compute_groups (dsp->t1d_pp_param.group, dsp->t1d_pp_param.ngroup, 
          &dsp->t1d_pp_param.numgroups, nrows, gdata)) {
        dsp->t1d.ppval = t1d_calc_indx (dsp->t1d_pp_op.pdata,
          cartentropy, &dsp->t1d_pp_param);
      if (basismeth == 1)
        kout = optimize0 (&dsp->t1d_pp_op, cartentropy, &dsp->t1d_pp_param);
      }
      break;
      /*    case CART_VAR: 
      alloc_cartvariance_p (&cvp, nrows, gdata);
      dsp->t1d.ppval = t1d_calc_indx (d->tform, 
        dsp->t1d.F, d->rows_in_plot.els, d->nrows, d->ncols, 
        cartvariance, &cvp);
      if (basismeth == 1)
        kout = optimize0 (&dsp->t1d_pp_op, cartentropy, &cep);
      free_cartvariance_p (&cvp);
      break;
    case SUBD: 
      alloc_subd_p (&sp, nrows, pdim);
      dsp->t1d.ppval  = t1d_calc_indx (d->tform, dsp->t1d.F, 
        d->rows_in_plot.els, d->nrows, d->ncols, subd, &sp);
      if (basismeth == 1)
        kout  = optimize0 (&dsp->t1d_pp_op, subd, &sp);
      free_subd_p (&sp);
      break;*/
    default: 
      g_free (gdata);
      return(true);
      break;
  }
  g_free (gdata);
  return(false);
}

#undef SUBD           
#undef LDA            
#undef CGINI      
#undef CENTROPY   
#undef CART_VAR       
#undef PCA            
#undef HOLES
#undef CENTRAL_MASS
