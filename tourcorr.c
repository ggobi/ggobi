/* tourcorr.c */

#include <gtk/gtk.h>
#ifdef USE_STRINGS_H
#include <strings.h>
#endif

#include <math.h>

#include "vars.h"
#include "externs.h"

void
alloc_tourcorr (displayd *dsp, ggobid *gg)
{
  datad *d = dsp->d;
  gint nc = d->ncols;

  /* first index is the projection dimensions, second dimension is ncols */
  arrayf_init(&dsp->tcorr1.u0);
  arrayf_alloc(&dsp->tcorr1.u0, nc, nc);

  arrayf_init(&dsp->tcorr1.u1);
  arrayf_alloc(&dsp->tcorr1.u1, nc, nc);

  arrayf_init(&dsp->tcorr1.u);
  arrayf_alloc(&dsp->tcorr1.u, nc, nc);

  arrayf_init(&dsp->tcorr1.v0);
  arrayf_alloc(&dsp->tcorr1.v0, nc, nc);

  arrayf_init(&dsp->tcorr1.v1);
  arrayf_alloc(&dsp->tcorr1.v1, nc, nc);

  arrayf_init(&dsp->tcorr1.v);
  arrayf_alloc(&dsp->tcorr1.v, nc, nc);

  arrayf_init(&dsp->tcorr1.uvevec);
  arrayf_alloc(&dsp->tcorr1.uvevec, nc, nc);

  arrayf_init(&dsp->tcorr1.tv);
  arrayf_alloc(&dsp->tcorr1.tv, nc, nc);

  vectori_init(&dsp->tcorr1.vars);
  vectori_alloc(&dsp->tcorr1.vars, nc);
  vectorf_init(&dsp->tcorr1.lambda);
  vectorf_alloc(&dsp->tcorr1.lambda, nc);
  vectorf_init(&dsp->tcorr1.tau);
  vectorf_alloc(&dsp->tcorr1.tau, nc);
  vectorf_init(&dsp->tcorr1.tinc);
  vectorf_alloc(&dsp->tcorr1.tinc, nc);

  /* first index is the projection dimensions, second dimension is ncols */
  arrayf_init(&dsp->tcorr2.u0);
  arrayf_alloc(&dsp->tcorr2.u0, nc, nc);

  arrayf_init(&dsp->tcorr2.u1);
  arrayf_alloc(&dsp->tcorr2.u1, nc, nc);

  arrayf_init(&dsp->tcorr2.u);
  arrayf_alloc(&dsp->tcorr2.u, nc, nc);

  arrayf_init(&dsp->tcorr2.v0);
  arrayf_alloc(&dsp->tcorr2.v0, nc, nc);

  arrayf_init(&dsp->tcorr2.v1);
  arrayf_alloc(&dsp->tcorr2.v1, nc, nc);

  arrayf_init(&dsp->tcorr2.v);
  arrayf_alloc(&dsp->tcorr2.v, nc, nc);

  arrayf_init(&dsp->tcorr2.uvevec);
  arrayf_alloc(&dsp->tcorr2.uvevec, nc, nc);

  arrayf_init(&dsp->tcorr2.tv);
  arrayf_alloc(&dsp->tcorr2.tv, nc, nc);

  vectori_init(&dsp->tcorr2.vars);
  vectori_alloc(&dsp->tcorr2.vars, nc);
  vectorf_init(&dsp->tcorr2.lambda);
  vectorf_alloc(&dsp->tcorr2.lambda, nc);
  vectorf_init(&dsp->tcorr2.tau);
  vectorf_alloc(&dsp->tcorr2.tau, nc);
  vectorf_init(&dsp->tcorr2.tinc);
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
      arrayf_delete_cols (&dsp->tcorr1.u0, nc, cols);
      arrayf_delete_cols (&dsp->tcorr1.u1, nc, cols);
      arrayf_delete_cols (&dsp->tcorr1.u, nc, cols);
      arrayf_delete_cols (&dsp->tcorr1.v0, nc, cols);
      arrayf_delete_cols (&dsp->tcorr1.v1, nc, cols);
      arrayf_delete_cols (&dsp->tcorr1.v, nc, cols);
      arrayf_delete_cols (&dsp->tcorr1.uvevec, nc, cols);
      arrayf_delete_cols (&dsp->tcorr1.tv, nc, cols);

      vectori_delete_els (&dsp->tcorr1.vars, nc, cols);
      vectorf_delete_els (&dsp->tcorr1.lambda, nc, cols);
      vectorf_delete_els (&dsp->tcorr1.tau, nc, cols);
      vectorf_delete_els (&dsp->tcorr1.tinc, nc, cols);

      arrayf_delete_cols (&dsp->tcorr2.u0, nc, cols);
      arrayf_delete_cols (&dsp->tcorr2.u1, nc, cols);
      arrayf_delete_cols (&dsp->tcorr2.u, nc, cols);
      arrayf_delete_cols (&dsp->tcorr2.v0, nc, cols);
      arrayf_delete_cols (&dsp->tcorr2.v1, nc, cols);
      arrayf_delete_cols (&dsp->tcorr2.v, nc, cols);
      arrayf_delete_cols (&dsp->tcorr2.uvevec, nc, cols);
      arrayf_delete_cols (&dsp->tcorr2.tv, nc, cols);

      vectori_delete_els (&dsp->tcorr2.vars, nc, cols);
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
  for (l=gg->displays; l; l=l->next) {
    dsp = (displayd *) l->data;
    if (dsp->d == d) {
      arrayf_add_cols (&dsp->tcorr1.u0, nc);
      arrayf_add_cols (&dsp->tcorr1.u1, nc);
      arrayf_add_cols (&dsp->tcorr1.u, nc);
      arrayf_add_cols (&dsp->tcorr1.v0, nc);
      arrayf_add_cols (&dsp->tcorr1.v1, nc);
      arrayf_add_cols (&dsp->tcorr1.v, nc);
      arrayf_add_cols (&dsp->tcorr1.uvevec, nc);
      arrayf_add_cols (&dsp->tcorr1.tv, nc);

      vectori_realloc (&dsp->tcorr1.vars, nc);
      vectorf_realloc (&dsp->tcorr1.lambda, nc);
      vectorf_realloc (&dsp->tcorr1.tau, nc);
      vectorf_realloc (&dsp->tcorr1.tinc, nc);

      arrayf_add_cols (&dsp->tcorr2.u0, nc);
      arrayf_add_cols (&dsp->tcorr2.u1, nc);
      arrayf_add_cols (&dsp->tcorr2.u, nc);
      arrayf_add_cols (&dsp->tcorr2.v0, nc);
      arrayf_add_cols (&dsp->tcorr2.v1, nc);
      arrayf_add_cols (&dsp->tcorr2.v, nc);
      arrayf_add_cols (&dsp->tcorr2.uvevec, nc);
      arrayf_add_cols (&dsp->tcorr2.tv, nc);

      vectori_realloc (&dsp->tcorr2.vars, nc);
      vectorf_realloc (&dsp->tcorr2.lambda, nc);
      vectorf_realloc (&dsp->tcorr2.tau, nc);
      vectorf_realloc (&dsp->tcorr2.tinc, nc);
    }
  }
}

void
free_tourcorr(displayd *dsp)
{
  /*  gint k;*/
  /*  datad *d = dsp->d;*/
  /*  gint nc = d->ncols;*/

  vectori_free(&dsp->tcorr1.vars);
  vectorf_free(&dsp->tcorr1.lambda);
  vectorf_free(&dsp->tcorr1.tau);
  vectorf_free(&dsp->tcorr1.tinc);

  arrayf_free(&dsp->tcorr1.u0, 0, 0);
  arrayf_free(&dsp->tcorr1.u1, 0, 0);
  arrayf_free(&dsp->tcorr1.u, 0, 0);

  arrayf_free(&dsp->tcorr1.v0, 0, 0);
  arrayf_free(&dsp->tcorr1.v1, 0, 0);
  arrayf_free(&dsp->tcorr1.v, 0, 0);

  arrayf_free(&dsp->tcorr1.uvevec, 0, 0);
  arrayf_free(&dsp->tcorr1.tv, 0, 0);

  vectori_free(&dsp->tcorr2.vars);
  vectorf_free(&dsp->tcorr2.lambda);
  vectorf_free(&dsp->tcorr2.tau);
  vectorf_free(&dsp->tcorr2.tinc);

  arrayf_free(&dsp->tcorr2.u0, 0, 0);
  arrayf_free(&dsp->tcorr2.u1, 0, 0);
  arrayf_free(&dsp->tcorr2.u, 0, 0);

  arrayf_free(&dsp->tcorr2.v0, 0, 0);
  arrayf_free(&dsp->tcorr2.v1, 0, 0);
  arrayf_free(&dsp->tcorr2.v, 0, 0);

  arrayf_free(&dsp->tcorr2.uvevec, 0, 0);
  arrayf_free(&dsp->tcorr2.tv, 0, 0);

}

void 
display_tourcorr_init (displayd *dsp, ggobid *gg) {
  gint i, j;
  datad *d = dsp->d;
  cpaneld *cpanel = &dsp->cpanel;
  gint nc = d->ncols;

  alloc_tourcorr(dsp, gg);
 
  /* Initialize first variable as the vertical and rest of the variables as
     the horizontal variables */
  dsp->tcorr1.nvars = 2;
  dsp->tcorr1.vars.els[0] = 0;
  dsp->tcorr1.vars.els[1] = 1;

  dsp->tcorr2.nvars = nc-2;
  for (j=0; j<nc-2; j++)
    dsp->tcorr2.vars.els[j] = j+2;

  /* declare starting vertical base as first variable */
  for (i=0; i<nc; i++)
    for (j=0; j<nc; j++)
      dsp->tcorr1.u0.vals[i][j] = dsp->tcorr1.u1.vals[i][j] = 
        dsp->tcorr1.u.vals[i][j] = 
        dsp->tcorr1.v0.vals[i][j] = dsp->tcorr1.v1.vals[i][j] = 0.0;

  dsp->tcorr1.u.vals[0][dsp->tcorr1.vars.els[0]] = 1.0;

  for (i=0; i<nc; i++)
    for (j=0; j<nc; j++)
      dsp->tcorr2.u0.vals[i][j] = dsp->tcorr2.u1.vals[i][j] = 
        dsp->tcorr2.u.vals[i][j] = 
        dsp->tcorr2.v0.vals[i][j] = dsp->tcorr2.v1.vals[i][j] = 0.0;

  dsp->tcorr2.u.vals[0][dsp->tcorr2.vars.els[0]] = 1.0;

  dsp->tcorr1.dv = 1.0;
  dsp->tcorr1.delta = cpanel->tcorr1_step*M_PI_2/10.0;
  dsp->tcorr1.nsteps = 1; 
  dsp->tcorr1.stepcntr = 1;

  dsp->tcorr1.idled = 0;
  dsp->tcorr1.get_new_target = true;

  dsp->tcorr2.dv = 1.0;
  dsp->tcorr2.delta = cpanel->tcorr2_step*M_PI_2/10.0;
  dsp->tcorr2.nsteps = 1; 
  dsp->tcorr2.stepcntr = 1;

  dsp->tcorr2.idled = 0;
  dsp->tcorr2.get_new_target = true;
}

void tourcorr_speed_set(gint slidepos, ggobid *gg) {
  displayd *dsp = gg->current_display; 
  cpaneld *cpanel = &dsp->cpanel;
  extern void speed_set (gint, gfloat *, gfloat *, gfloat, gint *, gint *);

  speed_set(slidepos, &cpanel->tcorr1_step, &dsp->tcorr1.delta,  
    dsp->tcorr1.dv, &dsp->tcorr1.nsteps, &dsp->tcorr1.stepcntr);

  speed_set(slidepos, &cpanel->tcorr2_step, &dsp->tcorr2.delta,  
    dsp->tcorr2.dv, &dsp->tcorr2.nsteps, &dsp->tcorr2.stepcntr);
}

void 
set_tourcorr_horvar(ggobid *gg, gint jvar)
{
  gint j, jtmp, k;
  gboolean active=false;
  displayd *dsp = gg->current_display;

  for (j=0; j<dsp->tcorr1.nvars; j++)
    if (jvar == dsp->tcorr1.vars.els[j])
      active = true;

  /* deselect var if tcorr1.nvars > 1 */
  if (active) {
    if (dsp->tcorr1.nvars > 1) {
      for (j=0; j<dsp->tcorr1.nvars; j++) {
        if (jvar == dsp->tcorr1.vars.els[j]) 
          break;
      }
      if (j<dsp->tcorr1.nvars-1) {
        for (k=j; k<dsp->tcorr1.nvars-1; k++){
          dsp->tcorr1.vars.els[k] = dsp->tcorr1.vars.els[k+1];
	}
      }
      dsp->tcorr1.nvars--;
      printf("active %d: ",dsp->tcorr1.nvars);
      for (j=0; j<dsp->tcorr1.nvars; j++)
        printf("%d ",dsp->tcorr1.vars.els[j]);
      printf("\n");
    }
  }
  else { /* not active, so add the variable */
    if (jvar > dsp->tcorr1.vars.els[dsp->tcorr1.nvars-1]) {
      dsp->tcorr1.vars.els[dsp->tcorr1.nvars] = jvar;
    }
    else if (jvar < dsp->tcorr1.vars.els[0]) {
      for (j=dsp->tcorr1.nvars; j>0; j--) {
          dsp->tcorr1.vars.els[j] = dsp->tcorr1.vars.els[j-1];
      }
      dsp->tcorr1.vars.els[0] = jvar;
    }
    else {
      for (j=0; j<dsp->tcorr1.nvars-1; j++) {
        if (jvar > dsp->tcorr1.vars.els[j] && jvar < 
          dsp->tcorr1.vars.els[j+1]) {
          jtmp = j+1;
          break;
	}
      }
      for (j=dsp->tcorr1.nvars-1;j>=jtmp; j--) 
          dsp->tcorr1.vars.els[j+1] = dsp->tcorr1.vars.els[j];
      dsp->tcorr1.vars.els[jtmp] = jvar;
    }
    dsp->tcorr1.nvars++;
    printf("not active %d: ",dsp->tcorr1.nvars);
    for (j=0; j<dsp->tcorr1.nvars; j++)
      printf("%d ",dsp->tcorr1.vars.els[j]);
    printf("\n");
  }

  dsp->tcorr1.get_new_target = true;

}

void 
set_tourcorr_vervar(ggobid *gg, gint jvar)
{
  gint j, jtmp, k;
  gboolean active=false;
  displayd *dsp = gg->current_display;

  for (j=0; j<dsp->tcorr2.nvars; j++)
    if (jvar == dsp->tcorr2.vars.els[j])
      active = true;

  /* deselect var if tcorr2.nvars > 1 */
  if (active) {
    if (dsp->tcorr2.nvars > 1) {
      for (j=0; j<dsp->tcorr2.nvars; j++) {
        if (jvar == dsp->tcorr2.vars.els[j]) 
          break;
      }
      if (j<dsp->tcorr2.nvars-1) {
        for (k=j; k<dsp->tcorr2.nvars-1; k++){
          dsp->tcorr2.vars.els[k] = dsp->tcorr2.vars.els[k+1];
	}
      }
      dsp->tcorr2.nvars--;
    }
  }
  else { /* not active, so add the variable */
    if (jvar > dsp->tcorr2.vars.els[dsp->tcorr2.nvars-1]) {
      dsp->tcorr2.vars.els[dsp->tcorr2.nvars] = jvar;
    }
    else if (jvar < dsp->tcorr2.vars.els[0]) {
      for (j=dsp->tcorr2.nvars; j>0; j--) {
          dsp->tcorr2.vars.els[j] = dsp->tcorr2.vars.els[j-1];
      }
      dsp->tcorr2.vars.els[0] = jvar;
    }
    else {
      for (j=0; j<dsp->tcorr2.nvars-1; j++) {
        if (jvar > dsp->tcorr2.vars.els[j] && jvar < dsp->tcorr2.vars.els[j+1]) {
          jtmp = j+1;
          break;
	}
      }
      for (j=dsp->tcorr2.nvars-1;j>=jtmp; j--) 
          dsp->tcorr2.vars.els[j+1] = dsp->tcorr2.vars.els[j];
      dsp->tcorr2.vars.els[jtmp] = jvar;
    }
    dsp->tcorr2.nvars++;
  }

  dsp->tcorr2.get_new_target = true;
}

void
tourcorr_varsel (ggobid *gg, gint jvar, gint button)
{
  if (button == 1) { 
    set_tourcorr_horvar(gg, jvar);
  }
  else if ( button == 2) {
    set_tourcorr_vervar(gg, jvar);
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
      sp->planar[i].x += (gint)(dsp->tcorr1.u.vals[0][j]*world_data[i][j]);
      sp->planar[i].y += (gint)(dsp->tcorr2.u.vals[0][j]*world_data[i][j]);
    }
  }
}

void
tourcorr_run(displayd *dsp, ggobid *gg)
{
  datad *d = dsp->d;
  gint i, nc = d->ncols;
  extern gboolean reached_target(gint, gint);
  extern void increment_tour(vector_f, vector_f, gint *, gint *, gfloat, 
    gfloat, gint);
  extern void do_last_increment(vector_f, vector_f, gint);
  extern void gt_basis(array_f, gint, vector_i, gint, gint);
  extern void path(array_f, array_f, array_f, gint, gint, array_f, 
    array_f, array_f, vector_f, array_f, array_f,
    vector_f, vector_f, gint *, gint *, gfloat *, gfloat);
  extern void tour_reproject(vector_f, array_f, array_f, array_f, 
    array_f, array_f, gint, gint);
  extern void copy_mat(gfloat **, gfloat **, gint, gint);

  if (!dsp->tcorr1.get_new_target && 
      !reached_target(dsp->tcorr1.nsteps, dsp->tcorr1.stepcntr)) {

    /*  if (!dsp->tcorr1.get_new_target && !dsp->tcorr2.get_new_target && 
       (!reached_target(dsp->tcorr1.nsteps, dsp->tcorr1.stepcntr) ||
       !reached_target(dsp->tcorr2.nsteps, dsp->tcorr2.stepcntr))) {*/

    increment_tour(dsp->tcorr1.tinc, dsp->tcorr1.tau, &dsp->tcorr1.nsteps, 
      &dsp->tcorr1.stepcntr, dsp->tcorr1.dv, dsp->tcorr1.delta, (gint) 1);

    tour_reproject(dsp->tcorr1.tinc, dsp->tcorr1.v, dsp->tcorr1.v0, 
      dsp->tcorr1.v1, dsp->tcorr1.u, dsp->tcorr1.uvevec, d->ncols, (gint) 1);
  }
  else { /* do final clean-up and get new target */
    if (!dsp->tcorr1.get_new_target) {
      do_last_increment(dsp->tcorr1.tinc, dsp->tcorr1.tau, (gint) 1);
      tour_reproject(dsp->tcorr1.tinc, dsp->tcorr1.v, dsp->tcorr1.v0, 
        dsp->tcorr1.v1, dsp->tcorr1.u, dsp->tcorr1.uvevec, d->ncols, (gint) 1);
      }
    
    copy_mat(dsp->tcorr1.u0.vals, dsp->tcorr1.u.vals, d->ncols, 1);

    gt_basis(dsp->tcorr1.u1, dsp->tcorr1.nvars, dsp->tcorr1.vars, d->ncols, 
      (gint) 1);
    path(dsp->tcorr1.u0, dsp->tcorr1.u1, dsp->tcorr1.u, d->ncols, (gint) 1, 
       dsp->tcorr1.v0, dsp->tcorr1.v1, dsp->tcorr1.v, dsp->tcorr1.lambda, 
       dsp->tcorr1.tv, dsp->tcorr1.uvevec,
       dsp->tcorr1.tau, dsp->tcorr1.tinc, &dsp->tcorr1.nsteps, 
       &dsp->tcorr1.stepcntr, 
       &dsp->tcorr1.dv, dsp->tcorr1.delta);

    dsp->tcorr1.get_new_target = false;
  }

  if (!dsp->tcorr2.get_new_target && 
      !reached_target(dsp->tcorr2.nsteps, dsp->tcorr2.stepcntr)) {
    increment_tour(dsp->tcorr2.tinc, dsp->tcorr2.tau, &dsp->tcorr2.nsteps, 
      &dsp->tcorr2.stepcntr, dsp->tcorr2.dv, dsp->tcorr2.delta, (gint) 1);

    tour_reproject(dsp->tcorr2.tinc, dsp->tcorr2.v, dsp->tcorr2.v0, 
      dsp->tcorr2.v1, dsp->tcorr2.u, dsp->tcorr2.uvevec, d->ncols, (gint) 1);
  }
  else { /* do final clean-up and get new target */
    if (!dsp->tcorr2.get_new_target) {
      do_last_increment(dsp->tcorr2.tinc, dsp->tcorr2.tau, (gint) 1);
      tour_reproject(dsp->tcorr2.tinc, dsp->tcorr2.v, dsp->tcorr2.v0, 
        dsp->tcorr2.v1, dsp->tcorr2.u, dsp->tcorr2.uvevec, d->ncols, (gint) 1);
    }
    copy_mat(dsp->tcorr2.u0.vals, dsp->tcorr2.u.vals, d->ncols, 1);

    gt_basis(dsp->tcorr2.u1, dsp->tcorr2.nvars, dsp->tcorr2.vars, d->ncols, 
      (gint) 1);
    path(dsp->tcorr2.u0, dsp->tcorr2.u1, dsp->tcorr2.u, d->ncols, (gint) 1, 
      dsp->tcorr2.v0, dsp->tcorr2.v1, dsp->tcorr2.v, dsp->tcorr2.lambda, 
      dsp->tcorr2.tv, dsp->tcorr2.uvevec,
      dsp->tcorr2.tau, dsp->tcorr2.tinc, &dsp->tcorr2.nsteps, 
      &dsp->tcorr2.stepcntr, &dsp->tcorr2.dv, dsp->tcorr2.delta);

    dsp->tcorr2.get_new_target = false;
  }
  
  display_tailpipe (dsp, gg);

  varcircles_refresh (d, gg);
}

void
tourcorr_do_step(displayd *dsp, ggobid *gg)
{
  tourcorr_run(dsp, gg);
}

gint
tourcorr_idle_func (ggobid *gg)
{
  displayd *dsp = gg->current_display;
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
    dsp->tcorr1.idled = gtk_idle_add_priority (G_PRIORITY_LOW,
                                   (GtkFunction) tourcorr_idle_func, gg);
    gg->tourcorr.idled = 1;
  } else {
    gtk_idle_remove (dsp->tcorr1.idled);
    gg->tourcorr.idled = 0;
  }

/*
   if (state)
     tour_idle = gtk_timeout_add (40, tour_idle_func, NULL);
   else
     gtk_timeout_remove (tour_idle);
*/
}

void tourcorr_reinit(ggobid *gg)
{
  int j, m;
  displayd *dsp = gg->current_display;
  datad *d = dsp->d;
  extern void tour_reproject(vector_f, array_f, array_f, array_f, 
    array_f, array_f, gint, gint);
  extern void zero_tinc(vector_f, gint);
  extern void zero_tau(vector_f, gint);
  extern void zero_lambda(vector_f, gint);

  printf(" in reinit\n");
  for (j=0; j<d->ncols; j++) {
    /*    m = dsp->tcorr1.vars.els[j];*/
    /*    dsp->tcorr1.u0.vals[0][j] = 0.;*/
    dsp->tcorr1.u.vals[0][j] = 0.;
    dsp->tcorr1.v0.vals[0][j] = 0.;
    dsp->tcorr1.u0.vals[0][j] = 0.;
    dsp->tcorr1.v1.vals[0][j] = 0.;
    dsp->tcorr1.u1.vals[0][j] = 0.;
  }
  m = dsp->tcorr1.vars.els[0];
  /*  dsp->tcorr1.u0.vals[0][m] = 1.;*/
  dsp->tcorr1.u.vals[0][m] = 1.;
  dsp->tcorr1.u0.vals[0][m] = 1.;
  dsp->tcorr1.v0.vals[0][m] = 1.;

  dsp->tcorr1.get_new_target = true;

  for (j=0; j<d->ncols; j++) {
    dsp->tcorr2.u.vals[0][j] = 0.;
    dsp->tcorr2.u0.vals[0][j] = 0.;
    dsp->tcorr2.v0.vals[0][j] = 0.;
    dsp->tcorr2.u1.vals[0][j] = 0.;
    dsp->tcorr2.v1.vals[0][j] = 0.;
  }
  m = dsp->tcorr2.vars.els[0];
  /*  dsp->tcorr2.u0.vals[0][m] = 1.;*/
  dsp->tcorr2.u.vals[0][m] = 1.;
  dsp->tcorr2.u0.vals[0][m] = 1.;
  dsp->tcorr2.v0.vals[0][m] = 1.;

  dsp->tcorr2.get_new_target = true;

  zero_tinc(dsp->tcorr1.tinc, 1);
  zero_tinc(dsp->tcorr2.tinc, 1);
  zero_tau(dsp->tcorr1.tau, 1);
  zero_tau(dsp->tcorr2.tau, 1);
  zero_lambda(dsp->tcorr1.lambda, 1);
  zero_lambda(dsp->tcorr2.lambda, 1);

  dsp->tcorr1.nsteps = 0;
  dsp->tcorr2.nsteps = 0;
  dsp->tcorr1.stepcntr = 0;
  dsp->tcorr2.stepcntr = 0;

  display_tailpipe (dsp, gg);

  varcircles_refresh (d, gg);

}






