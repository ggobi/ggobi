/* tour2d_pp.c */
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

#include "tour2d_pp.h"
#include "tour_pp.h"

static gfloat ppindx_mat[100]; /* needs to be global for easy
				                  initialization/clearing */
static gint ppindx_count;
static gfloat indx_min, indx_max;

#define HOLES 0
#define CENTRAL_MASS 1
#define SKEWNESS 2

#define EXPMINUS1 0.3678794411714423
#define ONEMINUSEXPMINUS1 0.63212056

/*
static gfloat
mean_fn(gfloat *x1, gint n, gint *rows_in_plot)
{
  gint i;
  gfloat tmpf;
  gfloat mean1;

  tmpf = 0.;
  for (i=0; i<n; i++)
    tmpf += (x1[rows_in_plot[i]]-x1[0]);
  mean1 = tmpf / (gfloat)n;
  mean1 += x1[0];

  return(mean1);
}
*/

static gfloat
mean_fn2(gfloat *x1, gfloat *x2, gint n)
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

/********************************************************************

Index          : Holes
Transformation : needs sphered variables/principal components
Purpose        : computes a projection into a normal density fn
Note           : only works for 2d now, could be generalized

*********************************************************************/
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

void
holes_deriv(holes_param *hp, gfloat **data, gfloat **pdata)
{
  gint i, k, m;
  gfloat tmpf;

  for (i=0; i<2; i++)
    for (k=0; k<hp->ncols; k++)
      hp->derivs[i][k] = 0.;

/* alpha */
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

/* beta */
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

/********************************************************************

Index          : Central Mass
Transformation : needs sphered variables/principal components
Purpose        : computes a neg projection into a normal density fn
Note           : only works for 2d now, could be generalized

*********************************************************************/

void
alloc_central_mass(holes_param *hp, gint nrows)
{
  hp->h0 = (gfloat *) g_malloc((guint) nrows*sizeof(gfloat *));
  hp->h1 = (gfloat *) g_malloc((guint) nrows*sizeof(gfloat *));
}

void
free_central_mass(holes_param *hp)
{
  g_free(hp->h0);
  g_free(hp->h1);
}

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
  *val = (hp->acoefs - (float)EXPMINUS1)/(float)ONEMINUSEXPMINUS1 ;
  return(0);
}

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
      for (j=0; j<dsp->t2d.nvars; j++)
        dsp->t2d_pp_op.proj_best.vals[j][i] = 
          dsp->t2d.u.vals[i][dsp->t2d.vars.els[j]];
    dsp->t2d.ppval = indx_min;
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
  gint margin=10;
  gint wid = dsp->t2d_ppda->allocation.width, 
    hgt = dsp->t2d_ppda->allocation.height;

  /* clear the pixmap */
  gdk_gc_set_foreground (gg->plot_GC, &gg->bg_color);
  gdk_draw_rectangle (dsp->t2d_pp_pixmap, gg->plot_GC,
                      true, 0, 0, wid, hgt);

  gdk_gc_set_foreground (gg->plot_GC, &gg->accent_color);
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
  gint i;

  /* clear the ppindx matrix */
  ppindx_count = 0;
  indx_min=1000.;
  indx_max=-1000.;
  for (i=0; i<100; i++) 
  {
    ppindx_mat[i] = 0.0;
  }

  t2d_clear_pppixmap(gg);
}

void t2d_ppdraw_all(gint wid, gint hgt, gfloat indx_min, gfloat indx_max, 
  gint margin, ggobid *gg)
{
  displayd *dsp = gg->current_display;
  /*gint xpos, ypos, xstrt, ystrt;*/
  GdkPoint pptrace[100];
  gint i;

  t2d_clear_pppixmap(gg);

  for (i=0; i<ppindx_count; i++) 
  {
    pptrace[i].x = margin+i*2;
    pptrace[i].y = hgt-margin-(gint)((gfloat)((ppindx_mat[i]-indx_min)/
      (gfloat) (indx_max-indx_min)) * (gfloat) (hgt - 2*margin));
  }
  gdk_draw_lines (dsp->t2d_pp_pixmap, gg->plot_GC,
    pptrace, ppindx_count);

  gdk_draw_pixmap (dsp->t2d_ppda->window, gg->plot_GC, dsp->t2d_pp_pixmap,
    0, 0, 0, 0, wid, hgt);

}

/* This is the pp index plot drawing routine */ 
void t2d_ppdraw(gfloat pp_indx_val, ggobid *gg)
{
  displayd *dsp = gg->current_display;
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

  ppindx_mat[ppindx_count] = pp_indx_val;

  if (indx_min > pp_indx_val)
      indx_min = pp_indx_val;
  if (indx_max < pp_indx_val)
    indx_max = pp_indx_val;

  if (indx_min == indx_max) indx_min *= 0.9999;

  g_strdup_printf (label,"PP index: (%3.1f) %5.3f (%3.1f)",
    indx_min, ppindx_mat[ppindx_count], indx_max);
  gtk_label_set_text(GTK_LABEL(dsp->t2d_pplabel),label);

  gdk_gc_set_foreground (gg->plot_GC, &gg->accent_color);
  if (ppindx_count == 0) 
  {
    ppindx_count++;
  }
  else if (ppindx_count > 0 && ppindx_count < 80) {
    t2d_ppdraw_all(wid, hgt, indx_min, indx_max, margin, gg);
    ppindx_count++;
  }
  else if (ppindx_count >= 80) 
  {
    /* cycle values back into array */
    for (j=0; j<=ppindx_count; j++)
      ppindx_mat[j] = ppindx_mat[j+1];
    t2d_ppdraw_all(wid, hgt, indx_min, indx_max, margin, gg);
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


gfloat t2d_calc_indx (array_f data, array_f proj, 
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
      pdata.vals[i][0] += data.vals[i][j]*proj.vals[0][j];
      pdata.vals[i][1] += data.vals[i][j]*proj.vals[1][j];
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

  for (i=0; i<d->nrows_in_plot; i++)
    for (j=0; j<dsp->t2d.nvars; j++)
      dsp->t2d_pp_op.data.vals[i][j] = 
        d->tform.vals[d->rows_in_plot[i]][dsp->t2d.vars.els[j]];

  for (i=0; i<2; i++)
    for (j=0; j<dsp->t2d.nvars; j++)
      dsp->t2d_pp_op.proj_best.vals[j][i] = 
        dsp->t2d.u.vals[i][dsp->t2d.vars.els[j]];

  switch (indxtype)
  { 
    case HOLES: 
      alloc_holes_p (&hp, nrows);
      dsp->t2d.ppval = t2d_calc_indx (d->tform, 
        dsp->t2d.u, d->rows_in_plot, d->nrows, d->ncols, holes, &hp);
      if (basismeth == 1)
        kout = optimize0 (&dsp->t2d_pp_op, holes, &hp);
      free_holes_p(&hp);
      break;
    case CENTRAL_MASS: 
      alloc_central_mass (&hp, nrows);
      dsp->t2d.ppval = t2d_calc_indx (d->tform, 
        dsp->t2d.u, d->rows_in_plot, d->nrows, d->ncols, central_mass, &hp);
      if (basismeth == 1)
        kout = optimize0 (&dsp->t2d_pp_op, central_mass, &hp);
      free_central_mass(&hp);
      break;
    case SKEWNESS: 
      /*      alloc_cartgini_p (&cgp, nrows, gdata);
      dsp->t2d.ppval = t2d_calc_indx (d->tform, 
        dsp->t2d.u, d->rows_in_plot, d->nrows, d->ncols,
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
