/* tour2d_pp.c */
/* Copyright (C) 2001 Dianne Cook and Sigbert Klinke and Eun-Kuung Lee

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

#include "vars.h"
#include "externs.h"

#include "tour2d_pp.h"
#include "tour_pp.h"

#define HOLES 0
#define CENTRAL_MASS 1
#define SKEWNESS 2

#define EXPMINUS1 0.3678794411714423
#define ONEMINUSEXPMINUS1 0.63212056

/*****************************************************/
/* Reference : An Introduction to Numerical Analysis */
/*             - Kendall E. Atkinson                 */
/*             (p 449 - 450)                         */
/*****************************************************/

gfloat mean_fn2(gfloat *x1, gfloat *x2, gint n)
{
  gint i;
  gfloat tmean, tmpf1;
  gfloat mean1, mean2;

  tmpf1 = 0.;
  for (i=0; i<n; i++)
    tmpf1 += x1[i];
  mean1 = tmpf1 / (gfloat)n;
  tmpf1 = 0.;
  for (i=0; i<n; i++)
    tmpf1 += x2[i];
  mean2 = tmpf1 / (gfloat)n;
  tmean = 0.;
  for (i=0; i<n; i++) {
    tmean += ((x1[i]-mean1)*(x2[i]-mean2));
  }
  tmean /= ((gfloat)n);
  tmean += (mean1*mean2);

  return(tmean);
}

void
alloc_holes_p(holes_param *hp, gint nrows)
{
  hp->h0 = (gfloat *) g_malloc(
    (guint) nrows*sizeof(gfloat *));
  hp->h1 = (gfloat *) g_malloc(
    (guint) nrows*sizeof(gfloat *));
}

void
free_holes_p(holes_param *hp)
{
  g_free(hp->h0);
  g_free(hp->h1);
}

/********************************************************************
Index          : Holes
Transformation : needs sphered variables/principal components
Purpose        : computes a projection into a normal density fn
Note           : only works for 2d now, could be generalized
*********************************************************************/

gint holes(array_f *pdata, void *param, gfloat *val)
{
  holes_param *hp = (holes_param *) param;
  gint i, m;

  /* Calculate coefficients */
  for (i=0; i<pdata->nrows; i++)
  {
    m = i;
    hp->h0[m] = exp(-pdata->vals[m][0]*pdata->vals[m][0]/2.) ;
    hp->h1[m] = exp(-pdata->vals[m][1]*pdata->vals[m][1]/2.) ;
  }

  /* Calculate index */
  hp->acoefs = mean_fn2(hp->h0,hp->h1,pdata->nrows);
  *val = (1. - hp->acoefs)/(gfloat) ONEMINUSEXPMINUS1 ;
  return(0);
}

/********************************************************************

Index          : Central Mass
Transformation : needs sphered variables/principal components
Purpose        : computes a neg projection into a normal density fn
Note           : only works for 2d now, could be generalized

*********************************************************************/

gint
central_mass(array_f *pdata, void *param, gfloat *val)
{
  holes_param *hp = (holes_param *) param;
  gint i, m;

  /* Calculate coefficients */
  for (i=0; i<pdata->nrows; i++)
  {
    m = i;
    hp->h0[m] = exp(-pdata->vals[m][0]*pdata->vals[m][0]/2.) ;
    hp->h1[m] = exp(-pdata->vals[m][1]*pdata->vals[m][1]/2.) ;
  }

  /* Calculate index */
  hp->acoefs = mean_fn2(hp->h0,hp->h1,pdata->nrows);
  *val = (hp->acoefs - (gfloat)EXPMINUS1)/(float)ONEMINUSEXPMINUS1 ;
  return(0);
}

/***************************************************/
/*  2D Holes index for raw data                    */
/*   holes_raw1                                    */
/*   holes_raw2 : use inverse function             */
/***************************************************/

gint holes_raw1( array_f *pdata, void *param, gfloat *val)
{ 
/*
   holes_param *hp = (holes_param *) param;
*/
   gint i, p=pdata->ncols, n=pdata->nrows;
   gfloat m1, m2,x1,x2,temp;
   gdouble *cov;
   gfloat det,acoefs;

   cov = (gdouble *) g_malloc(p*p*sizeof(gdouble));
   for(i=0; i<(p*p); i++) cov[i] = 0;
   m1=0; m2=0;
   for(i=0; i<n; i++)
   { m1 += pdata->vals[i][0];
     m2 += pdata->vals[i][1];
   }      
   m1 /= n;
   m2 /= n;

   for(i=0; i<n; i++)
     {  cov[0] += (pdata->vals[i][0]-m1)*(pdata->vals[i][0]-m1)/(n-1);
        cov[1] += (pdata->vals[i][0]-m1)*(pdata->vals[i][1]-m2)/(n-1);
        cov[3] += (pdata->vals[i][1]-m2)*(pdata->vals[i][1]-m2)/(n-1);
     }

   cov[2]= cov[1];
   det = cov[0]*cov[3]-cov[1]*cov[1];
   acoefs=0.;

   for(i=0; i<n; i++)
     {  x1 = pdata->vals[i][0]-m1; 
        x2=pdata->vals[i][1]-m2;
        temp= (cov[3]*x1*x1-2*cov[1]*x1*x2+cov[0]*x2*x2)/det;
        acoefs +=exp(-temp/2);
   }

   *val = (1.-acoefs/n)/(gfloat) ONEMINUSEXPMINUS1;
   free(cov);
   return(0);
}

gint holes_raw2( array_f *pdata, void *param, gfloat *val)
{ 
   gint i, p=pdata->ncols, n=pdata->nrows;
   gfloat m1, m2,x1,x2,temp;
   gdouble *cov;
/*
   holes_param *hp = (holes_param *) param;
   gfloat det;
*/
   gfloat acoefs;

   cov = (gdouble *) g_malloc(p*p*sizeof(gdouble));
   for(i=0; i<(p*p); i++) cov[i] = 0;
   m1=0; m2=0;
   for(i=0; i<n; i++)
   { m1 += pdata->vals[i][0]/n;
     m2 += pdata->vals[i][1]/n;
   }      
   for(i=0; i<n; i++)
     {  cov[0] += (pdata->vals[i][0]-m1)*(pdata->vals[i][0]-m1)/(n-1);
        cov[1] += (pdata->vals[i][0]-m1)*(pdata->vals[i][1]-m2)/(n-1);
        cov[3] += (pdata->vals[i][1]-m2)*(pdata->vals[i][1]-m2)/(n-1);
     }
   cov[2]= cov[1];
   inverse(cov,p);
   acoefs=0.;

   for(i=0; i<n; i++)
     {  x1 = pdata->vals[i][0]-m1; 
        x2=pdata->vals[i][1]-m2;
        temp= cov[0]*x1*x1-(cov[1]+cov[2])*x1*x2+cov[3]*x2*x2;
        acoefs +=exp(-temp/2);
   }

   *val = (1.-acoefs/n)/(gfloat) ONEMINUSEXPMINUS1;
   free(cov);
   return(0);
}


/**********************************************************/
/*  2D Central Mass index for raw data                    */
/*   central_mass_raw1                                    */
/*   central_mass_raw2 : use inverse function             */
/**********************************************************/

gint central_mass_raw1(array_f *pdata, void *param, gfloat *val)
{
/*
   holes_param *hp = (holes_param *) param;
*/
   gint i, p=pdata->ncols, n=pdata->nrows;
   gfloat m1, m2,x1,x2,temp;
   gdouble *cov;
   gfloat det,acoefs;

   cov = (gdouble *) g_malloc(p*p*sizeof(gdouble));
   for(i=0; i<(p*p); i++) cov[i] = 0;
   m1=0; m2=0;
   for(i=0; i<n; i++)
   { m1 += pdata->vals[i][0];
     m2 += pdata->vals[i][1];
   }      
   m1 /= n;
   m2 /= n;

   for(i=0; i<n; i++)
     {  cov[0] += (pdata->vals[i][0]-m1)*(pdata->vals[i][0]-m1)/(n-1);
        cov[1] += (pdata->vals[i][0]-m1)*(pdata->vals[i][1]-m2)/(n-1);
        cov[3] += (pdata->vals[i][1]-m2)*(pdata->vals[i][1]-m2)/(n-1);
     }

   cov[2]= cov[1];
   det = cov[0]*cov[3]-cov[1]*cov[1];
   acoefs=0.;

   for(i=0; i<n; i++)
     {  x1 = pdata->vals[i][0]-m1; 
        x2=pdata->vals[i][1]-m2;
        temp= (cov[3]*x1*x1-2*cov[1]*x1*x2+cov[0]*x2*x2)/det;
        acoefs +=exp(-temp/2);
   }

   *val = (acoefs/n-(gfloat)EXPMINUS1)/(gfloat) ONEMINUSEXPMINUS1;
   free(cov);
   return(0);

}

gint central_mass_raw2(array_f *pdata, void *param, gfloat *val)
{
   /*holes_param *hp = (holes_param *) param;*/
   gint i, p=pdata->ncols, n=pdata->nrows;
   gfloat m1, m2,x1,x2,temp;
   gdouble *cov;
   gfloat acoefs;

   cov = (gdouble *) g_malloc(p*p*sizeof(gdouble));
   for(i=0; i<(p*p); i++) cov[i] = 0;
   m1=0; m2=0;
   for(i=0; i<n; i++)
   { m1 += pdata->vals[i][0]/n;
     m2 += pdata->vals[i][1]/n;
   }      
   for(i=0; i<n; i++)
     {  cov[0] += (pdata->vals[i][0]-m1)*(pdata->vals[i][0]-m1)/(n-1);
        cov[1] += (pdata->vals[i][0]-m1)*(pdata->vals[i][1]-m2)/(n-1);
        cov[3] += (pdata->vals[i][1]-m2)*(pdata->vals[i][1]-m2)/(n-1);
     }
   cov[2]= cov[1];
   inverse(cov,p);
   acoefs=0.;

   for(i=0; i<n; i++)
     {  x1 = pdata->vals[i][0]-m1; 
        x2=pdata->vals[i][1]-m2;
        temp= cov[0]*x1*x1-(cov[1]+cov[2])*x1*x2+cov[3]*x2*x2;
        acoefs +=exp(-temp/2);
   }
   *val = (acoefs/n-(gfloat)EXPMINUS1)/(gfloat) ONEMINUSEXPMINUS1;
   free(cov);
   return(0);

}

/*void
holes_deriv(holes_param *hp, gfloat **data, gfloat **pdata)
{
  gint i, k, m;
  gfloat tmpf;

  for (i=0; i<2; i++)
    for (k=0; k<hp->ncols; k++)
      hp->derivs[i][k] = 0.;

* alpha *
  for (k=0; k<hp->ncols; k++)
  {
    tmpf = 0.;
    for (i=0; i<hp->nrows; i++)
    {
      m = i;
      tmpf += (pdata[0][m]*hp->h0[m]*hp->h1[m]*
        (data[m][k] -
        hp->alpha[k]*pdata[0][m] -
        hp->beta[k]*pdata[1][m]));
    }
    tmpf /= ((float)hp->nrows);
    hp->derivs[0][k] = tmpf;
  }

* beta *
  for (k=0; k<hp->ncols; k++)
  {
    tmpf = 0.;
    for (i=0; i<hp->nrows; i++)
    {
      m = i;
      tmpf += (pdata[1][m]*hp->h0[m]*hp->h1[m]*
        (data[m][k] -
        hp->alpha[k]*pdata[0][m] -
        hp->beta[k]*pdata[1][m]));
    }
    tmpf /= ((float)hp->nrows);
    hp->derivs[1][k] = tmpf;
  }

}
*/


/*void
central_mass_deriv(float **data, float **proj_data, float *alpha, float *beta,
float **derivs, int n, int *rows_in_plot, int p, int nactive, int *active_vars)
{
  int i, k, m;
  float tmpf;

  for (i=0; i<2; i++)
    for (k=0; k<p; k++)
      derivs[i][k] = 0.;

  for (k=0; k<nactive; k++)
  {
    tmpf = 0.;
    for (i=0; i<n; i++)
    {
      m = rows_in_plot[i];
      tmpf -= (proj_data[0][m]*hp->h0[m]*hp->h1[m]*
        (data[m][active_vars[k]] -
        alpha[active_vars[k]]*proj_data[0][m] -
        beta[active_vars[k]]*proj_data[1][m]));
    }
    tmpf /= ((float)n);
    derivs[0][active_vars[k]] = tmpf;
  }

  for (k=0; k<nactive; k++)
  {
    tmpf = 0.;
    for (i=0; i<n; i++)
    {
      m = rows_in_plot[i];
      tmpf -= (proj_data[1][m]*hp->h0[m]*hp->h1[m]*
        (data[m][active_vars[k]] -
        alpha[active_vars[k]]*proj_data[0][m] -
        beta[active_vars[k]]*proj_data[1][m]));
    }
    tmpf /= ((float)n);
    derivs[1][active_vars[k]] = tmpf;
  }

}*/

/* This function interacts with control  buttons in ggobi */
void t2d_optimz(gint optimz_on, gboolean *nt, gint *bm, displayd *dsp) {
  gboolean new_target = *nt;
  gint bas_meth = *bm;
  gint i, j;

  if (optimz_on) 
  {
    for (i=0; i<2; i++)
      for (j=0; j<dsp->t2d.nactive; j++)
        dsp->t2d_pp_op.proj_best.vals[i][j] = 
          dsp->t2d.F.vals[i][dsp->t2d.active_vars.els[j]];
    dsp->t2d.ppval = dsp->t2d_indx_min;
    bas_meth = 1;
  }
  else
    bas_meth = 0;

  new_target = true;

  *nt = new_target;
  *bm = bas_meth;
}

void t2d_clear_pppixmap(ggobid *gg)
{
  displayd *dsp = gg->current_display;
  colorschemed *scheme = gg->activeColorScheme;
  gint margin=10;
  gint wid = dsp->t2d_ppda->allocation.width, 
    hgt = dsp->t2d_ppda->allocation.height;

  /* clear the pixmap */
  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_bg);
  gdk_draw_rectangle (dsp->t2d_pp_pixmap, gg->plot_GC,
                      true, 0, 0, wid, hgt);

  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
  gdk_draw_line (dsp->t2d_pp_pixmap, gg->plot_GC,
    margin, hgt - margin,
    wid - margin, hgt - margin);
  gdk_draw_line (dsp->t2d_pp_pixmap, gg->plot_GC,
    margin, hgt - margin, margin, margin);

  gdk_draw_pixmap (dsp->t2d_ppda->window, gg->plot_GC, dsp->t2d_pp_pixmap,
                   0, 0, 0, 0,
                   wid, hgt);
}

void t2d_clear_ppda(ggobid *gg)
{
  displayd *dsp = gg->current_display; 
  gint i;

  /* clear the ppindx matrix */
  dsp->t2d_ppindx_count = 0;
  dsp->t2d_indx_min=1000.;
  dsp->t2d_indx_max=-1000.;
  for (i=0; i<100; i++) 
  {
    dsp->t2d_ppindx_mat[i] = 0.0;
  }

  t2d_clear_pppixmap(gg);
}

void t2d_ppdraw_all(gint wid, gint hgt, gint margin, ggobid *gg)
{
  displayd *dsp = gg->current_display;
  GdkPoint pptrace[100];
  gint i;

  t2d_clear_pppixmap(gg);

  for (i=0; i<dsp->t2d_ppindx_count; i++) 
  {
    pptrace[i].x = margin+i*2;
    pptrace[i].y = hgt-margin-(gint)((gfloat)((dsp->t2d_ppindx_mat[i]-
      dsp->t2d_indx_min)/(gfloat) (dsp->t2d_indx_max-dsp->t2d_indx_min)) * 
      (gfloat) (hgt - 2*margin));
  }
  gdk_draw_lines (dsp->t2d_pp_pixmap, gg->plot_GC,
    pptrace, dsp->t2d_ppindx_count);

  gdk_draw_pixmap (dsp->t2d_ppda->window, gg->plot_GC, dsp->t2d_pp_pixmap,
    0, 0, 0, 0, wid, hgt);

}

/* This is the pp index plot drawing routine */ 
void t2d_ppdraw(gfloat pp_indx_val, ggobid *gg)
{
  displayd *dsp = gg->current_display;
  colorschemed *scheme = gg->activeColorScheme;
  gint margin=10;
  gint wid = dsp->t2d_ppda->allocation.width, 
    hgt = dsp->t2d_ppda->allocation.height;
  gint j;
  static gboolean init = true;
  gchar *label = g_strdup("PP index: (0.0) 0.0000 (0.0)");

  if (init) {
    t2d_clear_ppda(gg);
    init = false;
  }

  dsp->t2d_ppindx_mat[dsp->t2d_ppindx_count] = pp_indx_val;

  if (dsp->t2d_indx_min > pp_indx_val)
      dsp->t2d_indx_min = pp_indx_val;
  if (dsp->t2d_indx_max < pp_indx_val)
    dsp->t2d_indx_max = pp_indx_val;

  if (dsp->t2d_indx_min == dsp->t2d_indx_max) dsp->t2d_indx_min *= 0.9999;

  label = g_strdup_printf ("PP index: (%3.1f) %5.3f (%3.1f)",
    dsp->t2d_indx_min, dsp->t2d_ppindx_mat[dsp->t2d_ppindx_count], dsp->t2d_indx_max);
  gtk_label_set_text(GTK_LABEL(dsp->t2d_pplabel),label);

  gdk_gc_set_foreground (gg->plot_GC, &scheme->rgb_accent);
  if (dsp->t2d_ppindx_count == 0) 
  {
    dsp->t2d_ppindx_count++;
  }
  else if (dsp->t2d_ppindx_count > 0 && dsp->t2d_ppindx_count < 80) {
    t2d_ppdraw_all(wid, hgt, margin, gg);
    dsp->t2d_ppindx_count++;
  }
  else if (dsp->t2d_ppindx_count >= 80) 
  {
    /* cycle values back into array */
    for (j=0; j<=dsp->t2d_ppindx_count; j++)
      dsp->t2d_ppindx_mat[j] = dsp->t2d_ppindx_mat[j+1];
    t2d_ppdraw_all(wid, hgt, margin, gg);
  }

  g_free (label);
}

void t2d_pp_reinit(ggobid *gg)
{
  gint i, j;
  displayd *dsp = gg->current_display;

  for (i=0; i<dsp->t2d_pp_op.proj_best.nrows; i++)
    for (j=0; j<dsp->t2d_pp_op.proj_best.ncols; j++)
      dsp->t2d_pp_op.proj_best.vals[i][j] = 0.;
  dsp->t2d.ppval = -100.0;
  dsp->t2d.oppval = -999.0;
  dsp->t2d_pp_op.index_best = -100.0;

  t2d_clear_ppda(gg);
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


gfloat t2d_calc_indx (array_f data, array_d proj, 
                gint *rows, gint nrows, 
                gint ncols,
                gint (*index) (array_f*, void*, gfloat*),
                void *param)
{ 
  gfloat indexval;
  array_f pdata;
  gint i, j, m;
  /* gint k; */

  arrayf_init_null (&pdata);
  arrayf_alloc_zero (&pdata, nrows, 2);

  /* fill projected data array */
  for (m=0; m<nrows; m++)
  { 
    i = rows[m];
    pdata.vals[i][0] = 0.0;
    pdata.vals[i][1] = 0.0;
    for (j=0; j<ncols; j++)
    { 
      pdata.vals[i][0] += data.vals[i][j]*(gfloat)proj.vals[0][j];
      pdata.vals[i][1] += data.vals[i][j]*(gfloat)proj.vals[1][j];
    }
  }

  index (&pdata, param, &indexval);
  arrayf_free (&pdata, 0, 0);

  return(indexval);
}

gboolean t2d_switch_index(gint indxtype, gint basismeth, ggobid *gg)
{
  displayd *dsp = gg->current_display; 
  datad *d = dsp->d;
  holes_param hp;
  gint kout, nrows = d->nrows_in_plot;
  gint i, j;
  /* gint pdim = 2; */

  gtk_signal_connect (GTK_OBJECT(d), "rows_in_plot_changed",
    reset_pp, gg);

  if (d->nrows_in_plot == 1)  /* can't do pp on no data! */
    return(false);

  for (i=0; i<d->nrows_in_plot; i++)
    for (j=0; j<dsp->t2d.nactive; j++)
      dsp->t2d_pp_op.data.vals[i][j] = 
        d->tform.vals[d->rows_in_plot[i]][dsp->t2d.active_vars.els[j]];

  for (i=0; i<2; i++)
    for (j=0; j<dsp->t2d.nactive; j++)
      dsp->t2d_pp_op.proj_best.vals[i][j] = 
        dsp->t2d.F.vals[i][dsp->t2d.active_vars.els[j]];

  switch (indxtype)
  { 
    case HOLES: 
      alloc_holes_p (&hp, nrows);
      dsp->t2d.ppval = t2d_calc_indx (d->tform, 
        dsp->t2d.F, d->rows_in_plot, d->nrows, d->ncols, holes_raw1, &hp);
      if (basismeth == 1)
        kout = optimize0 (&dsp->t2d_pp_op, holes, &hp);
      free_holes_p(&hp);
    break;
    case CENTRAL_MASS: 
      alloc_holes_p (&hp, nrows);
      dsp->t2d.ppval = t2d_calc_indx (d->tform, 
        dsp->t2d.F, d->rows_in_plot, d->nrows, d->ncols, central_mass_raw1, &hp);
      if (basismeth == 1)
        kout = optimize0 (&dsp->t2d_pp_op, central_mass, &hp);
      free_holes_p(&hp);
    break;
    case SKEWNESS: 
      /*      alloc_cartgini_p (&cgp, nrows, gdata);
      dsp->t2d.ppval = t2d_calc_indx (d->tform, 
        dsp->t2d.F, d->rows_in_plot, d->nrows, d->ncols,
        cartgini, &cgp);
      if (basismeth == 1)
        kout = optimize0 (&dsp->t2d_pp_op, cartgini, &cgp);
      free_cartgini_p (&cgp);*/
    break;
    default: 
      return(true);
    break;
  }
  return(false);
}

#undef HOLES
#undef CENTRAL_MASS
#undef SKEWNESS

#undef ONEMINUSEXPMINUS1
#undef EXPMINUS1
