/* tour1d.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/

#include <gtk/gtk.h>
#ifdef USE_STRINGS_H
#include <strings.h>
#endif

#include <math.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef WIN32 
#include <windows.h>
#endif

#include "vars.h"
#include "externs.h"

#include "tour1d_pp.h"

#define T1DON true
#define T1DOFF false

void
alloc_tour1d (displayd *dsp, ggobid *gg)
{
  datad *d = dsp->d;
  gint nc = d->ncols;

  arrayf_init_null(&dsp->t1d.u0);
  arrayf_alloc(&dsp->t1d.u0, 2, nc);

  arrayf_init_null(&dsp->t1d.u1);
  arrayf_alloc(&dsp->t1d.u1, 2, nc);

  arrayf_init_null(&dsp->t1d.u);
  arrayf_alloc(&dsp->t1d.u, 2, nc);

  /*  arrayf_init_null(&dsp->t1d.uold);
  arrayf_alloc(&dsp->t1d.uold, nc, nc);*/

  arrayf_init_null(&dsp->t1d.v0);
  arrayf_alloc(&dsp->t1d.v0, 2, nc);

  arrayf_init_null(&dsp->t1d.v1);
  arrayf_alloc(&dsp->t1d.v1, 2, nc);

  arrayf_init_null(&dsp->t1d.v);
  arrayf_alloc(&dsp->t1d.v, 2, nc);

  arrayf_init_null(&dsp->t1d.uvevec);
  arrayf_alloc(&dsp->t1d.uvevec, 2, nc);

  arrayf_init_null(&dsp->t1d.tv);
  arrayf_alloc(&dsp->t1d.tv, 2, nc);

  vectori_init_null(&dsp->t1d.vars);
  vectori_alloc(&dsp->t1d.vars, nc);
  vectorf_init_null(&dsp->t1d.lambda);
  vectorf_alloc(&dsp->t1d.lambda, nc);
  vectorf_init_null(&dsp->t1d.tau);
  vectorf_alloc(&dsp->t1d.tau, nc);
  vectorf_init_null(&dsp->t1d.tinc);
  vectorf_alloc(&dsp->t1d.tinc, nc);

  /* manipulation controls */
  arrayf_init_null(&dsp->t1d_manbasis);
  arrayf_alloc(&dsp->t1d_manbasis, 2, nc);
}

/*-- eliminate the nc columns contained in *cols --*/
void
tour1d_realloc_down (gint nc, gint *cols, datad *d, ggobid *gg)
{
  displayd *dsp;
  GList *l;
  for (l=gg->displays; l; l=l->next) {
    dsp = (displayd *) l->data;
    if (dsp->d == d) {
      arrayf_delete_cols (&dsp->t1d.u0, nc, cols);
      arrayf_delete_cols (&dsp->t1d.u1, nc, cols);
      arrayf_delete_cols (&dsp->t1d.u, nc, cols);
      arrayf_delete_cols (&dsp->t1d.v0, nc, cols);
      arrayf_delete_cols (&dsp->t1d.v1, nc, cols);
      arrayf_delete_cols (&dsp->t1d.v, nc, cols);
      arrayf_delete_cols (&dsp->t1d.uvevec, nc, cols);
      arrayf_delete_cols (&dsp->t1d.tv, nc, cols);

      vectori_delete_els (&dsp->t1d.vars, nc, cols);
      vectorf_delete_els (&dsp->t1d.lambda, nc, cols);
      vectorf_delete_els (&dsp->t1d.tau, nc, cols);
      vectorf_delete_els (&dsp->t1d.tinc, nc, cols);

      arrayf_delete_cols (&dsp->t1d_manbasis, (gint) nc, cols);
    }
  }
}

/*-- append columns for a total of nc columns --*/
void
tour1d_realloc_up (gint nc, datad *d, ggobid *gg)
{
  displayd *dsp;
  GList *l;
  gint old_ncols, i;

  for (l=gg->displays; l; l=l->next) {
    dsp = (displayd *) l->data;
    if (dsp->displaytype != scatterplot)
      continue;

    old_ncols = dsp->t1d.u0.ncols;

    if (old_ncols < 1 && nc >= 1) {
      display_tour1d_init(dsp, gg);
    }

    if (dsp->d == d) {
      arrayf_add_cols (&dsp->t1d.u0, nc);
      arrayf_add_cols (&dsp->t1d.u1, nc);
      arrayf_add_cols (&dsp->t1d.u, nc);
      arrayf_add_cols (&dsp->t1d.v0, nc);
      arrayf_add_cols (&dsp->t1d.v1, nc);
      arrayf_add_cols (&dsp->t1d.v, nc);
      arrayf_add_cols (&dsp->t1d.uvevec, nc);
      arrayf_add_cols (&dsp->t1d.tv, nc);

      vectori_realloc (&dsp->t1d.vars, nc);
      vectorf_realloc (&dsp->t1d.lambda, nc);
      vectorf_realloc (&dsp->t1d.tau, nc);
      vectorf_realloc (&dsp->t1d.tinc, nc);

      arrayf_add_cols (&dsp->t1d_manbasis, (gint) nc);

      /* need to zero extra cols */
      for (i=old_ncols; i<nc; i++) {
        dsp->t1d.u0.vals[0][i] = dsp->t1d.u0.vals[1][i] = 0.0;
        dsp->t1d.u1.vals[0][i] = dsp->t1d.u1.vals[1][i] = 0.0;
        dsp->t1d.u.vals[0][i] = dsp->t1d.u.vals[1][i] = 0.0;
        dsp->t1d.v0.vals[0][i] = dsp->t1d.v0.vals[1][i] = 0.0;
        dsp->t1d.v1.vals[0][i] = dsp->t1d.v1.vals[1][i] = 0.0;
        dsp->t1d.v.vals[0][i] = dsp->t1d.v.vals[1][i] = 0.0;
        dsp->t1d.uvevec.vals[0][i] = dsp->t1d.uvevec.vals[1][i] = 0.0;
        dsp->t1d.tv.vals[0][i] = dsp->t1d.tv.vals[1][i] = 0.0;
        dsp->t1d.vars.els[i] = 0;
        dsp->t1d.lambda.els[i] = 0.0;
        dsp->t1d.tau.els[i] = 0.0;
        dsp->t1d.tinc.els[i] = 0.0;
      }
    }
  }
}

void
free_tour1d(displayd *dsp)
{
  vectori_free(&dsp->t1d.vars);
  vectorf_free(&dsp->t1d.lambda);
  vectorf_free(&dsp->t1d.tau);
  vectorf_free(&dsp->t1d.tinc);

  arrayf_free(&dsp->t1d.u0, 0, 0);
  arrayf_free(&dsp->t1d.u1, 0, 0);
  arrayf_free(&dsp->t1d.u, 0, 0);

  arrayf_free(&dsp->t1d.v0, 0, 0);
  arrayf_free(&dsp->t1d.v1, 0, 0);
  arrayf_free(&dsp->t1d.v, 0, 0);

  arrayf_free(&dsp->t1d.uvevec, 0, 0);
  arrayf_free(&dsp->t1d.tv, 0, 0);

  arrayf_free(&dsp->t1d_manbasis, 0, 0);
}

void 
display_tour1d_init (displayd *dsp, ggobid *gg) {
  gint i, j;
  datad *d = dsp->d;
  cpaneld *cpanel = &dsp->cpanel;
  gint nc = d->ncols;

  alloc_tour1d(dsp, gg);
 
    /* Initialize starting subset of active variables */
  if (nc < 8) {
    dsp->t1d.nvars = nc;
    for (j=0; j<nc; j++)
      dsp->t1d.vars.els[j] = j;
  }
  else {
    dsp->t1d.nvars = 3;
    dsp->t1d.vars.els[0] = 0;
    dsp->t1d.vars.els[1] = 1;
    dsp->t1d.vars.els[2] = 2;
    for (j=3; j<nc; j++)
      dsp->t1d.vars.els[j] = 0;

  }

  /* declare starting base as first p chosen variables */
  for (i=0; i<2; i++)
    for (j=0; j<nc; j++)
      dsp->t1d.u0.vals[i][j] = dsp->t1d.u1.vals[i][j] = dsp->t1d.u.vals[i][j] = 
        dsp->t1d.v0.vals[i][j] = dsp->t1d.v1.vals[i][j] = 0.0;

  for (i=0; i<2; i++)
  {
    dsp->t1d.u1.vals[i][dsp->t1d.vars.els[i]] =
      dsp->t1d.u0.vals[i][dsp->t1d.vars.els[i]] = 
      dsp->t1d.u.vals[i][dsp->t1d.vars.els[i]] =
      dsp->t1d.v0.vals[i][dsp->t1d.vars.els[i]] = 
      dsp->t1d.v1.vals[i][dsp->t1d.vars.els[i]] = 1.0;
  }
  /*  zero_tau(dsp, gg);*/

  dsp->t1d.dv = 1.0;
  dsp->t1d.delta = cpanel->t1d_step*M_PI_2/10.0;
  dsp->t1d.nsteps = 1; 
  dsp->t1d.stepcntr = 1;

  dsp->t1d.idled = 0;
  dsp->t1d.get_new_target = true;

  /* manip */
  dsp->t1d_manip_var = 0;

  /* pp */
  dsp->t1d.target_basis_method = 0;
  dsp->t1d_ppda = NULL;
  dsp->t1d_axes = true;
}

/*-- called from the Options menu --*/
void
tour1d_fade_vars_cb (GtkCheckMenuItem *w, guint action) 
{
  ggobid *gg = GGobiFromWidget(GTK_WIDGET(w), true);

  gg->tour1d.fade_vars = !gg->tour1d.fade_vars;
}

void tour1d_speed_set(gint slidepos, ggobid *gg) {
  displayd *dsp = gg->current_display; 
  cpaneld *cpanel = &dsp->cpanel;
  extern void speed_set (gint, gfloat *, gfloat *, gfloat, gint *, gint *);

  cpanel->t1d_slidepos = slidepos;
  speed_set(slidepos, &cpanel->t1d_step, &dsp->t1d.delta,  dsp->t1d.dv,
    &dsp->t1d.nsteps, &dsp->t1d.stepcntr);
}

void tour1d_pause (cpaneld *cpanel, gboolean state, ggobid *gg) {
  cpanel->t1d_paused = state;

  tour1d_func (!cpanel->t1d_paused, gg->current_display, gg);

  if (cpanel->t1d_paused) {
    /*-- whenever motion stops, we need a FULL redraw --*/
    display_tailpipe (gg->current_display, FULL, gg);
  }
}

void 
tour1dvar_set (gint jvar, ggobid *gg)
{
  gint j, k;
  gboolean active=false;
  displayd *dsp = gg->current_display;
  datad *d = dsp->d;

  for (j=0; j<dsp->t1d.nvars; j++)
    if (jvar == dsp->t1d.vars.els[j])
      active = true;

  /* deselect var if t1d.nvars > 2 */
  if (active) {
    if (dsp->t1d.nvars > 1) {
      for (j=0; j<dsp->t1d.nvars; j++) {
        if (jvar == dsp->t1d.vars.els[j]) 
          break;
      }
      if (j<dsp->t1d.nvars-1) {
        for (k=j; k<dsp->t1d.nvars-1; k++){
          dsp->t1d.vars.els[k] = dsp->t1d.vars.els[k+1];
        }
      }
      dsp->t1d.nvars--;

      if (!gg->tour1d.fade_vars) /* set current position without sel var */
      {
        gt_basis(dsp->t1d.u0, dsp->t1d.nvars, dsp->t1d.vars, 
          d->ncols, (gint) 1);
        copy_mat(dsp->t1d.u.vals, dsp->t1d.u0.vals, d->ncols, 1);
      }
    }
  }
  else { /* not active, so add the variable */
    if (jvar > dsp->t1d.vars.els[dsp->t1d.nvars-1]) {
      dsp->t1d.vars.els[dsp->t1d.nvars] = jvar;
    }
    else if (jvar < dsp->t1d.vars.els[0]) {
      for (j=dsp->t1d.nvars; j>0; j--) {
          dsp->t1d.vars.els[j] = dsp->t1d.vars.els[j-1];
      }
      dsp->t1d.vars.els[0] = jvar;
    }
    else {
      gint jtmp = dsp->t1d.nvars;
      for (j=0; j<dsp->t1d.nvars-1; j++) {
        if (jvar > dsp->t1d.vars.els[j] && jvar < dsp->t1d.vars.els[j+1]) {
          jtmp = j+1;
          break;
        }
      }
      for (j=dsp->t1d.nvars-1;j>=jtmp; j--) 
          dsp->t1d.vars.els[j+1] = dsp->t1d.vars.els[j];
      dsp->t1d.vars.els[jtmp] = jvar;
    }
    dsp->t1d.nvars++;
  }

  dsp->t1d.get_new_target = true;
}

static void
tour1d_manip_var_set (gint j, gint btn, ggobid *gg)
{
  displayd *dsp = gg->current_display;

  if (btn == 1)
    dsp->t1d_manip_var = j;    
}

void
tour1d_varsel (gint jvar, gint button, datad *d, ggobid *gg)
{
/*-- any button --*/
  if (d->vcirc_ui.jcursor == GDK_HAND2) {
    tour1d_manip_var_set (jvar, button, gg);
    varcircles_cursor_set_default (d);

  } else {
    tour1dvar_set (jvar, gg);
  }
}

void
tour1d_projdata(splotd *sp, glong **world_data, datad *d, ggobid *gg)
{
  gint i, j, m;
  displayd *dsp = (displayd *) sp->displayptr;
  gfloat min, max, mean;
  gfloat precis = PRECISION1;
  cpaneld *cpanel = &dsp->cpanel;
  gfloat *yy;

  if (sp == NULL)
    return;

  yy = (gfloat *) g_malloc (d->nrows_in_plot * sizeof (gfloat));

  for (m=0; m < d->nrows_in_plot; m++)
  {
    i = d->rows_in_plot[m];
    yy[i] = sp->planar[i].x = 0;
    sp->planar[i].y = 0;
    for (j=0; j<d->ncols; j++)
    {
      yy[i] += (gint)(dsp->t1d.u.vals[0][j]*world_data[i][j]);
    }
  }
  do_ash1d (yy, d->nrows_in_plot,
            cpanel->t1d_nbins, cpanel->t1d_nASHes,
            sp->p1d_data.els, &min, &max, &mean);
  if (sp->tour1d.firsttime) {
    sp->tour1d.keepmin = min;
    sp->tour1d.keepmax = max;
    sp->tour1d.firsttime = false;
  }
  else {
    if (min < sp->tour1d.keepmin) sp->tour1d.keepmin = min;
    if (max > sp->tour1d.keepmax) sp->tour1d.keepmax = max;
  }

  max = 2*mean;  /* try letting the max for scaling depend on the mean */
  if (cpanel->t1d_vert) {
    for (i=0; i<d->nrows_in_plot; i++) {
      sp->planar[i].x = (glong) (precis*(-1.0+2.0*
        (sp->p1d_data.els[i]-sp->tour1d.keepmin)/(max-sp->tour1d.keepmin)));
      sp->planar[i].y = yy[i];
    }
  }
  else {
    for (i=0; i<d->nrows_in_plot; i++) {
      sp->planar[i].x = yy[i];
      sp->planar[i].y = (glong) (precis*(-1.0+2.0*
        (sp->p1d_data.els[i]-sp->tour1d.keepmin)/(max-sp->tour1d.keepmin)));
    }
  }

  g_free ((gpointer) yy);
}

void
tour1d_run(displayd *dsp, ggobid *gg)
{
  extern gboolean reached_target(gint, gint, gint, gfloat *, gfloat *);
  extern void increment_tour(vector_f, vector_f, gint *, gint *, gfloat, 
    gfloat, gint);
  extern void do_last_increment(vector_f, vector_f, gint);
  extern gint path(array_f, array_f, array_f, gint, gint, array_f, 
    array_f, array_f, vector_f, array_f, array_f,
    vector_f, vector_f, gint *, gint *, gfloat *, gfloat);
  extern void tour_reproject(vector_f, array_f, array_f, array_f, 
    array_f, array_f, gint, gint);
  extern void t1d_ppdraw(gfloat, ggobid *);
  datad *d = dsp->d;
  cpaneld *cpanel = &dsp->cpanel;
  static gint count = 0;
  gboolean revert_random = false;
  static gfloat oindxval = -999.0;
  gint pathprob = 0;

  gint i, j, nv;

  if (!dsp->t1d.get_new_target && 
      !reached_target(dsp->t1d.nsteps, dsp->t1d.stepcntr, 
        dsp->t1d.target_basis_method,&dsp->t1d.ppval, &oindxval)) {
    increment_tour(dsp->t1d.tinc, dsp->t1d.tau, &dsp->t1d.nsteps, 
      &dsp->t1d.stepcntr, dsp->t1d.dv, dsp->t1d.delta, (gint) 1);
    tour_reproject(dsp->t1d.tinc, dsp->t1d.v, dsp->t1d.v0, dsp->t1d.v1,
      dsp->t1d.u, dsp->t1d.uvevec, d->ncols, (gint) 1);

    /* plot pp indx */
    if (dsp->t1d_ppda != NULL) {

      revert_random = t1d_switch_index(cpanel->t1d_pp_indx, 
        0, gg);
      count++;
      if (count == 10) {
        count = 0;
        t1d_ppdraw(dsp->t1d.ppval, gg);
      }
    }

  }
  else { /* do final clean-up and get new target */
    if (dsp->t1d.get_new_target) {
      if (dsp->t1d.target_basis_method == 1)
      {
        dsp->t1d_pp_op.index_best = dsp->t1d.ppval;
        oindxval = dsp->t1d.ppval;
        for (j=0; j<dsp->t1d.nvars; j++)
          dsp->t1d_pp_op.proj_best.vals[j][0] = 
            dsp->t1d.u.vals[0][dsp->t1d.vars.els[j]];
      }
    }
    else 
    {
      if (dsp->t1d.target_basis_method == 1)
        t1d_ppdraw(dsp->t1d.ppval, gg);
      else
      {
        do_last_increment(dsp->t1d.tinc, dsp->t1d.tau, (gint) 1);
        tour_reproject(dsp->t1d.tinc, dsp->t1d.v, dsp->t1d.v0, dsp->t1d.v1,
          dsp->t1d.u, dsp->t1d.uvevec, d->ncols, (gint) 1);
      }
    }
    copy_mat(dsp->t1d.u0.vals, dsp->t1d.u.vals, d->ncols, 1);
    nv = 0;
    for (i=0; i<d->ncols; i++)
      if (fabs(dsp->t1d.u0.vals[0][i]) > 0.01) {
        nv++;
      }
    if (nv == 1 && dsp->t1d.nvars == 1) /* only generate new dir if num of
                                           active/used variables is > 2 */
      dsp->t1d.get_new_target = true;
    else {
      if (dsp->t1d.target_basis_method == 0) {
        gt_basis(dsp->t1d.u1, dsp->t1d.nvars, dsp->t1d.vars, d->ncols, (gint) 1);
      }
      else if (dsp->t1d.target_basis_method == 1) {
        /* pp guided tour  */
        revert_random = t1d_switch_index(cpanel->t1d_pp_indx, 
          dsp->t1d.target_basis_method, gg);

        if (!revert_random) {
          for (i=0; i<dsp->t1d.nvars; i++)
            dsp->t1d.u1.vals[0][dsp->t1d.vars.els[i]] = 
              dsp->t1d_pp_op.proj_best.vals[i][0];

          /* if the best projection is the same as the previous one, switch 
              to a random projection */
          if (!checkequiv(dsp->t1d.u0.vals, dsp->t1d.u1.vals, d->ncols, 1)) {
            g_printerr ("Using random projection\n");
            gt_basis(dsp->t1d.u1, dsp->t1d.nvars, dsp->t1d.vars, 
              d->ncols, (gint) 1);
            for (j=0; j<dsp->t1d.nvars; j++)
              dsp->t1d_pp_op.proj_best.vals[j][0] = 
                dsp->t1d.u1.vals[0][dsp->t1d.vars.els[j]];
              /*              dsp->t1d.ppval = -999.0;*/
            revert_random = t1d_switch_index(cpanel->t1d_pp_indx, 
              dsp->t1d.target_basis_method, gg);
          }
          t1d_ppdraw(dsp->t1d.ppval, gg);
          count = 0;
#ifndef WIN32
          sleep(2);
#else
          Sleep(2);
#endif
        }
        else
        {
          gt_basis(dsp->t1d.u1, dsp->t1d.nvars, dsp->t1d.vars, 
            d->ncols, (gint) 1);
        }
        
      }
      pathprob = path(dsp->t1d.u0, dsp->t1d.u1, dsp->t1d.u, d->ncols, 
        (gint) 1, dsp->t1d.v0,
        dsp->t1d.v1, dsp->t1d.v, dsp->t1d.lambda, dsp->t1d.tv, dsp->t1d.uvevec,
        dsp->t1d.tau, dsp->t1d.tinc, &dsp->t1d.nsteps, &dsp->t1d.stepcntr, 
        &dsp->t1d.dv, dsp->t1d.delta);
      dsp->t1d.get_new_target = false;
    }
  }
  /*  tour_reproject(dsp, 2);*/
  display_tailpipe (dsp, FULL, gg);

  varcircles_refresh (d, gg);

}

void
tour1d_do_step(displayd *dsp, ggobid *gg)
{
  tour1d_run(dsp, gg);
}

gint
tour1d_idle_func (displayd *dsp)
{
  ggobid *gg = GGobiFromDisplay (dsp);
  cpaneld *cpanel = &dsp->cpanel;
  gboolean doit = !cpanel->t1d_paused;

  if (doit) {
    tour1d_run (dsp, gg);
    gdk_flush ();
  }

  return (doit);
}

void tour1d_func (gboolean state, displayd *dsp, ggobid *gg)
{
  if (state) {
    if (dsp->t2d.idled == 0) {
      dsp->t1d.idled = gtk_idle_add_priority (G_PRIORITY_LOW,
                                     (GtkFunction) tour1d_idle_func, dsp);
    }
    gg->tour1d.idled = 1;
  } else {
    if (dsp->t1d.idled) {
      gtk_idle_remove (dsp->t1d.idled);
      dsp->t1d.idled = 0;
    }
    gg->tour1d.idled = 0;
  }

/*
   if (state)
     tour_idle = gtk_timeout_add (40, tour_idle_func, NULL);
   else
     gtk_timeout_remove (tour_idle);
*/
}

void tour1d_reinit(ggobid *gg)
{
  gint i, j;
  displayd *dsp = gg->current_display;
  datad *d = dsp->d;

  for (i=0; i<1; i++) {
    for (j=0; j<d->ncols; j++) {
      dsp->t1d.u0.vals[i][j] = 0.;
      dsp->t1d.u.vals[i][j] = 0.;
    }
    dsp->t1d.u0.vals[i][dsp->t1d.vars.els[i]] = 1.;
    dsp->t1d.u.vals[i][dsp->t1d.vars.els[i]] = 1.;
  }

  dsp->t1d.get_new_target = true;

  display_tailpipe (dsp, FULL, gg);

  varcircles_refresh (d, gg);
}

void tour1d_scramble(ggobid *gg)
{
  int i, j;
  displayd *dsp = gg->current_display;
  datad *d = dsp->d;
  gint nc = d->ncols;

  for (i=0; i<1; i++)
    for (j=0; j<nc; j++)
      dsp->t1d.u0.vals[i][j] = dsp->t1d.u1.vals[i][j] = 
        dsp->t1d.u.vals[i][j] = dsp->t1d.v0.vals[i][j] = 
        dsp->t1d.v1.vals[i][j] = 0.0;

  gt_basis(dsp->t1d.u0, dsp->t1d.nvars, dsp->t1d.vars, 
    d->ncols, (gint) 1);
  copy_mat(dsp->t1d.u.vals, dsp->t1d.u0.vals, d->ncols, 1);

  dsp->t1d.nsteps = 1; 
  dsp->t1d.stepcntr = 1;

  dsp->t1d.get_new_target = true;

  display_tailpipe (dsp, FULL, gg);

  varcircles_refresh (d, gg);
}

void tour1d_vert(cpaneld *cpanel, gboolean state)
{
  cpanel->t1d_vert = state;
}

/* Variable manipulation */
void
tour1d_manip_init(gint p1, gint p2, splotd *sp) 
{
  displayd *dsp = (displayd *) sp->displayptr;
  datad *d = dsp->d;
  cpaneld *cpanel = &dsp->cpanel;
  ggobid *gg = GGobiFromSPlot(sp);
  gint j;
  gint n1vars = dsp->t1d.nvars;
  gfloat ftmp, tol = 0.01; 
  gboolean dontdoit = false;
  extern void gram_schmidt(gfloat *, gfloat*, gint);

  dsp->t1d_phi = 0.;

  /* gets mouse position */
  if (cpanel->t1d_vert) 
    dsp->t1d_pos = dsp->t1d_pos_old = p2;
  else
    dsp->t1d_pos = dsp->t1d_pos_old = p1;

  /* initializes indicator for manip var being one of existing vars */
  dsp->t1d_manipvar_inc = false;

  /* need to turn off tour */
  if (!cpanel->t1d_paused)
    tour1d_func(T1DOFF, gg->current_display, gg);

  /* check if manip var is one of existing vars */
  /* n1vars, n2vars is the number of variables, excluding the
     manip var in hor and vert directions */
  for (j=0; j<dsp->t1d.nvars; j++)
    if (dsp->t1d.vars.els[j] == dsp->t1d_manip_var) {
      dsp->t1d_manipvar_inc = true;
      n1vars--;
    }

  /* make manip basis, from existing projection */
  /* 0 will be the remainder of the projection, and
     1 will be the indicator vector for the manip var */
  for (j=0; j<d->ncols; j++) {
    dsp->t1d_manbasis.vals[0][j] = dsp->t1d.u.vals[0][j];
    dsp->t1d_manbasis.vals[1][j] = 0.;
  }
  dsp->t1d_manbasis.vals[1][dsp->t1d_manip_var]=1.;

  if (n1vars > 0)
  {
    gram_schmidt(dsp->t1d_manbasis.vals[0],  dsp->t1d_manbasis.vals[1],
      d->ncols);
    ftmp = calc_norm (dsp->t1d_manbasis.vals[1], d->ncols);
    if (ftmp < tol)
      dontdoit = true;
  }

  if (dontdoit)
    disconnect_motion_signal (sp);
}

void
tour1d_manip(gint p1, gint p2, splotd *sp, ggobid *gg) 
{
  displayd *dsp = (displayd *) sp->displayptr;
  datad *d = dsp->d;
  cpaneld *cpanel = &dsp->cpanel;
  gfloat xcosphi=1., xsinphi=0.;
  gfloat distx, disty;
  gfloat denom = (float) MIN(sp->max.x, sp->max.y)/2.;
  gint actual_nxvars = dsp->t1d.nvars;
  gint j;
  gboolean offscreen = false;

  /* check if off the plot window */
  if (p1 > sp->max.x || p1 < 0 ||
      p2 > sp->max.y || p2 <0)
    offscreen = true;

  if (dsp->t1d_manipvar_inc)
    actual_nxvars = dsp->t1d.nvars-1;

  if (!offscreen) {
    dsp->t1d_pos_old = dsp->t1d_pos;
  
    dsp->t1d_pos = p1;

    if (actual_nxvars > 0)
    {
      if (cpanel->t1d_vert)
      {
        distx = 0.;
        disty = dsp->tc2_pos_old - dsp->tc2_pos;
      }
      else
      {
        distx = dsp->t1d_pos - dsp->t1d_pos_old;
        disty = 0.;
      }

      dsp->t1d_phi = dsp->t1d_phi + distx / denom;
  
      xcosphi = (gfloat) cos((gdouble) dsp->t1d_phi);
      xsinphi = (gfloat) sin((gdouble) dsp->t1d_phi);
      if (xcosphi > 1.0)
      {
        xcosphi = 1.0;
        xsinphi = 0.0;
      }
      else if (xcosphi < -1.0)
      {
        xcosphi = -1.0;
        xsinphi = 0.0;
      }
    }

    /* generate the projection basis */
    if (actual_nxvars > 0) 
    {
      for (j=0; j<d->ncols; j++)
        dsp->t1d.u.vals[0][j] = xcosphi * dsp->t1d_manbasis.vals[0][j] + 
         xsinphi * dsp->t1d_manbasis.vals[1][j];
    }
 
    display_tailpipe (dsp, FULL, gg);
    varcircles_refresh (d, gg);
  }
  else {
    disconnect_motion_signal (sp);
    copy_mat(dsp->t1d.u0.vals, dsp->t1d.u.vals, d->ncols, 1);
    dsp->t1d.get_new_target = true;
    if (!cpanel->t1d_paused)
      tour1d_func(T1DON, gg->current_display, gg);
  }
}

void
tour1d_manip_end(splotd *sp) 
{
  displayd *dsp = (displayd *) sp->displayptr;
  datad *d = dsp->d;
  cpaneld *cpanel = &dsp->cpanel;
  ggobid *gg = GGobiFromSPlot(sp);
  extern void copy_mat(gfloat **, gfloat **, gint, gint);

  disconnect_motion_signal (sp);

  copy_mat(dsp->t1d.u0.vals, dsp->t1d.u.vals, d->ncols, 1);
  dsp->t1d.get_new_target = true;

  /* need to turn on tour? */
  if (!cpanel->t1d_paused) {
    tour1d_pause(cpanel, T1DOFF, gg);

    /*-- whenever motion stops, we need a FULL redraw --*/
    display_tailpipe (gg->current_display, FULL, gg);
  }
}

#undef T1DON
#undef T1DOFF

