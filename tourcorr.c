/* tourcorr.c */
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

#include "vars.h"
#include "externs.h"

#define CTON true
#define CTOFF false

void
alloc_tourcorr (displayd *dsp, ggobid *gg)
{
  datad *d = dsp->d;
  gint nc = d->ncols;

  /* first index is the projection dimensions, second dimension is ncols */
  arrayd_init_null(&dsp->tcorr1.Fa);
  arrayd_alloc(&dsp->tcorr1.Fa, 1, nc);

  arrayd_init_null(&dsp->tcorr1.Fz);
  arrayd_alloc(&dsp->tcorr1.Fz, 1, nc);

  arrayd_init_null(&dsp->tcorr1.F);
  arrayd_alloc(&dsp->tcorr1.F, 1, nc);

  arrayd_init_null(&dsp->tcorr1.Ga);
  arrayd_alloc(&dsp->tcorr1.Ga, 1, nc);

  arrayd_init_null(&dsp->tcorr1.Gz);
  arrayd_alloc(&dsp->tcorr1.Gz, 1, nc);

  arrayd_init_null(&dsp->tcorr1.G);
  arrayd_alloc(&dsp->tcorr1.G, 1, nc);

  arrayd_init_null(&dsp->tcorr1.Va);
  arrayd_alloc(&dsp->tcorr1.Va, 1, nc);
  arrayd_init_null(&dsp->tcorr1.Vz);
  arrayd_alloc(&dsp->tcorr1.Vz, 1, nc);

  arrayd_init_null(&dsp->tcorr1.tv);
  arrayd_alloc(&dsp->tcorr1.tv, 1, nc);

  vectori_init_null(&dsp->tcorr1.active_vars);
  vectori_alloc(&dsp->tcorr1.active_vars, nc);
  vectorf_init_null(&dsp->tcorr1.lambda);
  vectorf_alloc(&dsp->tcorr1.lambda, nc);
  vectorf_init_null(&dsp->tcorr1.tau);
  vectorf_alloc(&dsp->tcorr1.tau, nc);
  vectorf_init_null(&dsp->tcorr1.tinc);
  vectorf_alloc(&dsp->tcorr1.tinc, nc);

  /* manipulation controls */
  arrayd_init_null(&dsp->tc1_manbasis);
  arrayd_alloc(&dsp->tc1_manbasis, 2, nc);
  arrayd_init_null(&dsp->tc2_manbasis);
  arrayd_alloc(&dsp->tc2_manbasis, 2, nc);

  /* first index is the projection dimensions, second dimension is ncols */
  arrayd_init_null(&dsp->tcorr2.Fa);
  arrayd_alloc(&dsp->tcorr2.Fa, 1, nc);

  arrayd_init_null(&dsp->tcorr2.Fz);
  arrayd_alloc(&dsp->tcorr2.Fz, 1, nc);

  arrayd_init_null(&dsp->tcorr2.F);
  arrayd_alloc(&dsp->tcorr2.F, 1, nc);

  arrayd_init_null(&dsp->tcorr2.Ga);
  arrayd_alloc(&dsp->tcorr2.Ga, 1, nc);

  arrayd_init_null(&dsp->tcorr2.Gz);
  arrayd_alloc(&dsp->tcorr2.Gz, 1, nc);

  arrayd_init_null(&dsp->tcorr2.G);
  arrayd_alloc(&dsp->tcorr2.G, 1, nc);

  arrayd_init_null(&dsp->tcorr2.Va);
  arrayd_alloc(&dsp->tcorr2.Va, 1, nc);
  arrayd_init_null(&dsp->tcorr2.Vz);
  arrayd_alloc(&dsp->tcorr2.Vz, 1, nc);

  arrayd_init_null(&dsp->tcorr2.tv);
  arrayd_alloc(&dsp->tcorr2.tv, 1, nc);

  vectori_init_null(&dsp->tcorr2.active_vars);
  vectori_alloc(&dsp->tcorr2.active_vars, nc);
  vectorf_init_null(&dsp->tcorr2.lambda);
  vectorf_alloc(&dsp->tcorr2.lambda, nc);
  vectorf_init_null(&dsp->tcorr2.tau);
  vectorf_alloc(&dsp->tcorr2.tau, nc);
  vectorf_init_null(&dsp->tcorr2.tinc);
  vectorf_alloc(&dsp->tcorr2.tinc, nc);

}

/*-- eliminate the nc columns contained in *cols --*/
void
tourcorr_realloc_down (gint nc, gint *cols, datad *d, ggobid *gg)
{
  displayd *dsp;
  GList *l;
  for (l=gg->displays; l; l=l->next) {
    dsp = (displayd *) l->data;
    if (dsp->d == d) {
      arrayd_delete_cols (&dsp->tcorr1.Fa, nc, cols);
      arrayd_delete_cols (&dsp->tcorr1.Fz, nc, cols);
      arrayd_delete_cols (&dsp->tcorr1.F, nc, cols);
      arrayd_delete_cols (&dsp->tcorr1.Ga, nc, cols);
      arrayd_delete_cols (&dsp->tcorr1.Gz, nc, cols);
      arrayd_delete_cols (&dsp->tcorr1.G, nc, cols);
      arrayd_delete_cols (&dsp->tcorr1.Va, nc, cols);
      arrayd_delete_cols (&dsp->tcorr1.Vz, nc, cols);
      arrayd_delete_cols (&dsp->tcorr1.tv, nc, cols);

      vectori_delete_els (&dsp->tcorr1.active_vars, nc, cols);
      vectorf_delete_els (&dsp->tcorr1.lambda, nc, cols);
      vectorf_delete_els (&dsp->tcorr1.tau, nc, cols);
      vectorf_delete_els (&dsp->tcorr1.tinc, nc, cols);

      arrayd_delete_cols (&dsp->tc1_manbasis, (gint) nc, cols);
      arrayd_delete_cols (&dsp->tc2_manbasis, (gint) nc, cols);

      arrayd_delete_cols (&dsp->tcorr2.Fa, nc, cols);
      arrayd_delete_cols (&dsp->tcorr2.Fz, nc, cols);
      arrayd_delete_cols (&dsp->tcorr2.F, nc, cols);
      arrayd_delete_cols (&dsp->tcorr2.Ga, nc, cols);
      arrayd_delete_cols (&dsp->tcorr2.Gz, nc, cols);
      arrayd_delete_cols (&dsp->tcorr2.G, nc, cols);
      arrayd_delete_cols (&dsp->tcorr2.Va, nc, cols);
      arrayd_delete_cols (&dsp->tcorr2.Vz, nc, cols);
      arrayd_delete_cols (&dsp->tcorr2.tv, nc, cols);

      vectori_delete_els (&dsp->tcorr2.active_vars, nc, cols);
      vectorf_delete_els (&dsp->tcorr2.lambda, nc, cols);
      vectorf_delete_els (&dsp->tcorr2.tau, nc, cols);
      vectorf_delete_els (&dsp->tcorr2.tinc, nc, cols);
    }
  }
}

/*-- append columns for a total of nc columns --*/
void
tourcorr_realloc_up (gint nc, datad *d, ggobid *gg)
{
  displayd *dsp;
  GList *l;
  gint old_ncols, i;

  if (nc < 3)
    return;

  for (l=gg->displays; l; l=l->next) {
    dsp = (displayd *) l->data;
    if (dsp->displaytype != scatterplot)
      continue;

    old_ncols = dsp->tcorr1.Fa.ncols;

    if (old_ncols < 2 && nc >= 2) {
      display_tourcorr_init(dsp, gg);
    }

    if (dsp->d == d) {
      arrayd_add_cols (&dsp->tcorr1.Fa, nc);
      arrayd_add_cols (&dsp->tcorr1.Fz, nc);
      arrayd_add_cols (&dsp->tcorr1.F, nc);
      arrayd_add_cols (&dsp->tcorr1.Ga, nc);
      arrayd_add_cols (&dsp->tcorr1.Gz, nc);
      arrayd_add_cols (&dsp->tcorr1.G, nc);
      arrayd_add_cols (&dsp->tcorr1.Va, nc);
      arrayd_add_cols (&dsp->tcorr1.Vz, nc);
      arrayd_add_cols (&dsp->tcorr1.tv, nc);

      vectori_realloc (&dsp->tcorr1.active_vars, nc);
      vectorf_realloc (&dsp->tcorr1.lambda, nc);
      vectorf_realloc (&dsp->tcorr1.tau, nc);
      vectorf_realloc (&dsp->tcorr1.tinc, nc);

      arrayd_add_cols (&dsp->tc1_manbasis, (gint) nc);
      arrayd_add_cols (&dsp->tc2_manbasis, (gint) nc);

      arrayd_add_cols (&dsp->tcorr2.Fa, nc);
      arrayd_add_cols (&dsp->tcorr2.Fz, nc);
      arrayd_add_cols (&dsp->tcorr2.F, nc);
      arrayd_add_cols (&dsp->tcorr2.Ga, nc);
      arrayd_add_cols (&dsp->tcorr2.Gz, nc);
      arrayd_add_cols (&dsp->tcorr2.G, nc);
      arrayd_add_cols (&dsp->tcorr2.Va, nc);
      arrayd_add_cols (&dsp->tcorr2.Vz, nc);
      arrayd_add_cols (&dsp->tcorr2.tv, nc);

      vectori_realloc (&dsp->tcorr2.active_vars, nc);
      vectorf_realloc (&dsp->tcorr2.lambda, nc);
      vectorf_realloc (&dsp->tcorr2.tau, nc);
      vectorf_realloc (&dsp->tcorr2.tinc, nc);

      /* need to zero extra cols */
      for (i=old_ncols; i<nc; i++) {
        dsp->tcorr1.Fa.vals[0][i] = 0.0;
        dsp->tcorr1.Fz.vals[0][i] = 0.0;
        dsp->tcorr1.F.vals[0][i] = 0.0;
        dsp->tcorr1.Ga.vals[0][i] = 0.0;
        dsp->tcorr1.Gz.vals[0][i] = 0.0;
        dsp->tcorr1.G.vals[0][i] = 0.0;
        dsp->tcorr1.Va.vals[0][i] = 0.0;
        dsp->tcorr1.Vz.vals[0][i] = 0.0;
        dsp->tcorr1.tv.vals[0][i] = 0.0;
        dsp->tcorr1.active_vars.els[i] = 0;
        dsp->tcorr1.lambda.els[i] = 0.0;
        dsp->tcorr1.tau.els[i] = 0.0;
        dsp->tcorr1.tinc.els[i] = 0.0;

        dsp->tcorr2.Fa.vals[0][i] = 0.0;
        dsp->tcorr2.Fz.vals[0][i] = 0.0;
        dsp->tcorr2.F.vals[0][i] = 0.0;
        dsp->tcorr2.Ga.vals[0][i] = 0.0;
        dsp->tcorr2.Gz.vals[0][i] = 0.0;
        dsp->tcorr2.G.vals[0][i] = 0.0;
        dsp->tcorr2.Va.vals[0][i] = 0.0;
        dsp->tcorr2.Vz.vals[0][i] = 0.0;
        dsp->tcorr2.tv.vals[0][i] = 0.0;
        dsp->tcorr2.active_vars.els[i] = 0;
        dsp->tcorr2.lambda.els[i] = 0.0;
        dsp->tcorr2.tau.els[i] = 0.0;
        dsp->tcorr2.tinc.els[i] = 0.0;
      }
    }
  }
}

void
free_tourcorr(displayd *dsp)
{
  /*  gint k;*/
  /*  datad *d = dsp->d;*/
  /*  gint nc = d->ncols;*/

  vectori_free(&dsp->tcorr1.active_vars);
  vectorf_free(&dsp->tcorr1.lambda);
  vectorf_free(&dsp->tcorr1.tau);
  vectorf_free(&dsp->tcorr1.tinc);

  arrayd_free(&dsp->tcorr1.Fa, 0, 0);
  arrayd_free(&dsp->tcorr1.Fz, 0, 0);
  arrayd_free(&dsp->tcorr1.F, 0, 0);

  arrayd_free(&dsp->tcorr1.Ga, 0, 0);
  arrayd_free(&dsp->tcorr1.Gz, 0, 0);
  arrayd_free(&dsp->tcorr1.G, 0, 0);

  arrayd_free(&dsp->tcorr1.Va, 0, 0);
  arrayd_free(&dsp->tcorr1.Vz, 0, 0);
  arrayd_free(&dsp->tcorr1.tv, 0, 0);

  arrayd_free(&dsp->tc1_manbasis, 0, 0);
  arrayd_free(&dsp->tc2_manbasis, 0, 0);

  vectori_free(&dsp->tcorr2.active_vars);
  vectorf_free(&dsp->tcorr2.lambda);
  vectorf_free(&dsp->tcorr2.tau);
  vectorf_free(&dsp->tcorr2.tinc);

  arrayd_free(&dsp->tcorr2.Fa, 0, 0);
  arrayd_free(&dsp->tcorr2.Fz, 0, 0);
  arrayd_free(&dsp->tcorr2.F, 0, 0);

  arrayd_free(&dsp->tcorr2.Ga, 0, 0);
  arrayd_free(&dsp->tcorr2.Gz, 0, 0);
  arrayd_free(&dsp->tcorr2.G, 0, 0);

  arrayd_free(&dsp->tcorr2.Va, 0, 0);
  arrayd_free(&dsp->tcorr2.Vz, 0, 0);
  arrayd_free(&dsp->tcorr2.tv, 0, 0);

}

void 
display_tourcorr_init (displayd *dsp, ggobid *gg) {
  gint i, j;
  datad *d = dsp->d;
  cpaneld *cpanel = &dsp->cpanel;
  gint nc = d->ncols;

  if (nc < 3)
    return;

  alloc_tourcorr(dsp, gg);
 
  /* Initialize first variable as the vertical and rest of the variables as
     the horizontal variables */
  dsp->tcorr1.nactive = 2;
  dsp->tcorr1.active_vars.els[0] = 0;
  dsp->tcorr1.active_vars.els[1] = 1;

  if (nc < 8) {
    dsp->tcorr2.nactive = nc-2;
    for (j=0; j<nc-2; j++)
      dsp->tcorr2.active_vars.els[j] = j+2;
  }
  else {
    dsp->tcorr2.nactive = 3;
    for (j=0; j<3; j++)
      dsp->tcorr2.active_vars.els[j] = j+2;
    for (j=3; j<nc-2; j++)
      dsp->tcorr2.active_vars.els[j] = 0;
  }

  /* declare starting vertical base as first variable */
  for (i=0; i<1; i++)
    for (j=0; j<nc; j++)
      dsp->tcorr1.Fa.vals[i][j] = dsp->tcorr1.Fz.vals[i][j] = 
        dsp->tcorr1.F.vals[i][j] = 
        dsp->tcorr1.Ga.vals[i][j] = dsp->tcorr1.Gz.vals[i][j] = 0.0;

  dsp->tcorr1.F.vals[0][dsp->tcorr1.active_vars.els[0]] = 1.0;

  for (i=0; i<1; i++)
    for (j=0; j<nc; j++)
      dsp->tcorr2.Fa.vals[i][j] = dsp->tcorr2.Fz.vals[i][j] = 
        dsp->tcorr2.F.vals[i][j] = 
        dsp->tcorr2.Ga.vals[i][j] = dsp->tcorr2.Gz.vals[i][j] = 0.0;

  dsp->tcorr2.F.vals[0][dsp->tcorr2.active_vars.els[0]] = 1.0;

  dsp->tcorr1.dist_az = 1.0;
  dsp->tcorr1.delta = cpanel->tcorr1_step*M_PI_2/10.0;
  dsp->tcorr1.nsteps = 1; 
  dsp->tcorr1.stepcntr = 1;

  dsp->tcorr1.idled = 0;
  dsp->tcorr1.get_new_target = true;

  /* vertical */
  dsp->tcorr2.dist_az = 1.0;
  dsp->tcorr2.delta = cpanel->tcorr2_step*M_PI_2/10.0;
  dsp->tcorr2.nsteps = 1; 
  dsp->tcorr2.stepcntr = 1;

  dsp->tcorr2.idled = 0;
  dsp->tcorr2.get_new_target = true;

  /* manip */
  dsp->tc_manip_mode = CMANIP_OFF;
  dsp->tc1_manip_var = dsp->tcorr1.active_vars.els[0];
  dsp->tc2_manip_var = dsp->tcorr2.active_vars.els[0];

  /* pp */
  dsp->tcorr1.target_selection_method = 0;
  dsp->tcorr2.target_selection_method = 0;
  dsp->tcorr_axes = true;
}

/*-- called from the Options menu --*/
void
tourcorr_fade_vars_cb (GtkCheckMenuItem *w, guint action) 
{
  ggobid *gg = GGobiFromWidget(GTK_WIDGET(w), true);

  gg->tourcorr.fade_vars = !gg->tourcorr.fade_vars;
}

void tourcorr_speed_set(gint slidepos, ggobid *gg) {
  displayd *dsp = gg->current_display; 
  cpaneld *cpanel = &dsp->cpanel;
  extern void speed_set (gint, gfloat *, gfloat *, gfloat, gint *, gint *);

  speed_set(slidepos, &cpanel->tcorr1_step, &dsp->tcorr1.delta,  
    dsp->tcorr1.dist_az, &dsp->tcorr1.nsteps, &dsp->tcorr1.stepcntr);

  speed_set(slidepos, &cpanel->tcorr2_step, &dsp->tcorr2.delta,  
    dsp->tcorr2.dist_az, &dsp->tcorr2.nsteps, &dsp->tcorr2.stepcntr);

  cpanel->tc_slidepos = slidepos;
}

void tourcorr_pause (cpaneld *cpanel, gboolean state, ggobid *gg)
{
  cpanel->tcorr1_paused = state;
  cpanel->tcorr2_paused = state;

  tourcorr_func (!cpanel->tcorr1_paused, gg->current_display, gg);
  tourcorr_func (!cpanel->tcorr2_paused, gg->current_display, gg);

  if (cpanel->tcorr1_paused && cpanel->tcorr2_paused) {
    /*-- whenever motion stops, we need a FULL redraw --*/
    display_tailpipe (gg->current_display, FULL, gg);
  }
}

void 
tourcorr_horvar_set (gint jvar, ggobid *gg)
{
  gint j, k;
  gboolean active=false;
  displayd *dsp = gg->current_display;
  datad *d = dsp->d;

  for (j=0; j<dsp->tcorr1.nactive; j++)
    if (jvar == dsp->tcorr1.active_vars.els[j])
      active = true;

  /* deselect var if tcorr1.nactive > 1 */
  if (active) {
    if (dsp->tcorr1.nactive > 1) {
      for (j=0; j<dsp->tcorr1.nactive; j++) {
        if (jvar == dsp->tcorr1.active_vars.els[j]) 
          break;
      }
      if (j<dsp->tcorr1.nactive-1) {
        for (k=j; k<dsp->tcorr1.nactive-1; k++){
          dsp->tcorr1.active_vars.els[k] = dsp->tcorr1.active_vars.els[k+1];
        }
      }
      dsp->tcorr1.nactive--;
      if (!gg->tourcorr.fade_vars) /* set current position without sel var */
      {
        gt_basis(dsp->tcorr1.Fa, dsp->tcorr1.nactive, dsp->tcorr1.active_vars, 
          d->ncols, (gint) 1);
        arrayd_copy(&dsp->tcorr1.Fa, &dsp->tcorr1.F);
	/*        copy_mat(dsp->tcorr1.F.vals, dsp->tcorr1.Fa.vals, d->ncols, 1);*/
      }
    }
  }
  else { /* not active, so add the variable */
    if (jvar > dsp->tcorr1.active_vars.els[dsp->tcorr1.nactive-1]) {
      dsp->tcorr1.active_vars.els[dsp->tcorr1.nactive] = jvar;
    }
    else if (jvar < dsp->tcorr1.active_vars.els[0]) {
      for (j=dsp->tcorr1.nactive; j>0; j--) {
          dsp->tcorr1.active_vars.els[j] = dsp->tcorr1.active_vars.els[j-1];
      }
      dsp->tcorr1.active_vars.els[0] = jvar;
    }
    else {
      gint jtmp = dsp->tcorr1.nactive;
      for (j=0; j<dsp->tcorr1.nactive-1; j++) {
        if (jvar > dsp->tcorr1.active_vars.els[j] && jvar < 
          dsp->tcorr1.active_vars.els[j+1]) {
          jtmp = j+1;
          break;
        }
      }
      for (j=dsp->tcorr1.nactive-1;j>=jtmp; j--) 
          dsp->tcorr1.active_vars.els[j+1] = dsp->tcorr1.active_vars.els[j];
      dsp->tcorr1.active_vars.els[jtmp] = jvar;
    }
    dsp->tcorr1.nactive++;
  }

  dsp->tcorr1.get_new_target = true;

}

void 
tourcorr_vervar_set (gint jvar, ggobid *gg)
{
  gint j, k;
  gboolean active=false;
  displayd *dsp = gg->current_display;
  datad *d = dsp->d;

  for (j=0; j<dsp->tcorr2.nactive; j++)
    if (jvar == dsp->tcorr2.active_vars.els[j])
      active = true;

  /* deselect var if tcorr2.nactive > 1 */
  if (active) {
    if (dsp->tcorr2.nactive > 1) {
      for (j=0; j<dsp->tcorr2.nactive; j++) {
        if (jvar == dsp->tcorr2.active_vars.els[j]) 
          break;
      }
      if (j<dsp->tcorr2.nactive-1) {
        for (k=j; k<dsp->tcorr2.nactive-1; k++){
          dsp->tcorr2.active_vars.els[k] = dsp->tcorr2.active_vars.els[k+1];
        }
      }
      dsp->tcorr2.nactive--;

      if (!gg->tourcorr.fade_vars) /* set current position without sel var */
      {
        gt_basis(dsp->tcorr2.Fa, dsp->tcorr2.nactive, dsp->tcorr2.active_vars, 
          d->ncols, (gint) 1);
        arrayd_copy(&dsp->tcorr2.Fa, &dsp->tcorr2.F);
	/*        copy_mat(dsp->tcorr2.F.vals, dsp->tcorr2.Fa.vals, d->ncols, 1);*/
      }
    }
  }
  else { /* not active, so add the variable */
    if (jvar > dsp->tcorr2.active_vars.els[dsp->tcorr2.nactive-1]) {
      dsp->tcorr2.active_vars.els[dsp->tcorr2.nactive] = jvar;
    }
    else if (jvar < dsp->tcorr2.active_vars.els[0]) {
      for (j=dsp->tcorr2.nactive; j>0; j--) {
          dsp->tcorr2.active_vars.els[j] = dsp->tcorr2.active_vars.els[j-1];
      }
      dsp->tcorr2.active_vars.els[0] = jvar;
    }
    else {
      gint jtmp = dsp->tcorr2.nactive;
      for (j=0; j<dsp->tcorr2.nactive-1; j++) {
        if (jvar > dsp->tcorr2.active_vars.els[j] && jvar < dsp->tcorr2.active_vars.els[j+1])
        {
          jtmp = j+1;
          break;
        }
      }
      for (j=dsp->tcorr2.nactive-1;j>=jtmp; j--) 
          dsp->tcorr2.active_vars.els[j+1] = dsp->tcorr2.active_vars.els[j];
      dsp->tcorr2.active_vars.els[jtmp] = jvar;
    }
    dsp->tcorr2.nactive++;
  }

  dsp->tcorr2.get_new_target = true;
}

void
tourcorr_manip_var_set (gint j, gint btn, ggobid *gg)
{
  displayd *dsp = gg->current_display;

  if (btn == 1)
    dsp->tc1_manip_var = j;    
  if (btn == 2)
    dsp->tc2_manip_var = j;
}

void
tourcorr_varsel (gint jvar, gint button, datad *d, ggobid *gg)
{
  if (d->vcirc_ui.jcursor == GDK_HAND2) {
    tourcorr_manip_var_set (jvar, button, gg);
    d->vcirc_ui.jcursor = (gint) NULL;  /*-- start with the default cursor --*/
  }
  else {
    if (button == 1) { 
      tourcorr_horvar_set (jvar, gg);
    }
    else if (button == 2 || button == 3) {
      tourcorr_vervar_set (jvar, gg);
    }
  }
}

void
tourcorr_projdata(splotd *sp, glong **world_data, datad *d, ggobid *gg) {
  gint i, j, m;
  displayd *dsp = (displayd *) sp->displayptr;

  for (m=0; m<d->nrows_in_plot; m++)
  {
    i = d->rows_in_plot[m];
    sp->planar[i].x = 0;
    sp->planar[i].y = 0;
    for (j=0; j<d->ncols; j++)
    {
      sp->planar[i].x += (gint)(dsp->tcorr1.F.vals[0][j]*world_data[i][j]);
      sp->planar[i].y += (gint)(dsp->tcorr2.F.vals[0][j]*world_data[i][j]);
    }
  }
}

void
tourcorr_run(displayd *dsp, ggobid *gg)
{
  datad *d = dsp->d;
  extern gboolean reached_target(gint, gint, gint, gfloat *, gfloat *);
  extern void increment_tour(vector_f, vector_f, gint *, gint *, gfloat, 
    gfloat, gint);
  extern void do_last_increment(vector_f, vector_f, gint);
  extern gint path(array_d, array_d, array_d, gint, gint, array_d, 
    array_d, array_d, vector_f, array_d, array_d, array_d,
    vector_f, vector_f, gint *, gint *, gfloat *, gfloat);
  extern void tour_reproject(vector_f, array_d, array_d, array_d, 
    array_d, array_d, gint, gint);
  /*  extern void copy_mat(gdouble **, gdouble **, gint, gint);*/
  gint i, nv;
  gint pathprob = 0;

  if (!dsp->tcorr1.get_new_target && 
      !reached_target(dsp->tcorr1.nsteps, dsp->tcorr1.stepcntr, 0, 0, 0)) {

    increment_tour(dsp->tcorr1.tinc, dsp->tcorr1.tau, &dsp->tcorr1.nsteps, 
      &dsp->tcorr1.stepcntr, dsp->tcorr1.dist_az, dsp->tcorr1.delta, (gint) 1);

    tour_reproject(dsp->tcorr1.tinc, dsp->tcorr1.G, dsp->tcorr1.Ga, 
      dsp->tcorr1.Gz, dsp->tcorr1.F, dsp->tcorr1.Va, d->ncols, (gint) 1);
  }
  else { /* do final clean-up and get new target */
    if (!dsp->tcorr1.get_new_target) {
      do_last_increment(dsp->tcorr1.tinc, dsp->tcorr1.tau, (gint) 1);
      tour_reproject(dsp->tcorr1.tinc, dsp->tcorr1.G, dsp->tcorr1.Ga, 
        dsp->tcorr1.Gz, dsp->tcorr1.F, dsp->tcorr1.Va, d->ncols, (gint) 1);
      }
    
    arrayd_copy(&dsp->tcorr1.F, &dsp->tcorr1.Fa);
    /*    copy_mat(dsp->tcorr1.Fa.vals, dsp->tcorr1.F.vals, d->ncols, 1);*/
    nv = 0;
    for (i=0; i<d->ncols; i++) 
      if (fabs(dsp->tcorr1.Fa.vals[0][i]) > 0.01) {
        nv++;
      }
    if (nv == 1 && dsp->tcorr1.nactive == 1) /* only generate new dir if num of
                                           active/used variables is > 1 */
      dsp->tcorr1.get_new_target = true;
    else {
      gt_basis(dsp->tcorr1.Fz, dsp->tcorr1.nactive, 
        dsp->tcorr1.active_vars, d->ncols, 
        (gint) 1);
      pathprob = path(dsp->tcorr1.Fa, dsp->tcorr1.Fz, dsp->tcorr1.F, 
        d->ncols, (gint) 1, 
        dsp->tcorr1.Ga, dsp->tcorr1.Gz, dsp->tcorr1.G, dsp->tcorr1.lambda, 
        dsp->tcorr1.tv, dsp->tcorr1.Va, dsp->tcorr1.Vz,
        dsp->tcorr1.tau, dsp->tcorr1.tinc, &dsp->tcorr1.nsteps, 
        &dsp->tcorr1.stepcntr, 
        &dsp->tcorr1.dist_az, dsp->tcorr1.delta);
      dsp->tcorr1.get_new_target = false;
    }
  }

  if (!dsp->tcorr2.get_new_target && 
      !reached_target(dsp->tcorr2.nsteps, dsp->tcorr2.stepcntr, 0, 0, 0)) {
    increment_tour(dsp->tcorr2.tinc, dsp->tcorr2.tau, &dsp->tcorr2.nsteps, 
      &dsp->tcorr2.stepcntr, dsp->tcorr2.dist_az, dsp->tcorr2.delta, (gint) 1);

    tour_reproject(dsp->tcorr2.tinc, dsp->tcorr2.G, dsp->tcorr2.Ga, 
      dsp->tcorr2.Gz, dsp->tcorr2.F, dsp->tcorr2.Va, d->ncols, (gint) 1);
  }
  else { /* do final clean-up and get new target */
    if (!dsp->tcorr2.get_new_target) {
      do_last_increment(dsp->tcorr2.tinc, dsp->tcorr2.tau, (gint) 1);
      tour_reproject(dsp->tcorr2.tinc, dsp->tcorr2.G, dsp->tcorr2.Ga, 
        dsp->tcorr2.Gz, dsp->tcorr2.F, dsp->tcorr2.Va, d->ncols, (gint) 1);
    }
    arrayd_copy(&dsp->tcorr2.F, &dsp->tcorr2.Fa);
    /*    copy_mat(dsp->tcorr2.Fa.vals, dsp->tcorr2.F.vals, d->ncols, 1);*/
    nv = 0;
    for (i=0; i<d->ncols; i++) 
      if (fabs(dsp->tcorr2.Fa.vals[0][i]) > 0.01) {
        nv++;
      }
    if (nv == 1 && dsp->tcorr2.nactive == 1) /* only generate new dir if num of
                                           active/used variables is > 1 */
      dsp->tcorr2.get_new_target = true;
    else {
      gt_basis(dsp->tcorr2.Fz, dsp->tcorr2.nactive, dsp->tcorr2.active_vars, d->ncols, 
        (gint) 1);
      path(dsp->tcorr2.Fa, dsp->tcorr2.Fz, dsp->tcorr2.F, d->ncols, (gint) 1, 
        dsp->tcorr2.Ga, dsp->tcorr2.Gz, dsp->tcorr2.G, dsp->tcorr2.lambda, 
        dsp->tcorr2.tv, dsp->tcorr2.Va,
	   dsp->tcorr2.Vz,
        dsp->tcorr2.tau, dsp->tcorr2.tinc, &dsp->tcorr2.nsteps, 
        &dsp->tcorr2.stepcntr, &dsp->tcorr2.dist_az, dsp->tcorr2.delta);
      dsp->tcorr2.get_new_target = false;
    }
  }
  
  display_tailpipe (dsp, FULL, gg);

  varcircles_refresh (d, gg);
}

void
tourcorr_do_step(displayd *dsp, ggobid *gg)
{
  tourcorr_run(dsp, gg);
}

gint
tourcorr_idle_func (displayd *dsp)
{
  ggobid *gg = GGobiFromDisplay (dsp);
  cpaneld *cpanel = &dsp->cpanel;
  gboolean doit = !cpanel->tcorr1_paused;

  if (doit) {
    tourcorr_run(dsp, gg);
    gdk_flush ();
  }

  return (doit);
}

void tourcorr_func (gboolean state, displayd *dsp, ggobid *gg)
{
  if (state) {
    if (dsp->tcorr1.idled == 0) {
      dsp->tcorr1.idled = gtk_idle_add_priority (G_PRIORITY_LOW,
        (GtkFunction) tourcorr_idle_func, dsp);
    }
    gg->tourcorr.idled = 1;
  } else {
    if (dsp->tcorr1.idled) {
      gtk_idle_remove (dsp->tcorr1.idled);
      dsp->tcorr1.idled = 0;
    }
    gg->tourcorr.idled = 0;
  }
}

void tourcorr_reinit(ggobid *gg)
{
  int j, m;
  displayd *dsp = gg->current_display;
  datad *d = dsp->d;

  for (j=0; j<d->ncols; j++) {
    dsp->tcorr1.F.vals[0][j] = 0.;
    dsp->tcorr1.Fa.vals[0][j] = 0.;
  }
  m = dsp->tcorr1.active_vars.els[0];
  dsp->tcorr1.F.vals[0][m] = 1.;
  dsp->tcorr1.Fa.vals[0][m] = 1.;

  dsp->tcorr1.get_new_target = true;

  for (j=0; j<d->ncols; j++) {
    dsp->tcorr2.F.vals[0][j] = 0.;
    dsp->tcorr2.Fa.vals[0][j] = 0.;
  }
  m = dsp->tcorr2.active_vars.els[0];
  dsp->tcorr2.F.vals[0][m] = 1.;
  dsp->tcorr2.Fa.vals[0][m] = 1.;

  dsp->tcorr2.get_new_target = true;

  dsp->tcorr1.nsteps = 0;
  dsp->tcorr2.nsteps = 0;
  dsp->tcorr1.stepcntr = 0;
  dsp->tcorr2.stepcntr = 0;

  display_tailpipe (dsp, FULL, gg);

  varcircles_refresh (d, gg);

}

void tourcorr_scramble(ggobid *gg)
{
  gint j;
  displayd *dsp = gg->current_display;
  datad *d = dsp->d;
  /*gint i, nc = d->ncols;*/

  for (j=0; j<d->ncols; j++) {
    dsp->tcorr1.F.vals[0][j] = 0.;
    dsp->tcorr1.Fa.vals[0][j] = 0.;
  }

  for (j=0; j<d->ncols; j++) {
    dsp->tcorr2.F.vals[0][j] = 0.;
    dsp->tcorr2.Fa.vals[0][j] = 0.;
  }

  gt_basis(dsp->tcorr1.Fa, dsp->tcorr1.nactive, dsp->tcorr1.active_vars, 
    d->ncols, (gint) 1);
  arrayd_copy(&dsp->tcorr1.Fa, &dsp->tcorr1.F);
  /*  copy_mat(dsp->tcorr1.F.vals, dsp->tcorr1.Fa.vals, d->ncols, 1);*/

  gt_basis(dsp->tcorr2.Fa, dsp->tcorr2.nactive, dsp->tcorr2.active_vars, 
    d->ncols, (gint) 1);
  arrayd_copy(&dsp->tcorr2.Fa, &dsp->tcorr2.F);
  /*  copy_mat(dsp->tcorr2.F.vals, dsp->tcorr2.Fa.vals, d->ncols, 1);*/

  dsp->tcorr1.nsteps = 1; 
  dsp->tcorr1.stepcntr = 1;
  dsp->tcorr2.nsteps = 1; 
  dsp->tcorr2.stepcntr = 1;

  dsp->tcorr1.get_new_target = true;
  dsp->tcorr2.get_new_target = true;

  display_tailpipe (dsp, FULL, gg);

  varcircles_refresh (d, gg);
}

/* Variable manipulation */
void
tourcorr_manip_init(gint p1, gint p2, splotd *sp) 
{
  displayd *dsp = (displayd *) sp->displayptr;
  datad *d = dsp->d;
  cpaneld *cpanel = &dsp->cpanel;
  ggobid *gg = GGobiFromSPlot(sp);
  gint j;
  gint n1vars = dsp->tcorr1.nactive, n2vars = dsp->tcorr2.nactive;
  gfloat ftmp, tol = 0.01; 
  gboolean dontdoit = false;
  extern void gram_schmidt(gdouble *, gdouble*, gint);
  extern gdouble calc_norm(gdouble *, gint);

  /* need to turn off tour */
  if (!cpanel->tcorr1_paused && !cpanel->tcorr2_paused) {
    tourcorr_func(CTOFF, gg->current_display, gg);
  }

  dsp->tc1_phi = 0.;
  dsp->tc2_phi = 0.;

  /* gets mouse position */
  dsp->tc1_pos = dsp->tc1_pos_old = p1;
  dsp->tc2_pos = dsp->tc2_pos_old = p2;

  /* initializes indicator for manip var being one of existing vars */
  dsp->tc1_manipvar_inc = false;
  dsp->tc2_manipvar_inc = false;

  /* check if manip var is one of existing vars */
  /* n1vars, n2vars is the number of variables, excluding the
     manip var in hor and vert directions */
  for (j=0; j<dsp->tcorr1.nactive; j++)
    if (dsp->tcorr1.active_vars.els[j] == dsp->tc1_manip_var) {
      dsp->tc1_manipvar_inc = true;
      n1vars--;
    }
  for (j=0; j<dsp->tcorr2.nactive; j++)
    if (dsp->tcorr2.active_vars.els[j] == dsp->tc2_manip_var) {
      dsp->tc2_manipvar_inc = true;
      n2vars--;
    }

  /* make manip basis, from existing projection */
  /* 0 will be the remainder of the projection, and
     1 will be the indicator vector for the manip var */
  for (j=0; j<d->ncols; j++) {
    dsp->tc1_manbasis.vals[0][j] = dsp->tcorr1.F.vals[0][j];
    dsp->tc1_manbasis.vals[1][j] = 0.;
  }
  for (j=0; j<d->ncols; j++) {
    dsp->tc2_manbasis.vals[0][j] = dsp->tcorr2.F.vals[0][j];
    dsp->tc2_manbasis.vals[1][j] = 0.;
  }
  dsp->tc1_manbasis.vals[1][dsp->tc1_manip_var]=1.;
  dsp->tc2_manbasis.vals[1][dsp->tc2_manip_var]=1.;

  if (n1vars > 0)
  {
    gram_schmidt(dsp->tc1_manbasis.vals[0],  dsp->tc1_manbasis.vals[1],
      d->ncols);
    ftmp = calc_norm (dsp->tc1_manbasis.vals[1], d->ncols);
    if (ftmp < tol)
      dontdoit = true;
  }
  if (n2vars > 0)
  {
    gram_schmidt(dsp->tc2_manbasis.vals[0],  dsp->tc2_manbasis.vals[1],
      d->ncols);
    ftmp = calc_norm(dsp->tc2_manbasis.vals[1], d->ncols);
    if (ftmp < tol)
      dontdoit = true;
  } 

  if (dontdoit) {
    disconnect_motion_signal (sp);
  }
}

void
tourcorr_manip(gint p1, gint p2, splotd *sp, ggobid *gg) 
{
  displayd *dsp = (displayd *) sp->displayptr;
  datad *d = dsp->d;
  cpaneld *cpanel = &dsp->cpanel;
  gfloat xcosphi=1., xsinphi=0., ycosphi=1., ysinphi=0.;
  gfloat distx = 0., disty = 0.;
  gfloat denom = (float) MIN(sp->max.x, sp->max.y)/2.;
  gint actual_nxvars = dsp->tcorr1.nactive, actual_nyvars = dsp->tcorr2.nactive;
  gint j;
  gboolean offscreen = false;

  /* check if off the plot window */
  if (p1 > sp->max.x || p1 < 0 ||
      p2 > sp->max.y || p2 <0)
    offscreen = true;

  if (dsp->tc1_manipvar_inc)
    actual_nxvars = dsp->tcorr1.nactive-1;
  if (dsp->tc2_manipvar_inc)
    actual_nyvars = dsp->tcorr2.nactive-1;

  if (!offscreen) {
    dsp->tc1_pos_old = dsp->tc1_pos;
    dsp->tc2_pos_old = dsp->tc2_pos;
  
    dsp->tc1_pos = p1;
    dsp->tc2_pos = p2;

    if (actual_nxvars > 0 || actual_nyvars > 0)
    {
      if (dsp->tc_manip_mode == CMANIP_VERT)
      {
        distx = 0.;
        if (actual_nyvars > 0)
          disty = dsp->tc2_pos_old - dsp->tc2_pos;
      }
      else if (dsp->tc_manip_mode == CMANIP_HOR)
      {
        if (actual_nxvars > 0)
          distx = dsp->tc1_pos - dsp->tc1_pos_old;
        disty = 0.;
      }
      else if (dsp->tc_manip_mode == CMANIP_COMB)
      {
        if (actual_nxvars > 0)
          distx = dsp->tc1_pos - dsp->tc1_pos_old;
        if (actual_nyvars > 0)
          disty = dsp->tc2_pos_old - dsp->tc2_pos;
      }
      else if (dsp->tc_manip_mode == CMANIP_EQUAL)
      {
        if (actual_nxvars > 0)
          distx = dsp->tc1_pos - dsp->tc1_pos_old;
        if (actual_nyvars > 0)
          disty = dsp->tc2_pos_old - dsp->tc2_pos;
        if (fabs(distx) != fabs(disty))
        {
          distx = (distx+disty)/1.414214;
          disty = distx;
        }
      }

      dsp->tc1_phi = dsp->tc1_phi + distx / denom;
      dsp->tc2_phi = dsp->tc2_phi + disty / denom;
  
      xcosphi = (gfloat) cos((gdouble) dsp->tc1_phi);
      xsinphi = (gfloat) sin((gdouble) dsp->tc1_phi);
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
      ycosphi = (float) cos((double) dsp->tc2_phi);
      ysinphi = (float) sin((double) dsp->tc2_phi);
      if (ycosphi > 1.0)
      {
        ycosphi = 1.0;
        ysinphi = 0.0;
      }
      else if (ycosphi < -1.0)
      {
        ycosphi = -1.0;
        ysinphi = 0.0;
      }
    }

    /* generate the projection basis */
    if (actual_nxvars > 0) 
    {
      for (j=0; j<d->ncols; j++)
        dsp->tcorr1.F.vals[0][j] = xcosphi * dsp->tc1_manbasis.vals[0][j] + 
         xsinphi * dsp->tc1_manbasis.vals[1][j];
    }
 
    if (actual_nyvars > 0)
    {
      for (j=0; j<d->ncols; j++)
        dsp->tcorr2.F.vals[0][j] = ycosphi * dsp->tc2_manbasis.vals[0][j] + 
         ysinphi * dsp->tc2_manbasis.vals[1][j];
    }

    display_tailpipe (dsp, FULL, gg);
    varcircles_refresh (d, gg);
  }
  else {
    disconnect_motion_signal (sp);
    arrayd_copy(&dsp->tcorr1.F, &dsp->tcorr1.Fa);
    /*    copy_mat(dsp->tcorr1.Fa.vals, dsp->tcorr1.F.vals, d->ncols, 1);*/
    arrayd_copy(&dsp->tcorr2.F, &dsp->tcorr2.Fa);
    /*    copy_mat(dsp->tcorr2.Fa.vals, dsp->tcorr2.F.vals, d->ncols, 1);*/
    dsp->tcorr1.get_new_target = true;
    dsp->tcorr2.get_new_target = true;
    if (!cpanel->tcorr1_paused && !cpanel->tcorr2_paused)
      tourcorr_func(CTON, gg->current_display, gg);
  }
}

void
tourcorr_manip_end(splotd *sp) 
{
  displayd *dsp = (displayd *) sp->displayptr;
  datad *d = dsp->d;
  cpaneld *cpanel = &dsp->cpanel;
  ggobid *gg = GGobiFromSPlot(sp);
  /*  extern void copy_mat(gdouble **, gdouble **, gint, gint);*/

  disconnect_motion_signal (sp);

  arrayd_copy(&dsp->tcorr1.F, &dsp->tcorr1.Fa);
  /*  copy_mat(dsp->tcorr1.Fa.vals, dsp->tcorr1.F.vals, d->ncols, 1);*/
  arrayd_copy(&dsp->tcorr2.F, &dsp->tcorr2.Fa);
  /*  copy_mat(dsp->tcorr2.Fa.vals, dsp->tcorr2.F.vals, d->ncols, 1);*/
  dsp->tcorr1.get_new_target = true;
  dsp->tcorr2.get_new_target = true;

  /* need to turn on tour? */
  if (!cpanel->tcorr1_paused && !cpanel->tcorr2_paused) {
    tourcorr_func(CTON, gg->current_display, gg);

    /*-- whenever motion stops, we need a FULL redraw --*/
    display_tailpipe (gg->current_display, FULL, gg);
  }

}

#undef CTON
#undef CTOFF
