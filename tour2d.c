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
alloc_tour2d (displayd *dsp, ggobid *gg)
{
  datad *d = dsp->d;
  gint nc = d->ncols;

  /* first index is the projection dimensions, second dimension is ncols */
  arrayf_init_null(&dsp->t2d.u0);
  arrayf_alloc(&dsp->t2d.u0, 2, nc);

  arrayf_init_null(&dsp->t2d.u1);
  arrayf_alloc(&dsp->t2d.u1, 2, nc);

  arrayf_init_null(&dsp->t2d.u);
  arrayf_alloc(&dsp->t2d.u, 2, nc);

  arrayf_init_null(&dsp->t2d.v0);
  arrayf_alloc(&dsp->t2d.v0, 2, nc);

  arrayf_init_null(&dsp->t2d.v1);
  arrayf_alloc(&dsp->t2d.v1, 2, nc);

  arrayf_init_null(&dsp->t2d.v);
  arrayf_alloc(&dsp->t2d.v, 2, nc);

  arrayf_init_null(&dsp->t2d.uvevec);
  arrayf_alloc(&dsp->t2d.uvevec, 2, nc);

  arrayf_init_null(&dsp->t2d.tv);
  arrayf_alloc(&dsp->t2d.tv, 2, nc);

  vectori_init_null(&dsp->t2d.vars);
  vectori_alloc(&dsp->t2d.vars, nc);
  vectorf_init_null(&dsp->t2d.lambda);
  vectorf_alloc(&dsp->t2d.lambda, nc);
  vectorf_init_null(&dsp->t2d.tau);
  vectorf_alloc(&dsp->t2d.tau, nc);
  vectorf_init_null(&dsp->t2d.tinc);
  vectorf_alloc(&dsp->t2d.tinc, nc);

  /* manipulation variables */
  arrayf_init_null(&dsp->t2d_Rmat1);
  arrayf_alloc(&dsp->t2d_Rmat1, 3, 3);
  arrayf_init_null(&dsp->t2d_Rmat2);
  arrayf_alloc(&dsp->t2d_Rmat2, 3, 3);
  arrayf_init_null(&dsp->t2d_mvar_3dbasis);
  arrayf_alloc(&dsp->t2d_mvar_3dbasis, 3, 3);
  arrayf_init_null(&dsp->t2d_manbasis);
  arrayf_alloc(&dsp->t2d_manbasis, 3, nc);

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
      arrayf_delete_cols (&dsp->t2d.u0, nc, cols);
      arrayf_delete_cols (&dsp->t2d.u1, nc, cols);
      arrayf_delete_cols (&dsp->t2d.u, nc, cols);
      arrayf_delete_cols (&dsp->t2d.v0, nc, cols);
      arrayf_delete_cols (&dsp->t2d.v1, nc, cols);
      arrayf_delete_cols (&dsp->t2d.v, nc, cols);
      arrayf_delete_cols (&dsp->t2d.uvevec, nc, cols);
      arrayf_delete_cols (&dsp->t2d.tv, nc, cols);

      vectori_delete_els (&dsp->t2d.vars, nc, cols);
      vectorf_delete_els (&dsp->t2d.lambda, nc, cols);
      vectorf_delete_els (&dsp->t2d.tau, nc, cols);
      vectorf_delete_els (&dsp->t2d.tinc, nc, cols);

      arrayf_delete_cols (&dsp->t2d_manbasis, (gint) nc, cols);
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

    old_ncols = dsp->t2d.u0.ncols;

    if (old_ncols < 2 && nc >= 2) {
      display_tour2d_init(dsp, gg);
    }

    if (dsp->d == d) {
      arrayf_add_cols (&dsp->t2d.u0, nc);
      arrayf_add_cols (&dsp->t2d.u1, nc);
      arrayf_add_cols (&dsp->t2d.u, nc);
      arrayf_add_cols (&dsp->t2d.v0, nc);
      arrayf_add_cols (&dsp->t2d.v1, nc);
      arrayf_add_cols (&dsp->t2d.v, nc);
      arrayf_add_cols (&dsp->t2d.uvevec, nc);
      arrayf_add_cols (&dsp->t2d.tv, nc);

      vectori_realloc (&dsp->t2d.vars, nc);
      vectorf_realloc (&dsp->t2d.lambda, nc);
      vectorf_realloc (&dsp->t2d.tau, nc);
      vectorf_realloc (&dsp->t2d.tinc, nc);

      arrayf_add_cols (&dsp->t2d_manbasis, (gint) nc);

      /* need to zero extra cols */
      for (i=old_ncols; i<nc; i++) {
        dsp->t2d.u0.vals[0][i] = dsp->t2d.u0.vals[1][i] = 0.0;
        dsp->t2d.u1.vals[0][i] = dsp->t2d.u1.vals[1][i] = 0.0;
        dsp->t2d.u.vals[0][i] = dsp->t2d.u.vals[1][i] = 0.0;
        dsp->t2d.v0.vals[0][i] = dsp->t2d.v0.vals[1][i] = 0.0;
        dsp->t2d.v1.vals[0][i] = dsp->t2d.v1.vals[1][i] = 0.0;
        dsp->t2d.v.vals[0][i] = dsp->t2d.v.vals[1][i] = 0.0;
        dsp->t2d.uvevec.vals[0][i] = dsp->t2d.uvevec.vals[1][i] = 0.0;
        dsp->t2d.tv.vals[0][i] = dsp->t2d.tv.vals[1][i] = 0.0;
        dsp->t2d.vars.els[i] = 0;
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

  vectori_free(&dsp->t2d.vars);
  vectorf_free(&dsp->t2d.lambda);
  vectorf_free(&dsp->t2d.tau);
  vectorf_free(&dsp->t2d.tinc);

  arrayf_free(&dsp->t2d.u0, 0, 0);
  arrayf_free(&dsp->t2d.u1, 0, 0);
  arrayf_free(&dsp->t2d.u, 0, 0);

  arrayf_free(&dsp->t2d.v0, 0, 0);
  arrayf_free(&dsp->t2d.v1, 0, 0);
  arrayf_free(&dsp->t2d.v, 0, 0);

  arrayf_free(&dsp->t2d.uvevec, 0, 0);
  arrayf_free(&dsp->t2d.tv, 0, 0);

  arrayf_free(&dsp->t2d_Rmat1, 0, 0);
  arrayf_free(&dsp->t2d_Rmat2, 0, 0);
  arrayf_free(&dsp->t2d_mvar_3dbasis, 0, 0);
  arrayf_free(&dsp->t2d_manbasis, 0, 0);
}

void 
display_tour2d_init (displayd *dsp, ggobid *gg) {
  gint i, j;
  datad *d = dsp->d;
  cpaneld *cpanel = &dsp->cpanel;
  gint nc = d->ncols;

  alloc_tour2d(dsp, gg);
 
    /* Initialize starting subset of active variables */
  dsp->t2d.nvars = nc;
  for (j=0; j<nc; j++)
    dsp->t2d.vars.els[j] = j;

  /* declare starting base as first p chosen variables */
  for (i=0; i<2; i++)
    for (j=0; j<nc; j++)
      dsp->t2d.u0.vals[i][j] = dsp->t2d.u1.vals[i][j] = 
        dsp->t2d.u.vals[i][j] = dsp->t2d.v0.vals[i][j] = 
        dsp->t2d.v1.vals[i][j] = 0.0;

  for (i=0; i<2; i++)
  {
    dsp->t2d.u1.vals[i][dsp->t2d.vars.els[i]] =
      dsp->t2d.u0.vals[i][dsp->t2d.vars.els[i]] = 
      dsp->t2d.u.vals[i][dsp->t2d.vars.els[i]] =
      dsp->t2d.v0.vals[i][dsp->t2d.vars.els[i]] = 
      dsp->t2d.v1.vals[i][dsp->t2d.vars.els[i]] = 1.0;
  }

  dsp->t2d.dv = 1.0;
  dsp->t2d.delta = cpanel->t2d_step*M_PI_2/10.0;
  dsp->t2d.nsteps = 1; 
  dsp->t2d.stepcntr = 1;

  dsp->t2d.idled = 0;
  dsp->t2d.get_new_target = true;

  /* manip */
  dsp->t2d_manip_mode = MANIP_OFF;
  dsp->t2d_manip_var = 0;

  /* pp */
  dsp->t2d.target_basis_method = 0;
  dsp->t2d_ppda = NULL;
  dsp->t2d_axes = true;
}

void tour2d_speed_set(gint slidepos, ggobid *gg) {
  displayd *dsp = gg->current_display; 
  cpaneld *cpanel = &dsp->cpanel;
  extern void speed_set (gint, gfloat *, gfloat *, gfloat, gint *, gint *);

  speed_set(slidepos, &cpanel->t2d_step, &dsp->t2d.delta,  dsp->t2d.dv,
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

  for (j=0; j<dsp->t2d.nvars; j++)
    if (jvar == dsp->t2d.vars.els[j])
      active = true;

  /* deselect var if t2d.nvars > 2 */
  if (active) {
    if (dsp->t2d.nvars > 2) {
      for (j=0; j<dsp->t2d.nvars; j++) {
        if (jvar == dsp->t2d.vars.els[j]) 
          break;
      }
      if (j<dsp->t2d.nvars-1) {
        for (k=j; k<dsp->t2d.nvars-1; k++) {
          dsp->t2d.vars.els[k] = dsp->t2d.vars.els[k+1];
        }
      }
      dsp->t2d.nvars--;
    }
  }
  else { /* not active, so add the variable */
    if (jvar > dsp->t2d.vars.els[dsp->t2d.nvars-1]) {
      dsp->t2d.vars.els[dsp->t2d.nvars] = jvar;
    }
    else if (jvar < dsp->t2d.vars.els[0]) {
      for (j=dsp->t2d.nvars; j>0; j--) {
          dsp->t2d.vars.els[j] = dsp->t2d.vars.els[j-1];
      }
      dsp->t2d.vars.els[0] = jvar;
    }
    else {
      for (j=0; j<dsp->t2d.nvars-1; j++) {
        if (jvar > dsp->t2d.vars.els[j] && jvar < dsp->t2d.vars.els[j+1]) {
          jtmp = j+1;
          break;
        }
      }
      for (j=dsp->t2d.nvars-1;j>=jtmp; j--) 
          dsp->t2d.vars.els[j+1] = dsp->t2d.vars.els[j];
      dsp->t2d.vars.els[jtmp] = jvar;
    }
    dsp->t2d.nvars++;
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
  if (button == 1 || button == 2) {

    if (d->vcirc_ui.jcursor == GDK_HAND2) {
      tour2d_manip_var_set (jvar, gg);
      varcircles_cursor_set_default (d);

    } else {
      tour2dvar_set (jvar, gg);
    }
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
      sp->planar[i].x += (gint)(dsp->t2d.u.vals[0][j]*world_data[i][j]);
      sp->planar[i].y += (gint)(dsp->t2d.u.vals[1][j]*world_data[i][j]);
    }
  }
}

void
tour2d_run(displayd *dsp, ggobid *gg)
{
  extern gboolean reached_target(gint, gint, gint, gfloat *, gfloat *);
  extern void increment_tour(vector_f, vector_f, gint *, gint *, gfloat, 
    gfloat, gint);
  extern void do_last_increment(vector_f, vector_f, gint);
  extern void gt_basis(array_f, gint, vector_i, gint, gint);
  extern void path(array_f, array_f, array_f, gint, gint, array_f, 
    array_f, array_f, vector_f, array_f, array_f,
    vector_f, vector_f, gint *, gint *, gfloat *, gfloat);
  extern void tour_reproject(vector_f, array_f, array_f, array_f, 
    array_f, array_f, gint, gint);
  extern void t2d_ppdraw(gfloat, ggobid *);
  datad *d = dsp->d;
  cpaneld *cpanel = &dsp->cpanel;
  gint i, j, nv;
  static gint count = 0;
  gboolean revert_random = false;
  static gfloat oindxval = -999.0;

  if (!dsp->t2d.get_new_target && 
       !reached_target(dsp->t2d.nsteps, dsp->t2d.stepcntr, 
         dsp->t2d.target_basis_method, &dsp->t2d.ppval, &oindxval)) {
    increment_tour(dsp->t2d.tinc, dsp->t2d.tau, &dsp->t2d.nsteps, 
      &dsp->t2d.stepcntr, dsp->t2d.dv, dsp->t2d.delta, (gint) 2);
    tour_reproject(dsp->t2d.tinc, dsp->t2d.v, dsp->t2d.v0, dsp->t2d.v1, 
      dsp->t2d.u, dsp->t2d.uvevec, d->ncols, (gint) 2);

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
      if (dsp->t2d.target_basis_method == 1)
      {
        dsp->t2d_pp_op.index_best = dsp->t2d.ppval;
        oindxval = dsp->t2d.ppval;
        for (i=0; i<2; i++)
          for (j=0; j<dsp->t2d.nvars; j++)
            dsp->t2d_pp_op.proj_best.vals[j][i] = 
              dsp->t2d.u.vals[i][dsp->t2d.vars.els[j]];
      }
    }
    else 
    {
      if (dsp->t2d.target_basis_method == 1)
        t2d_ppdraw(dsp->t2d.ppval, gg);
      else
      {
        do_last_increment(dsp->t2d.tinc, dsp->t2d.tau, (gint) 2);
        tour_reproject(dsp->t2d.tinc, dsp->t2d.v, dsp->t2d.v0, dsp->t2d.v1,
          dsp->t2d.u, dsp->t2d.uvevec, d->ncols, (gint) 2);
      }
    }
    copy_mat(dsp->t2d.u0.vals, dsp->t2d.u.vals, d->ncols, 2);
    nv = 0;
    for (i=0; i<d->ncols; i++)
      if (fabs(dsp->t2d.u0.vals[0][i]) > 0.01 || 
          fabs(dsp->t2d.u0.vals[1][i]) > 0.01) {
        nv++;
      }
    if (nv <= 2 && dsp->t2d.nvars <= 2) /* only generate new dir if num of
                                           active/used variables is > 2 */
      dsp->t2d.get_new_target = true;
    else {
      if (dsp->t2d.target_basis_method == 0) {
        gt_basis(dsp->t2d.u1, dsp->t2d.nvars, dsp->t2d.vars, 
          d->ncols, (gint) 2);
      }
      else if (dsp->t2d.target_basis_method == 1) {
        /* pp guided tour  */
        revert_random = t2d_switch_index(cpanel->t2d_pp_indx, 
          dsp->t2d.target_basis_method, gg);

        if (!revert_random) {
          for (i=0; i<2; i++)
            for (j=0; j<dsp->t2d.nvars; j++)
              dsp->t2d.u1.vals[i][dsp->t2d.vars.els[j]] = 
                dsp->t2d_pp_op.proj_best.vals[j][i];

          /* if the best projection is the same as the previous one, switch 
              to a random projection */
          if (!checkequiv(dsp->t2d.u0.vals, dsp->t2d.u1.vals, d->ncols, 2)) 
          {
            printf("Using random projection\n");
            gt_basis(dsp->t2d.u1, dsp->t2d.nvars, dsp->t2d.vars, 
              d->ncols, (gint) 2);
            for (i=0; i<2; i++)
              for (j=0; j<dsp->t2d.nvars; j++)
                dsp->t2d_pp_op.proj_best.vals[j][i] = 
                  dsp->t2d.u1.vals[i][dsp->t2d.vars.els[j]];
            revert_random = t2d_switch_index(cpanel->t2d_pp_indx, 
              dsp->t2d.target_basis_method, gg);
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
          gt_basis(dsp->t2d.u1, dsp->t2d.nvars, dsp->t2d.vars, 
            d->ncols, (gint) 2);
        }
        
      }
      path(dsp->t2d.u0, dsp->t2d.u1, dsp->t2d.u, d->ncols, (gint) 2, dsp->t2d.v0,
      dsp->t2d.v1, dsp->t2d.v, dsp->t2d.lambda, dsp->t2d.tv, dsp->t2d.uvevec,
      dsp->t2d.tau, dsp->t2d.tinc, &dsp->t2d.nsteps, &dsp->t2d.stepcntr, 
      &dsp->t2d.dv, dsp->t2d.delta);
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
    dsp->t2d.idled = gtk_idle_add_priority (G_PRIORITY_LOW,
                                   (GtkFunction) tour2d_idle_func, dsp);
    gg->tour2d.idled = 1;
  } else {
    if (dsp->t2d.idled)
      gtk_idle_remove (dsp->t2d.idled);
    gg->tour2d.idled = 0;
  }

/*
   if (state)
     tour_idle = gtk_timeout_add (40, tour_idle_func, NULL);
   else
     gtk_timeout_remove (tour_idle);
*/
}

void tour2d_reinit(ggobid *gg)
{
  int i, j;
  displayd *dsp = gg->current_display;
  datad *d = dsp->d;

  for (i=0; i<2; i++) {
    for (j=0; j<d->ncols; j++) {
      dsp->t2d.u0.vals[i][j] = 0.;
      dsp->t2d.u.vals[i][j] = 0.;
    }
    dsp->t2d.u0.vals[i][dsp->t2d.vars.els[i]] = 1.;
    dsp->t2d.u.vals[i][dsp->t2d.vars.els[i]] = 1.;
  }

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
  gint n1vars = dsp->t2d.nvars;
  gfloat ftmp, tol = 0.01; 
  gdouble dtmp1;
  gboolean dontdoit = false;
  extern void gram_schmidt(gfloat *, gfloat*, gint);
  extern gfloat calc_norm(gfloat *, gint);

  /* need to turn off tour */
  if (!cpanel->t2d_paused)
    tour2d_func(T2DOFF, gg->current_display, gg);

  dsp->t2d_manipvar_inc = false;
  dsp->t2d_pos1 = dsp->t2d_pos1_old = p1;
  dsp->t2d_pos2 = dsp->t2d_pos2_old = p2;
  /* check if manip var is one of existing vars */
  /* n1vars, n2vars is the number of variables, excluding the
     manip var in hor and vert directions */
  for (j=0; j<dsp->t2d.nvars; j++)
    if (dsp->t2d.vars.els[j] == dsp->t2d_manip_var) {
      dsp->t2d_manipvar_inc = true;
      n1vars--;
    }

  if (n1vars > 1)
  {
    /* make manip basis, from existing projection */
    /* 0,1 will be the remainder of the projection, and
       2 will be the indicator vector for the manip var */
    for (j=0; j<d->ncols; j++) 
    {
      dsp->t2d_manbasis.vals[0][j] = dsp->t2d.u.vals[0][j];
      dsp->t2d_manbasis.vals[1][j] = dsp->t2d.u.vals[1][j];
      dsp->t2d_manbasis.vals[2][j] = 0.;
    }
    dsp->t2d_manbasis.vals[2][dsp->t2d_manip_var] = 1.;

    for (j=0; j<3; j++)
    {
      for (k=0; k<3; k++)
        dsp->t2d_mvar_3dbasis.vals[j][k] = 0.;
      dsp->t2d_mvar_3dbasis.vals[j][j] = 1.;
    }

    gram_schmidt(dsp->t2d_manbasis.vals[0],  dsp->t2d_manbasis.vals[2],
      d->ncols);
    gram_schmidt(dsp->t2d_manbasis.vals[1],  dsp->t2d_manbasis.vals[2],
      d->ncols);
    ftmp = calc_norm (dsp->t2d_manbasis.vals[2], d->ncols);
    if (ftmp < tol)
      dontdoit = true;

    dsp->t2d_no_dir_flag = false;
    if (dsp->t2d_manip_mode == MANIP_RADIAL)
    {
      if ((dsp->t2d.u.vals[0][dsp->t2d_manip_var]*
        dsp->t2d.u.vals[0][dsp->t2d_manip_var] +
        dsp->t2d.u.vals[1][dsp->t2d_manip_var]*
        dsp->t2d.u.vals[1][dsp->t2d_manip_var]) < tol)
        dsp->t2d_no_dir_flag = true;
      else
      {
        dsp->t2d_rx = dsp->t2d.u.vals[0][dsp->t2d_manip_var];
        dsp->t2d_ry = dsp->t2d.u.vals[1][dsp->t2d_manip_var];
        dtmp1 = sqrt(dsp->t2d_rx*dsp->t2d_rx+dsp->t2d_ry*dsp->t2d_ry);
        dsp->t2d_rx /= dtmp1;
        dsp->t2d_ry /= dtmp1;
      }
    }
  }

  if (dontdoit)
    disconnect_motion_signal (sp);
}

void
tour2d_manip(gint p1, gint p2, splotd *sp, ggobid *gg) 
{
  displayd *dsp = (displayd *) sp->displayptr;
  datad *d = dsp->d;
  gint actual_nvars = dsp->t2d.nvars;
  gboolean offscreen = false;
  gfloat phi, cosphi, sinphi, ca, sa, cosm, cospsi, sinpsi;
  gfloat distx, disty, x1, x2, y1, y2;
  gfloat denom = (float) MIN(sp->max.x, sp->max.y)/2.;
  gfloat tol = 0.01;
  gdouble dtmp1, dtmp2;
  gfloat len_motion;
  gint i,j,k;
  extern void gram_schmidt(gfloat *, gfloat*, gint);
  extern void copy_mat(gfloat **, gfloat **, gint, gint);

  /* check if off the plot window */
  if (p1 > sp->max.x || p1 < 0 ||
      p2 > sp->max.y || p2 < 0)
    offscreen = true;

  if (dsp->t2d_manipvar_inc)
    actual_nvars = dsp->t2d.nvars-1;

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
          disty = dsp->t2d_pos2 - dsp->t2d_pos2_old;
        }
        else if (dsp->t2d_manip_mode == MANIP_VERT) 
        {
          distx = 0.;
          disty = dsp->t2d_pos2 - dsp->t2d_pos2_old;
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
            disty = dsp->t2d_pos2 - dsp->t2d_pos2_old;
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
      copy_mat(dsp->t2d_mvar_3dbasis.vals, dsp->t2d_Rmat1.vals, 3, 3);
  
      gram_schmidt(dsp->t2d_mvar_3dbasis.vals[0], 
        dsp->t2d_mvar_3dbasis.vals[1], 3);
      gram_schmidt(dsp->t2d_mvar_3dbasis.vals[0], 
        dsp->t2d_mvar_3dbasis.vals[2], 3);
      gram_schmidt(dsp->t2d_mvar_3dbasis.vals[1], 
        dsp->t2d_mvar_3dbasis.vals[2], 3);

      for (j=0; j<d->ncols; j++)
      {
        dsp->t2d.u.vals[0][j] = 
          dsp->t2d_manbasis.vals[0][j]*dsp->t2d_mvar_3dbasis.vals[0][0] +
          dsp->t2d_manbasis.vals[1][j]*dsp->t2d_mvar_3dbasis.vals[0][1] +
          dsp->t2d_manbasis.vals[2][j]*dsp->t2d_mvar_3dbasis.vals[0][2];
        dsp->t2d.u.vals[1][j] = 
          dsp->t2d_manbasis.vals[0][j]*dsp->t2d_mvar_3dbasis.vals[1][0] +
          dsp->t2d_manbasis.vals[1][j]*dsp->t2d_mvar_3dbasis.vals[1][1] +
          dsp->t2d_manbasis.vals[2][j]*dsp->t2d_mvar_3dbasis.vals[1][2];
      }
    }
    /*display_tailpipe (dsp, FULL, gg);*/
    display_tailpipe (dsp, FULL_1PIXMAP, gg);
    varcircles_refresh (d, gg);
  }
}

void
tour2d_manip_end(splotd *sp) 
{
  displayd *dsp = (displayd *) sp->displayptr;
  datad *d = dsp->d;
  cpaneld *cpanel = &dsp->cpanel;
  ggobid *gg = GGobiFromSPlot(sp);
  extern void copy_mat(gfloat **, gfloat **, gint, gint);

  disconnect_motion_signal (sp);

  copy_mat(dsp->t2d.u0.vals, dsp->t2d.u.vals, d->ncols, 2);
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
