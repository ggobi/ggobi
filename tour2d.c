/* tour2d.c */

#include <gtk/gtk.h>
#ifdef USE_STRINGS_H
#include <strings.h>
#endif

#include <math.h>

#include "vars.h"
#include "externs.h"

void
alloc_tour2d (displayd *dsp, ggobid *gg)
{
  datad *d = dsp->d;
  gint nc = d->ncols;

  /* first index is the projection dimensions, second dimension is ncols */
  arrayf_null(&dsp->t2d.u0);
  arrayf_alloc(&dsp->t2d.u0, 2, nc);

  arrayf_null(&dsp->t2d.u1);
  arrayf_alloc(&dsp->t2d.u1, 2, nc);

  arrayf_null(&dsp->t2d.u);
  arrayf_alloc(&dsp->t2d.u, 2, nc);

  arrayf_null(&dsp->t2d.v0);
  arrayf_alloc(&dsp->t2d.v0, 2, nc);

  arrayf_null(&dsp->t2d.v1);
  arrayf_alloc(&dsp->t2d.v1, 2, nc);

  arrayf_null(&dsp->t2d.v);
  arrayf_alloc(&dsp->t2d.v, 2, nc);

  arrayf_null(&dsp->t2d.uvevec);
  arrayf_alloc(&dsp->t2d.uvevec, 2, nc);

  arrayf_null(&dsp->t2d.tv);
  arrayf_alloc(&dsp->t2d.tv, 2, nc);

  vectori_null(&dsp->t2d.vars);
  vectori_alloc(&dsp->t2d.vars, nc);
  vectorf_null(&dsp->t2d.lambda);
  vectorf_alloc(&dsp->t2d.lambda, nc);
  vectorf_null(&dsp->t2d.tau);
  vectorf_alloc(&dsp->t2d.tau, nc);
  vectorf_null(&dsp->t2d.tinc);
  vectorf_alloc(&dsp->t2d.tinc, nc);

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
    }
  }
}

/*-- append columns for a total of nc columns --*/
void
tour2d_realloc_up (gint nc, datad *d, ggobid *gg)
{
  displayd *dsp;
  GList *l;
  for (l=gg->displays; l; l=l->next) {
    dsp = (displayd *) l->data;
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
  /*  dsp->t2d.vars.els[0] = 0;
  dsp->t2d.vars.els[1] = 1;
  dsp->t2d.vars.els[2] = 2;*/

  /* declare starting base as first p chosen variables */
  for (i=0; i<2; i++)
    for (j=0; j<nc; j++)
      dsp->t2d.u0.vals[i][j] = dsp->t2d.u1.vals[i][j] = dsp->t2d.u.vals[i][j] = 
        dsp->t2d.v0.vals[i][j] = dsp->t2d.v1.vals[i][j] = 0.0;

  for (i=0; i<2; i++)
  {
    dsp->t2d.u1.vals[i][dsp->t2d.vars.els[i]] =
      dsp->t2d.u0.vals[i][dsp->t2d.vars.els[i]] = 
      dsp->t2d.u.vals[i][dsp->t2d.vars.els[i]] =
      dsp->t2d.v0.vals[i][dsp->t2d.vars.els[i]] = 
      dsp->t2d.v1.vals[i][dsp->t2d.vars.els[i]] = 1.0;
  }

  /*  dsp->ts[0] = 0;
  dsp->ts[1] = M_PI_2;
  dsp->coss[0] = 1.0;
  dsp->coss[1] = 0.0;
  dsp->sins[1] = 1.0;
  dsp->sins[0] = 0.0;
  dsp->icoss[0] = PRECISION2;
  dsp->icoss[1] = 0;
  dsp->isins[1] = PRECISION2;
  dsp->isins[0] = 0;*/

  /*  zero_tau(dsp, gg);*/
  dsp->t2d.dv = 1.0;
  dsp->t2d.delta = cpanel->t2d_step*M_PI_2/10.0;
  dsp->t2d.nsteps = 1; 
  dsp->t2d.stepcntr = 1;

  dsp->t2d.idled = 0;
  dsp->t2d.get_new_target = true;

  /* pp */
  dsp->t2d.target_basis_method = 0;
}

void tour2d_speed_set(gint slidepos, ggobid *gg) {
  displayd *dsp = gg->current_display; 
  cpaneld *cpanel = &dsp->cpanel;
  extern void speed_set (gint, gfloat *, gfloat *, gfloat, gint *, gint *);

  speed_set(slidepos, &cpanel->t2d_step, &dsp->t2d.delta,  dsp->t2d.dv,
    &dsp->t2d.nsteps, &dsp->t2d.stepcntr);
}

void 
tour2dvar_set (gint jvar, ggobid *gg)
{
  gint j, jtmp, k;
  gboolean selected=false;
  displayd *dsp = gg->current_display;
  /*  extern void zero_tau(displayd *, ggobid *);
  extern void zero_tinc(displayd *, ggobid *);
  extern void init_basis(displayd *, ggobid *);*/

  for (j=0; j<dsp->t2d.nvars; j++)
    if (jvar == dsp->t2d.vars.els[j])
      selected = true;

  /* deselect var if t2d.nvars > 2 */
  if (selected) {
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
  else { /* not selected, so add the variable */
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
  g_printerr ("set the manipulation variable; not yet implemented\n");
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
  int i, j, m;
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
  datad *d = dsp->d;
  gint i, nv;

  if (!dsp->t2d.get_new_target && 
       !reached_target(dsp->t2d.nsteps, dsp->t2d.stepcntr)) {
    increment_tour(dsp->t2d.tinc, dsp->t2d.tau, &dsp->t2d.nsteps, 
      &dsp->t2d.stepcntr, dsp->t2d.dv, dsp->t2d.delta, (gint) 2);
    tour_reproject(dsp->t2d.tinc, dsp->t2d.v, dsp->t2d.v0, dsp->t2d.v1, 
      dsp->t2d.u, dsp->t2d.uvevec, d->ncols, (gint) 2);
  }
  else { /* do final clean-up and get new target */
    if (!dsp->t2d.get_new_target) {
      do_last_increment(dsp->t2d.tinc, dsp->t2d.tau, (gint) 2);
      tour_reproject(dsp->t2d.tinc, dsp->t2d.v, dsp->t2d.v0, dsp->t2d.v1,
	  dsp->t2d.u, dsp->t2d.uvevec, d->ncols, (gint) 2);
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
      gt_basis(dsp->t2d.u1, dsp->t2d.nvars, dsp->t2d.vars, d->ncols, (gint) 2);
      path(dsp->t2d.u0, dsp->t2d.u1, dsp->t2d.u, d->ncols, (gint) 2, dsp->t2d.v0,
      dsp->t2d.v1, dsp->t2d.v, dsp->t2d.lambda, dsp->t2d.tv, dsp->t2d.uvevec,
      dsp->t2d.tau, dsp->t2d.tinc, &dsp->t2d.nsteps, &dsp->t2d.stepcntr, 
      &dsp->t2d.dv, dsp->t2d.delta);
      dsp->t2d.get_new_target = false;
    }
  }
  
  display_tailpipe (dsp, gg);

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
  int i, j, m;
  displayd *dsp = gg->current_display;

  for (i=0; i<2; i++) {
    for (j=0; j<dsp->t2d.nvars; j++) {
      m = dsp->t2d.vars.els[j];
      dsp->t2d.u0.vals[i][m] = 0.;
      dsp->t2d.u.vals[i][m] = 0.;
    }
    m = dsp->t2d.vars.els[i];
    dsp->t2d.u0.vals[i][m] = 1.;
    dsp->t2d.u.vals[i][m] = 1.;
  }

  dsp->t2d.get_new_target = true;

}




