/* tour2d.c */
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

#ifdef WIN32 
#include <windows.h>
#endif

#include <math.h>
#include <unistd.h>

#include "vars.h"
#include "externs.h"

#define T2DON true
#define T2DOFF false

void
display_tour2d_init_null (displayd *dsp, ggobid *gg)
{
  arrayd_init_null(&dsp->t2d.Fa);
  arrayd_init_null(&dsp->t2d.Fz);
  arrayd_init_null(&dsp->t2d.F);

  arrayd_init_null(&dsp->t2d.Ga);
  arrayd_init_null(&dsp->t2d.Gz);
  arrayd_init_null(&dsp->t2d.G);

  arrayd_init_null(&dsp->t2d.Va);
  arrayd_init_null(&dsp->t2d.Vz);

  arrayd_init_null(&dsp->t2d.tv);

  vectori_init_null(&dsp->t2d.active_vars);
  vectorf_init_null(&dsp->t2d.lambda);
  vectorf_init_null(&dsp->t2d.tau);
  vectorf_init_null(&dsp->t2d.tinc);

  /* manipulation variables */
  arrayd_init_null(&dsp->t2d_Rmat1);
  arrayd_init_null(&dsp->t2d_Rmat2);
  arrayd_init_null(&dsp->t2d_mvar_3dbasis);
  arrayd_init_null(&dsp->t2d_manbasis);
}

void
alloc_tour2d (displayd *dsp, ggobid *gg)
{
  datad *d = dsp->d;
  gint nc = d->ncols;

  /* first index is the projection dimensions, second dimension is ncols */
  arrayd_alloc(&dsp->t2d.Fa, 2, nc);
  arrayd_alloc(&dsp->t2d.Fz, 2, nc);
  arrayd_alloc(&dsp->t2d.F, 2, nc);

  arrayd_alloc(&dsp->t2d.Ga, 2, nc);
  arrayd_alloc(&dsp->t2d.Gz, 2, nc);
  arrayd_alloc(&dsp->t2d.G, 2, nc);

  arrayd_alloc(&dsp->t2d.Va, 2, nc);
  arrayd_alloc(&dsp->t2d.Vz, 2, nc);

  arrayd_alloc(&dsp->t2d.tv, 2, nc);

  vectori_alloc(&dsp->t2d.active_vars, nc);
  vectorf_alloc(&dsp->t2d.lambda, nc);
  vectorf_alloc(&dsp->t2d.tau, nc);
  vectorf_alloc(&dsp->t2d.tinc, nc);

  /* manipulation variables */
  arrayd_alloc(&dsp->t2d_Rmat1, 3, 3);
  arrayd_alloc(&dsp->t2d_Rmat2, 3, 3);
  arrayd_alloc(&dsp->t2d_mvar_3dbasis, 3, 3);
  arrayd_alloc(&dsp->t2d_manbasis, 3, nc);

}

/*-- eliminate the nc columns contained in *cols --*/
void
tour2d_realloc_down (gint nc, gint *cols, datad *d, ggobid *gg)
{
  displayd *dsp;
  GList *l;
  for (l=gg->displays; l; l=l->next) {
    dsp = (displayd *) l->data;
    if (dsp->d == d) {
      arrayd_delete_cols (&dsp->t2d.Fa, nc, cols);
      arrayd_delete_cols (&dsp->t2d.Fz, nc, cols);
      arrayd_delete_cols (&dsp->t2d.F, nc, cols);
      arrayd_delete_cols (&dsp->t2d.Ga, nc, cols);
      arrayd_delete_cols (&dsp->t2d.Gz, nc, cols);
      arrayd_delete_cols (&dsp->t2d.G, nc, cols);
      arrayd_delete_cols (&dsp->t2d.Va, nc, cols);
      arrayd_delete_cols (&dsp->t2d.Vz, nc, cols);
      arrayd_delete_cols (&dsp->t2d.tv, nc, cols);

      vectori_delete_els (&dsp->t2d.active_vars, nc, cols);
      vectorf_delete_els (&dsp->t2d.lambda, nc, cols);
      vectorf_delete_els (&dsp->t2d.tau, nc, cols);
      vectorf_delete_els (&dsp->t2d.tinc, nc, cols);

      arrayd_delete_cols (&dsp->t2d_manbasis, (gint) nc, cols);
    }
  }
}

/*-- append columns for a total of nc columns --*/
/*-- we don't know for certain that tour has been initialized, do we? --*/
void
tour2d_realloc_up (gint nc, datad *d, ggobid *gg)
{
  displayd *dsp;
  GList *l;
  gint old_ncols, i;

  /*printf("%d\n", nc);*/
  for (l=gg->displays; l; l=l->next) {
    dsp = (displayd *) l->data;
    if (dsp->displaytype != scatterplot)
      continue;

    /*
     * because display_tour2d_init_null has been performed even if
     * alloc_tour2d has not, Fa.ncols has been initialized.
    */
    old_ncols = dsp->t2d.Fa.ncols;

    if (old_ncols < MIN_NVARS_FOR_TOUR2D && nc >= MIN_NVARS_FOR_TOUR2D) {
      display_tour2d_init(dsp, gg);
    }

    if (dsp->d == d) {
      arrayd_add_cols (&dsp->t2d.Fa, nc);
      arrayd_add_cols (&dsp->t2d.Fz, nc);
      arrayd_add_cols (&dsp->t2d.F, nc);
      arrayd_add_cols (&dsp->t2d.Ga, nc);
      arrayd_add_cols (&dsp->t2d.Gz, nc);
      arrayd_add_cols (&dsp->t2d.G, nc);
      arrayd_add_cols (&dsp->t2d.Va, nc);
      arrayd_add_cols (&dsp->t2d.Vz, nc);
      arrayd_add_cols (&dsp->t2d.tv, nc);

      vectori_realloc (&dsp->t2d.active_vars, nc);
      vectorf_realloc (&dsp->t2d.lambda, nc);
      vectorf_realloc (&dsp->t2d.tau, nc);
      vectorf_realloc (&dsp->t2d.tinc, nc);

      arrayd_add_cols (&dsp->t2d_manbasis, (gint) nc);

      /* need to zero extra cols */
      for (i=old_ncols; i<nc; i++) {
        dsp->t2d.Fa.vals[0][i] = dsp->t2d.Fa.vals[1][i] = 0.0;
        dsp->t2d.Fz.vals[0][i] = dsp->t2d.Fz.vals[1][i] = 0.0;
        dsp->t2d.F.vals[0][i] = dsp->t2d.F.vals[1][i] = 0.0;
        dsp->t2d.Ga.vals[0][i] = dsp->t2d.Ga.vals[1][i] = 0.0;
        dsp->t2d.Gz.vals[0][i] = dsp->t2d.Gz.vals[1][i] = 0.0;
        dsp->t2d.G.vals[0][i] = dsp->t2d.G.vals[1][i] = 0.0;
        dsp->t2d.Va.vals[0][i] = dsp->t2d.Va.vals[1][i] = 0.0;
        dsp->t2d.Vz.vals[0][i] = dsp->t2d.Vz.vals[1][i] = 0.0;
        dsp->t2d.tv.vals[0][i] = dsp->t2d.tv.vals[1][i] = 0.0;
        dsp->t2d.active_vars.els[i] = 0;
        dsp->t2d.lambda.els[i] = 0.0;
        dsp->t2d.tau.els[i] = 0.0;
        dsp->t2d.tinc.els[i] = 0.0;
      }

    }
  }
}

void
free_tour2d(displayd *dsp)
{
  /*  gint k;*/
  /*  datad *d = dsp->d;*/
  /*  gint nc = d->ncols;*/

  vectori_free(&dsp->t2d.active_vars);
  vectorf_free(&dsp->t2d.lambda);
  vectorf_free(&dsp->t2d.tau);
  vectorf_free(&dsp->t2d.tinc);

  arrayd_free(&dsp->t2d.Fa, 0, 0);
  arrayd_free(&dsp->t2d.Fz, 0, 0);
  arrayd_free(&dsp->t2d.F, 0, 0);

  arrayd_free(&dsp->t2d.Ga, 0, 0);
  arrayd_free(&dsp->t2d.Gz, 0, 0);
  arrayd_free(&dsp->t2d.G, 0, 0);

  arrayd_free(&dsp->t2d.Va, 0, 0);
  arrayd_free(&dsp->t2d.Vz, 0, 0);
  arrayd_free(&dsp->t2d.tv, 0, 0);

  arrayd_free(&dsp->t2d_Rmat1, 0, 0);
  arrayd_free(&dsp->t2d_Rmat2, 0, 0);
  arrayd_free(&dsp->t2d_mvar_3dbasis, 0, 0);
  arrayd_free(&dsp->t2d_manbasis, 0, 0);
}

void 
display_tour2d_init (displayd *dsp, ggobid *gg) {
  gint i, j;
  datad *d = dsp->d;
  cpaneld *cpanel = &dsp->cpanel;
  gint nc = d->ncols;

  if (nc < MIN_NVARS_FOR_TOUR2D)
    return;

  alloc_tour2d(dsp, gg);
 
    /* Initialize starting subset of active variables */
  if (nc < 8) {
    dsp->t2d.nactive = nc;
    for (j=0; j<nc; j++)
      dsp->t2d.active_vars.els[j] = j;
  }
  else {
    dsp->t2d.nactive = 3;
    for (j=0; j<3; j++)
      dsp->t2d.active_vars.els[j] = j;
    for (j=3; j<nc; j++)
      dsp->t2d.active_vars.els[j] = 0;
  }

  /* declare starting base as first p chosen variables */
  for (i=0; i<2; i++)
    for (j=0; j<nc; j++)
      dsp->t2d.Fa.vals[i][j] = dsp->t2d.Fz.vals[i][j] = 
        dsp->t2d.F.vals[i][j] = dsp->t2d.Ga.vals[i][j] = 
        dsp->t2d.Gz.vals[i][j] = 0.0;

  for (i=0; i<2; i++)
  {
    dsp->t2d.Fz.vals[i][dsp->t2d.active_vars.els[i]] =
      dsp->t2d.Fa.vals[i][dsp->t2d.active_vars.els[i]] = 
      dsp->t2d.F.vals[i][dsp->t2d.active_vars.els[i]] =
      dsp->t2d.Ga.vals[i][dsp->t2d.active_vars.els[i]] = 
      dsp->t2d.Gz.vals[i][dsp->t2d.active_vars.els[i]] = 1.0;
  }

  dsp->t2d.dist_az = 0.0;
  dsp->t2d.delta = cpanel->t2d_step*M_PI_2/10.0;
  dsp->t2d.tang = 0.0;
  dsp->t2d.nsteps = 1; 
  dsp->t2d.stepcntr = 1;

  dsp->t2d.idled = 0;
  dsp->t2d.get_new_target = true;

  /* manip */
  dsp->t2d_manip_mode = MANIP_OFF;
  dsp->t2d_manip_var = 0;

  /* pp */
  dsp->t2d.target_selection_method = 0;
  dsp->t2d_ppda = NULL;
  dsp->t2d_axes = true;
}

/*-- called from the Options menu --*/
void
tour2d_fade_vars_cb (GtkCheckMenuItem *w, guint action) 
{
  ggobid *gg = GGobiFromWidget(GTK_WIDGET(w), true);

  gg->tour2d.fade_vars = !gg->tour2d.fade_vars;
}

void tour2d_speed_set(gint slidepos, ggobid *gg) {
  displayd *dsp = gg->current_display; 
  cpaneld *cpanel = &dsp->cpanel;
  extern void speed_set (gint, gfloat *, gfloat *, gint *, gint *);

  cpanel->t2d_slidepos = slidepos;
  speed_set(slidepos, &cpanel->t2d_step, &dsp->t2d.delta, 
    &dsp->t2d.nsteps, &dsp->t2d.stepcntr);
}

void tour2d_pause (cpaneld *cpanel, gboolean state, ggobid *gg) {
  cpanel->t2d_paused = state;

  tour2d_func (!cpanel->t2d_paused, gg->current_display, gg);

  if (cpanel->t2d_paused) {
    /*-- whenever motion stops, we need a FULL redraw --*/
    display_tailpipe (gg->current_display, FULL, gg);
  }
}

void 
tour2dvar_set (gint jvar, ggobid *gg)
{
  gint j, jtmp, k;
  gboolean active=false;
  displayd *dsp = gg->current_display;
  datad *d = dsp->d;
  /*  extern void copy_mat(gdouble **, gdouble **, gint, gint);*/

  for (j=0; j<dsp->t2d.nactive; j++)
    if (jvar == dsp->t2d.active_vars.els[j])
      active = true;

  /* deselect var if t2d.nactive > 2 */
  if (active) {
    if (dsp->t2d.nactive > 2) {
      for (j=0; j<dsp->t2d.nactive; j++) {
        if (jvar == dsp->t2d.active_vars.els[j]) 
          break;
      }
      if (j<dsp->t2d.nactive-1) {
        for (k=j; k<dsp->t2d.nactive-1; k++) {
          dsp->t2d.active_vars.els[k] = dsp->t2d.active_vars.els[k+1];
        }
      }
      dsp->t2d.nactive--;
 
      if (!gg->tour2d.fade_vars) /* set current position without sel var */
      {
        gt_basis(dsp->t2d.Fa, dsp->t2d.nactive, dsp->t2d.active_vars, 
          d->ncols, (gint) 2);
        arrayd_copy(&dsp->t2d.Fa, &dsp->t2d.F);
	/*        copy_mat(dsp->t2d.F.vals, dsp->t2d.Fa.vals, d->ncols, 2);*/
      }
    }
  }
  else { /* not active, so add the variable */
    if (jvar > dsp->t2d.active_vars.els[dsp->t2d.nactive-1]) {
      dsp->t2d.active_vars.els[dsp->t2d.nactive] = jvar;
    }
    else if (jvar < dsp->t2d.active_vars.els[0]) {
      for (j=dsp->t2d.nactive; j>0; j--) {
          dsp->t2d.active_vars.els[j] = dsp->t2d.active_vars.els[j-1];
      }
      dsp->t2d.active_vars.els[0] = jvar;
    }
    else {
      for (j=0; j<dsp->t2d.nactive-1; j++) {
        if (jvar > dsp->t2d.active_vars.els[j] && jvar < dsp->t2d.active_vars.els[j+1]) {
          jtmp = j+1;
          break;
        }
      }
      for (j=dsp->t2d.nactive-1;j>=jtmp; j--) 
          dsp->t2d.active_vars.els[j+1] = dsp->t2d.active_vars.els[j];
      dsp->t2d.active_vars.els[jtmp] = jvar;
    }
    dsp->t2d.nactive++;
  }

  dsp->t2d.get_new_target = true;
}

static void
tour2d_manip_var_set (gint j, ggobid *gg)
{
  displayd *dsp = gg->current_display;

  dsp->t2d_manip_var = j;    
}

void
tour2d_varsel (gint jvar, gint button, datad *d, ggobid *gg)
{
  displayd *dsp = gg->current_display;
  extern gint realloc_optimize0_p(optimize0_param *, gint, gint, gint);

/*-- we don't care which button it is --*/
  if (d->vcirc_ui.jcursor == GDK_HAND2) {
    tour2d_manip_var_set (jvar, gg);
    varcircles_cursor_set_default (d);

  } else {
    tour2dvar_set (jvar, gg);
    /*    if (dsp->t2d.target_selection_method == 1)*/
      realloc_optimize0_p(&dsp->t2d_pp_op, d->nrows_in_plot, 
        dsp->t2d.nactive, 2);
  }
}

void
tour2d_projdata(splotd *sp, glong **world_data, datad *d, ggobid *gg) {
  gint i, j, m;
  displayd *dsp = (displayd *) sp->displayptr;

  for (m=0; m<d->nrows_in_plot; m++)
  {
    i = d->rows_in_plot[m];
    sp->planar[i].x = 0;
    sp->planar[i].y = 0;
    for (j=0; j<d->ncols; j++)
    {
      sp->planar[i].x += (gint)(dsp->t2d.F.vals[0][j]*world_data[i][j]);
      sp->planar[i].y += (gint)(dsp->t2d.F.vals[1][j]*world_data[i][j]);
    }
  }
}

void tour2d_scramble(ggobid *gg)
{
  int i, j;
  displayd *dsp = gg->current_display;
  datad *d = dsp->d;
  gint nc = d->ncols;
  /*  extern void copy_mat(gdouble **, gdouble **, gint, gint);*/

  for (i=0; i<2; i++)
    for (j=0; j<nc; j++)
      dsp->t2d.Fa.vals[i][j] = dsp->t2d.Fz.vals[i][j] = 
        dsp->t2d.F.vals[i][j] = dsp->t2d.Ga.vals[i][j] = 
        dsp->t2d.Gz.vals[i][j] = 0.0;

  gt_basis(dsp->t2d.Fa, dsp->t2d.nactive, dsp->t2d.active_vars, 
    d->ncols, (gint) 2);
  arrayd_copy(&dsp->t2d.Fa, &dsp->t2d.F);
  /*  copy_mat(dsp->t2d.F.vals, dsp->t2d.Fa.vals, d->ncols, 2);*/

  dsp->t2d.nsteps = 1; 
  dsp->t2d.stepcntr = 1;

  dsp->t2d.get_new_target = true;

  display_tailpipe (dsp, FULL, gg);

  varcircles_refresh (d, gg);
}

void
tour2d_run(displayd *dsp, ggobid *gg)
{
  extern gboolean reached_target(gint, gint, gfloat, gfloat, gint, gfloat *, gfloat *);
  extern gboolean reached_target2(vector_f, vector_f, gint, gfloat *, gfloat *, gint);
  extern void increment_tour(vector_f, vector_f, gint *, gint *, gfloat, 
    gfloat, gfloat *, gint);
  extern void do_last_increment(vector_f, vector_f, gfloat, gint);
  extern gint path(array_d, array_d, array_d, gint, gint, array_d, 
    array_d, array_d, vector_f, array_d, array_d, array_d,
    vector_f, vector_f, gint *, gint *, gfloat *, gfloat *, gfloat);
  extern void tour_reproject(vector_f, array_d, array_d, array_d, 
    array_d, array_d, gint, gint);
  extern void t2d_ppdraw(gfloat, ggobid *);
  /*  extern void copy_mat(gdouble **, gdouble **, gint, gint);*/
  extern gboolean checkequiv(gdouble **, gdouble **, gint, gint);
  datad *d = dsp->d;
  cpaneld *cpanel = &dsp->cpanel;
  gint i, j, nv;
  static gint count = 0;
  gboolean revert_random = false;
  static gfloat oindxval = -999.0;
  gint k;
  gboolean chosen;
  gfloat eps = .01;
  gint pathprob = 0;

  if (!dsp->t2d.get_new_target && 
      !reached_target(dsp->t2d.nsteps, dsp->t2d.stepcntr, dsp->t2d.tang,
       dsp->t2d.dist_az,
       dsp->t2d.target_selection_method, &dsp->t2d.ppval, &oindxval)) {

    increment_tour(dsp->t2d.tinc, dsp->t2d.tau, &dsp->t2d.nsteps, 
      &dsp->t2d.stepcntr, dsp->t2d.dist_az, dsp->t2d.delta, &dsp->t2d.tang, 
      (gint) 2);
    tour_reproject(dsp->t2d.tinc, dsp->t2d.G, dsp->t2d.Ga, dsp->t2d.Gz, 
      dsp->t2d.F, dsp->t2d.Va, d->ncols, (gint) 2);

    /* plot pp indx */
    if (dsp->t2d_ppda != NULL) {

      revert_random = t2d_switch_index(cpanel->t2d_pp_indx, 
        0, gg);
      count++;
      if (count == 10) {
        count = 0;
        t2d_ppdraw(dsp->t2d.ppval, gg);
      }
    }
  }
  else { /* do final clean-up and get new target */
    if (dsp->t2d.get_new_target) {
      if (dsp->t2d.target_selection_method == 1)
      {
        dsp->t2d_pp_op.index_best = dsp->t2d.ppval;
        oindxval = dsp->t2d.ppval;
        for (i=0; i<2; i++)
          for (j=0; j<dsp->t2d.nactive; j++)
            dsp->t2d_pp_op.proj_best.vals[j][i] = 
              dsp->t2d.F.vals[i][dsp->t2d.active_vars.els[j]];
      }
    }
    else 
    {
      if (dsp->t2d.target_selection_method == 1)
        t2d_ppdraw(dsp->t2d.ppval, gg);
      else
      {
        do_last_increment(dsp->t2d.tinc, dsp->t2d.tau, 
          dsp->t2d.dist_az, (gint) 2);
        tour_reproject(dsp->t2d.tinc, dsp->t2d.G, dsp->t2d.Ga, dsp->t2d.Gz,
          dsp->t2d.F, dsp->t2d.Va, d->ncols, (gint) 2);
      }
    }
    nv = 0;
    for (i=0; i<d->ncols; i++) {
      chosen = false;
      for (k=0; k<dsp->t2d.nactive; k++) {
        if (dsp->t2d.active_vars.els[k] == i) {
          chosen = true;
          break;
        }
      }
      if (!chosen) {
        if (fabs(dsp->t2d.F.vals[0][i]) < eps && 
          fabs(dsp->t2d.F.vals[1][i]) < eps)
          dsp->t2d.F.vals[0][i] = dsp->t2d.F.vals[1][i] = 0.0;
        if (fabs(dsp->t2d.F.vals[0][i]) > eps || 
          fabs(dsp->t2d.F.vals[1][i]) > eps) {
          nv++;
	}
      }
    }
    arrayd_copy(&dsp->t2d.F, &dsp->t2d.Fa);
    if (nv == 0 && dsp->t2d.nactive <= 2) /* only generate new dir if num of
                                           active/used variables is > 2 -
                                           this code allows for motion to
                                           continue while a variable is 
                                           fading out. */
      dsp->t2d.get_new_target = true;
    else {
      if (dsp->t2d.target_selection_method == 0) {
        gt_basis(dsp->t2d.Fz, dsp->t2d.nactive, dsp->t2d.active_vars, 
          d->ncols, (gint) 2);
      }
      else if (dsp->t2d.target_selection_method == 1) {
        /* pp guided tour  */
        revert_random = t2d_switch_index(cpanel->t2d_pp_indx, 
          dsp->t2d.target_selection_method, gg);

        if (!revert_random) {
          for (i=0; i<2; i++)
            for (j=0; j<dsp->t2d.nactive; j++)
              dsp->t2d.Fz.vals[i][dsp->t2d.active_vars.els[j]] = 
                dsp->t2d_pp_op.proj_best.vals[j][i];

          /* if the best projection is the same as the previous one, switch 
              to a random projection */
          if (!checkequiv(dsp->t2d.Fa.vals, dsp->t2d.Fz.vals, d->ncols, 2)) 
          {
            printf("Using random projection\n");
            gt_basis(dsp->t2d.Fz, dsp->t2d.nactive, dsp->t2d.active_vars, 
              d->ncols, (gint) 2);
            for (i=0; i<2; i++)
              for (j=0; j<dsp->t2d.nactive; j++)
                dsp->t2d_pp_op.proj_best.vals[j][i] = 
                  dsp->t2d.Fz.vals[i][dsp->t2d.active_vars.els[j]];
            revert_random = t2d_switch_index(cpanel->t2d_pp_indx, 
              dsp->t2d.target_selection_method, gg);
          }
          t2d_ppdraw(dsp->t2d.ppval, gg);
          count = 0;
#ifndef WIN32
          sleep(2);
#else
          Sleep(2);
#endif
        }
        else
        {
          gt_basis(dsp->t2d.Fz, dsp->t2d.nactive, dsp->t2d.active_vars, 
            d->ncols, (gint) 2);
        }
        
      }
      pathprob = path(dsp->t2d.Fa, dsp->t2d.Fz, dsp->t2d.F, d->ncols, 
        (gint) 2, dsp->t2d.Ga,
        dsp->t2d.Gz, dsp->t2d.G, dsp->t2d.lambda, dsp->t2d.tv, dsp->t2d.Va,
        dsp->t2d.Vz,
        dsp->t2d.tau, dsp->t2d.tinc, &dsp->t2d.nsteps, &dsp->t2d.stepcntr, 
        &dsp->t2d.dist_az, &dsp->t2d.tang, cpanel->t2d_step);
      if (pathprob != 0) {
        if (pathprob == 1) {
          gt_basis(dsp->t2d.Fa, dsp->t2d.nactive, dsp->t2d.active_vars, 
            d->ncols, (gint) 2);
          arrayd_copy(&dsp->t2d.Fa, &dsp->t2d.F);
	  /*          copy_mat(dsp->t2d.F.vals, dsp->t2d.Fa.vals, d->ncols, 2);*/
	}
        else if (pathprob == 2) {
          gt_basis(dsp->t2d.Fz, dsp->t2d.nactive, dsp->t2d.active_vars, 
            d->ncols, (gint) 2);
	}
        else if (pathprob == 3) {
          tour2d_scramble(gg);
	}
        dsp->t2d.get_new_target = true;
      }
      else 
        dsp->t2d.get_new_target = false;
    }
  }
  
  display_tailpipe (dsp, FULL_1PIXMAP, gg);
  varcircles_refresh (d, gg);
}

void
tour2d_do_step(displayd *dsp, ggobid *gg)
{
  tour2d_run(dsp, gg);
}

gint
tour2d_idle_func (displayd *dsp)
{
  ggobid *gg = GGobiFromDisplay (dsp);
  cpaneld *cpanel = &dsp->cpanel;
  gboolean doit = !cpanel->t2d_paused;

  if (doit) {
    tour2d_run (dsp, gg);
    gdk_flush ();
  }

  return (doit);
}

void tour2d_func (gboolean state, displayd *dsp, ggobid *gg)
{
  if (state) {
    if (dsp->t2d.idled == 0) {
      dsp->t2d.idled = gtk_idle_add_priority (G_PRIORITY_LOW,
                                   (GtkFunction) tour2d_idle_func, dsp);
    }
    gg->tour2d.idled = 1;
  } else {
    if (dsp->t2d.idled != 0) {
      gtk_idle_remove (dsp->t2d.idled);
      dsp->t2d.idled = 0;
    }
    gg->tour2d.idled = 0;
  }
}

void tour2d_reinit(ggobid *gg)
{
  int i, j;
  displayd *dsp = gg->current_display;
  datad *d = dsp->d;
  gint nc = d->ncols;

  for (i=0; i<2; i++)
    for (j=0; j<nc; j++)
      dsp->t2d.Fa.vals[i][j] = dsp->t2d.Fz.vals[i][j] = 
        dsp->t2d.F.vals[i][j] = dsp->t2d.Ga.vals[i][j] = 
        dsp->t2d.Gz.vals[i][j] = 0.0;

  for (i=0; i<2; i++)
  {
    dsp->t2d.Fz.vals[i][dsp->t2d.active_vars.els[i]] =
      dsp->t2d.Fa.vals[i][dsp->t2d.active_vars.els[i]] = 
      dsp->t2d.F.vals[i][dsp->t2d.active_vars.els[i]] =
      dsp->t2d.Ga.vals[i][dsp->t2d.active_vars.els[i]] = 
      dsp->t2d.Gz.vals[i][dsp->t2d.active_vars.els[i]] = 1.0;
  }
  /*  for (i=0; i<2; i++) {
    for (j=0; j<d->ncols; j++) {
      dsp->t2d.Fa.vals[i][j] = 0.;
      dsp->t2d.F.vals[i][j] = 0.;
    }
    dsp->t2d.Fa.vals[i][dsp->t2d.active_vars.els[i]] = 1.;
    dsp->t2d.F.vals[i][dsp->t2d.active_vars.els[i]] = 1.;
    }*/
  dsp->t2d.nsteps = 1; 
  dsp->t2d.stepcntr = 1;

  dsp->t2d.get_new_target = true;

  display_tailpipe (dsp, FULL, gg);

  varcircles_refresh (d, gg);
}

/* Variable manipulation */
void
tour2d_manip_init(gint p1, gint p2, splotd *sp) 
{
  displayd *dsp = (displayd *) sp->displayptr;
  datad *d = dsp->d;
  cpaneld *cpanel = &dsp->cpanel;
  ggobid *gg = GGobiFromSPlot(sp);
  gint j, k;
  gint n1vars = dsp->t2d.nactive;
  gfloat ftmp, tol = 0.01; 
  gdouble dtmp1;
  extern void gram_schmidt(gdouble *, gdouble*, gint);
  extern gdouble calc_norm(gdouble *, gint);
  extern gdouble inner_prod(gdouble *, gdouble *, gint);

  /* need to turn off tour */
  if (!cpanel->t2d_paused)
    tour2d_func(T2DOFF, gg->current_display, gg);

  dsp->t2d_manipvar_inc = false;
  dsp->t2d_pos1 = dsp->t2d_pos1_old = p1;
  dsp->t2d_pos2 = dsp->t2d_pos2_old = p2;
  /* check if manip var is one of existing vars */
  /* n1vars, n2vars is the number of variables, excluding the
     manip var in hor and vert directions */
  for (j=0; j<dsp->t2d.nactive; j++)
    if (dsp->t2d.active_vars.els[j] == dsp->t2d_manip_var) {
      dsp->t2d_manipvar_inc = true;
      n1vars--;
    }

  /* here need to check if the manip var is wholly contained in u, and
     if so do some check */

  if (n1vars > 1)
  {
    /* make manip basis, from existing projection */
    /* 0,1 will be the remainder of the projection, and
       2 will be the indicator vector for the manip var */
    for (j=0; j<d->ncols; j++) 
    {
      dsp->t2d_manbasis.vals[0][j] = dsp->t2d.F.vals[0][j];
      dsp->t2d_manbasis.vals[1][j] = dsp->t2d.F.vals[1][j];
      dsp->t2d_manbasis.vals[2][j] = 0.;
    }
    dsp->t2d_manbasis.vals[2][dsp->t2d_manip_var] = 1.;

    for (j=0; j<3; j++)
    {
      for (k=0; k<3; k++)
        dsp->t2d_mvar_3dbasis.vals[j][k] = 0.;
      dsp->t2d_mvar_3dbasis.vals[j][j] = 1.;
    }

    if ((inner_prod(dsp->t2d_manbasis.vals[0],dsp->t2d_manbasis.vals[2],
       d->ncols)>1.0-tol) || (inner_prod(dsp->t2d_manbasis.vals[1],
       dsp->t2d_manbasis.vals[2],d->ncols)>1.0-tol))
      ftmp = 0.0;
    else {
      gram_schmidt(dsp->t2d_manbasis.vals[0],  dsp->t2d_manbasis.vals[2],
        d->ncols);
      gram_schmidt(dsp->t2d_manbasis.vals[1],  dsp->t2d_manbasis.vals[2],
        d->ncols);
      ftmp = calc_norm (dsp->t2d_manbasis.vals[2], d->ncols);
    }

    while (ftmp < tol) {
        gt_basis(dsp->t2d.tv, dsp->t2d.nactive, dsp->t2d.active_vars, 
          d->ncols, (gint) 1);
        for (j=0; j<d->ncols; j++) 
          dsp->t2d_manbasis.vals[2][j] = dsp->t2d.tv.vals[0][j];
        gram_schmidt(dsp->t2d_manbasis.vals[0],  dsp->t2d_manbasis.vals[2],
          d->ncols);
        gram_schmidt(dsp->t2d_manbasis.vals[1],  dsp->t2d_manbasis.vals[2],
          d->ncols);
        ftmp = calc_norm (dsp->t2d_manbasis.vals[2], d->ncols);
    }

    dsp->t2d_no_dir_flag = false;
    if (dsp->t2d_manip_mode == MANIP_RADIAL)
      { /* check if variable is currently visible in plot */
      if ((dsp->t2d.F.vals[0][dsp->t2d_manip_var]*
        dsp->t2d.F.vals[0][dsp->t2d_manip_var] +
        dsp->t2d.F.vals[1][dsp->t2d_manip_var]*
        dsp->t2d.F.vals[1][dsp->t2d_manip_var]) < tol)
        dsp->t2d_no_dir_flag = true; /* no */
      else
	{ /* yes: set radial manip direction to be current direction
             of contribution */
        dsp->t2d_rx = (gfloat) dsp->t2d.F.vals[0][dsp->t2d_manip_var];
        dsp->t2d_ry = (gfloat) dsp->t2d.F.vals[1][dsp->t2d_manip_var];
        dtmp1 = sqrt(dsp->t2d_rx*dsp->t2d_rx+dsp->t2d_ry*dsp->t2d_ry);
        dsp->t2d_rx /= dtmp1;
        dsp->t2d_ry /= dtmp1;
      }
    }
  }

}

void
tour2d_manip(gint p1, gint p2, splotd *sp, ggobid *gg) 
{
  displayd *dsp = (displayd *) sp->displayptr;
  datad *d = dsp->d;
  gint actual_nvars = dsp->t2d.nactive;
  gboolean offscreen = false;
  gfloat phi, cosphi, sinphi, ca, sa, cosm, cospsi, sinpsi;
  gfloat distx, disty, x1, x2, y1, y2;
  gfloat denom = (float) MIN(sp->max.x, sp->max.y)/2.;
  gfloat tol = 0.01;
  gdouble dtmp1, dtmp2;
  gfloat len_motion;
  gint i,j,k;
  extern void gram_schmidt(gdouble *, gdouble*, gint);
  /*  extern void copy_mat(gdouble **, gdouble **, gint, gint);*/

  /* check if off the plot window */
  if (p1 > sp->max.x || p1 < 0 ||
      p2 > sp->max.y || p2 < 0)
    offscreen = true;

  if (dsp->t2d_manipvar_inc)
    actual_nvars = dsp->t2d.nactive-1;

  if (!offscreen) {
    dsp->t2d_pos1_old = dsp->t2d_pos1;
    dsp->t2d_pos2_old = dsp->t2d_pos2;
  
    dsp->t2d_pos1 = p1;
    dsp->t2d_pos2 = p2;

    if (actual_nvars > 1)
    {
      if (dsp->t2d_manip_mode != MANIP_ANGULAR)
      {
        if (dsp->t2d_manip_mode == MANIP_OBLIQUE) 
        {
          distx = dsp->t2d_pos1 - dsp->t2d_pos1_old;
          disty = dsp->t2d_pos2_old - dsp->t2d_pos2;
          /* seems to go in the wrong direction - 90deg? */
        }
        else if (dsp->t2d_manip_mode == MANIP_VERT) 
        {
          distx = 0.;
          disty = dsp->t2d_pos2_old - dsp->t2d_pos2;
        }
        else if (dsp->t2d_manip_mode == MANIP_HOR) 
        {
          distx = dsp->t2d_pos1 - dsp->t2d_pos1_old;
          disty = 0.;
        }
        else if (dsp->t2d_manip_mode == MANIP_RADIAL) 
        {
          if (dsp->t2d_no_dir_flag)
          {
            distx = dsp->t2d_pos1 - dsp->t2d_pos1_old;
            disty = dsp->t2d_pos2_old - dsp->t2d_pos2;
            dsp->t2d_rx = distx;
            dsp->t2d_ry = disty; 
            dtmp1 = sqrt(dsp->t2d_rx*dsp->t2d_rx+dsp->t2d_ry*dsp->t2d_ry);
            dsp->t2d_rx /= dtmp1;
            dsp->t2d_ry /= dtmp1;
            dsp->t2d_no_dir_flag = false;
          }
          distx = (dsp->t2d_rx*(dsp->t2d_pos1 - dsp->t2d_pos1_old) + 
            dsp->t2d_ry*(dsp->t2d_pos2_old - dsp->t2d_pos2))*dsp->t2d_rx;
          disty = (dsp->t2d_rx*(dsp->t2d_pos1 - dsp->t2d_pos1_old) + 
            dsp->t2d_ry*(dsp->t2d_pos2_old - dsp->t2d_pos2))*dsp->t2d_ry;
        }
        dtmp1 = (gdouble) (distx*distx+disty*disty);
        len_motion = (gfloat) sqrt(dtmp1);

        if (len_motion != 0)
        {
          phi = len_motion / denom;
     
          ca = distx/len_motion;
          sa = disty/len_motion;
      
          cosphi = (gfloat) cos((gdouble) phi);
          sinphi = (gfloat) sin((gdouble) phi);
          cosm = 1.0 - cosphi;
          dsp->t2d_Rmat2.vals[0][0] = ca*ca*cosphi + sa*sa;
          dsp->t2d_Rmat2.vals[0][1] = -cosm*ca*sa;
          dsp->t2d_Rmat2.vals[0][2] = sinphi*ca;
          dsp->t2d_Rmat2.vals[1][0] = -cosm*ca*sa;
          dsp->t2d_Rmat2.vals[1][1] = sa*sa*cosphi + ca*ca;
          dsp->t2d_Rmat2.vals[1][2] = sinphi*sa;
          dsp->t2d_Rmat2.vals[2][0] = -sinphi*ca;
          dsp->t2d_Rmat2.vals[2][1] = -sinphi*sa;
          dsp->t2d_Rmat2.vals[2][2] = cosphi;
        }
      }
      else 
      { /* angular constrained manipulation */
        if (dsp->t2d_pos1_old != sp->max.x/2 && 
          dsp->t2d_pos2_old != sp->max.y/2 &&
          dsp->t2d_pos1 != sp->max.x/2 && 
          dsp->t2d_pos2 != sp->max.y/2)
        {
          x1 = dsp->t2d_pos1_old - sp->max.x/2;
          y1 = dsp->t2d_pos2_old - sp->max.y/2;
          dtmp1 = sqrt(x1*x1+y1*y1);
          x1 /= dtmp1;
          y1 /= dtmp1;
          x2 = dsp->t2d_pos1 - sp->max.x/2;
          y2 = dsp->t2d_pos2 - sp->max.y/2;
          dtmp2 = sqrt(x2*x2+y2*y2);
          x2 /= dtmp2;
          y2 /= dtmp2;
          if (dtmp1 > tol && dtmp2 > tol)
          {
            cospsi = x1*x2+y1*y2;
            sinpsi = x1*y2-y1*x2;
          }
          else
          {
            cospsi = 1.;    
            sinpsi = 0.;
          }
        }
        else
        {
          cospsi = 1.;
          sinpsi = 0.;
        }
        dsp->t2d_Rmat2.vals[0][0] = cospsi;
        dsp->t2d_Rmat2.vals[0][1] = sinpsi;
        dsp->t2d_Rmat2.vals[0][2] = 0.;
        dsp->t2d_Rmat2.vals[1][0] = -sinpsi;
        dsp->t2d_Rmat2.vals[1][1] = cospsi;
        dsp->t2d_Rmat2.vals[1][2] = 0.;
        dsp->t2d_Rmat2.vals[2][0] = 0.;
        dsp->t2d_Rmat2.vals[2][1] = 0.;
        dsp->t2d_Rmat2.vals[2][2] = 1.;
      }

      for (i=0; i<3; i++) 
        for (j=0; j<3; j++)
        {
          dtmp1 = 0.;
          for (k=0; k<3; k++)
            dtmp1 += (dsp->t2d_mvar_3dbasis.vals[i][k]*
              dsp->t2d_Rmat2.vals[k][j]);
          dsp->t2d_Rmat1.vals[i][j] = dtmp1;
        }
      arrayd_copy(&dsp->t2d_Rmat1, &dsp->t2d_mvar_3dbasis);
      /*      copy_mat(dsp->t2d_mvar_3dbasis.vals, dsp->t2d_Rmat1.vals, 3, 3);*/
  
      gram_schmidt(dsp->t2d_mvar_3dbasis.vals[0], 
        dsp->t2d_mvar_3dbasis.vals[1], 3);
      gram_schmidt(dsp->t2d_mvar_3dbasis.vals[0], 
        dsp->t2d_mvar_3dbasis.vals[2], 3);
      gram_schmidt(dsp->t2d_mvar_3dbasis.vals[1], 
        dsp->t2d_mvar_3dbasis.vals[2], 3);

      for (j=0; j<d->ncols; j++)
      {
        dsp->t2d.F.vals[0][j] = 
          dsp->t2d_manbasis.vals[0][j]*dsp->t2d_mvar_3dbasis.vals[0][0] +
          dsp->t2d_manbasis.vals[1][j]*dsp->t2d_mvar_3dbasis.vals[0][1] +
          dsp->t2d_manbasis.vals[2][j]*dsp->t2d_mvar_3dbasis.vals[0][2];
        dsp->t2d.F.vals[1][j] = 
          dsp->t2d_manbasis.vals[0][j]*dsp->t2d_mvar_3dbasis.vals[1][0] +
          dsp->t2d_manbasis.vals[1][j]*dsp->t2d_mvar_3dbasis.vals[1][1] +
          dsp->t2d_manbasis.vals[2][j]*dsp->t2d_mvar_3dbasis.vals[1][2];
      }
    }
    display_tailpipe (dsp, FULL, gg);
    /*    display_tailpipe (dsp, FULL_1PIXMAP, gg);*/
    varcircles_refresh (d, gg);
  }
}

void
tour2d_manip_end(splotd *sp) 
{
  displayd *dsp = (displayd *) sp->displayptr;
  cpaneld *cpanel = &dsp->cpanel;
  ggobid *gg = GGobiFromSPlot(sp);
  /*  extern void copy_mat(gdouble **, gdouble **, gint, gint);*/

  disconnect_motion_signal (sp);

  arrayd_copy(&dsp->t2d.F, &dsp->t2d.Fa);
  /*  copy_mat(dsp->t2d.Fa.vals, dsp->t2d.F.vals, d->ncols, 2);*/
  dsp->t2d.get_new_target = true;

  /* need to turn on tour? */
  if (!cpanel->t2d_paused) {
    tour2d_func(T2DON, gg->current_display, gg);

    /*-- whenever motion stops, we need a FULL redraw --*/
    display_tailpipe (gg->current_display, FULL, gg);
  }
}

#undef T2DON
#undef T2DOFF
