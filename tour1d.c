/* tour1d.c */

#include <gtk/gtk.h>
#ifdef USE_STRINGS_H
#include <strings.h>
#endif

#include <math.h>
#include <malloc.h>
#include <stdlib.h>

#include "vars.h"
#include "externs.h"

#include "tour1d_pp.h"

void
alloc_tour1d (displayd *dsp, ggobid *gg)
{
  datad *d = dsp->d;
  gint nc = d->ncols;

  arrayf_init(&dsp->t1d.u0);
  arrayf_alloc(&dsp->t1d.u0, nc, nc);

  arrayf_init(&dsp->t1d.u1);
  arrayf_alloc(&dsp->t1d.u1, nc, nc);

  arrayf_init(&dsp->t1d.u);
  arrayf_alloc(&dsp->t1d.u, nc, nc);

  /*  arrayf_init(&dsp->t1d.uold);
  arrayf_alloc(&dsp->t1d.uold, nc, nc);*/

  arrayf_init(&dsp->t1d.v0);
  arrayf_alloc(&dsp->t1d.v0, nc, nc);

  arrayf_init(&dsp->t1d.v1);
  arrayf_alloc(&dsp->t1d.v1, nc, nc);

  arrayf_init(&dsp->t1d.v);
  arrayf_alloc(&dsp->t1d.v, nc, nc);

  arrayf_init(&dsp->t1d.uvevec);
  arrayf_alloc(&dsp->t1d.uvevec, nc, nc);

  arrayf_init(&dsp->t1d.tv);
  arrayf_alloc(&dsp->t1d.tv, nc, nc);

  vectori_init(&dsp->t1d.vars);
  vectori_alloc(&dsp->t1d.vars, nc);
  vectorf_init(&dsp->t1d.lambda);
  vectorf_alloc(&dsp->t1d.lambda, nc);
  vectorf_init(&dsp->t1d.tau);
  vectorf_alloc(&dsp->t1d.tau, nc);
  vectorf_init(&dsp->t1d.tinc);
  vectorf_alloc(&dsp->t1d.tinc, nc);
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
    }
  }
}

/*-- append columns for a total of nc columns --*/
void
tour1d_realloc_up (gint nc, datad *d, ggobid *gg)
{
  displayd *dsp;
  GList *l;
  for (l=gg->displays; l; l=l->next) {
    dsp = (displayd *) l->data;
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
}

void 
display_tour1d_init (displayd *dsp, ggobid *gg) {
  gint i, j;
  datad *d = dsp->d;
  cpaneld *cpanel = &dsp->cpanel;
  gint nc = d->ncols;

  alloc_tour1d(dsp, gg);
 
    /* Initialize starting subset of active variables */
  dsp->t1d.nvars = nc;
  for (j=0; j<nc; j++)
    dsp->t1d.vars.els[j] = j;
  /*  dsp->t1d.vars.els[0] = 0;
  dsp->t1d.vars.els[1] = 1;
  dsp->t1d.vars.els[2] = 2;*/

  /* declare starting base as first p chosen variables */
  for (i=0; i<nc; i++)
    for (j=0; j<nc; j++)
      dsp->t1d.u0.vals[i][j] = dsp->t1d.u1.vals[i][j] = dsp->t1d.u.vals[i][j] = 
        dsp->t1d.v0.vals[i][j] = dsp->t1d.v1.vals[i][j] = 0.0;

  for (i=0; i<nc; i++)
  {
    dsp->t1d.u1.vals[i][dsp->t1d.vars.els[i]] =
      dsp->t1d.u0.vals[i][dsp->t1d.vars.els[i]] = 
      dsp->t1d.u.vals[i][dsp->t1d.vars.els[i]] =
      dsp->t1d.v0.vals[i][dsp->t1d.vars.els[i]] = 
      dsp->t1d.v1.vals[i][dsp->t1d.vars.els[i]] = 1.0;
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
  dsp->t1d.dv = 1.0;
  dsp->t1d.delta = cpanel->t1d_step*M_PI_2/10.0;
  dsp->t1d.nsteps = 1; 
  dsp->t1d.stepcntr = 1;

  dsp->t1d.idled = 0;
  dsp->t1d.get_new_target = true;

  /* pp */
  dsp->t1d.target_basis_method = 0;
}

void tour1d_speed_set(gint slidepos, ggobid *gg) {
  displayd *dsp = gg->current_display; 
  cpaneld *cpanel = &dsp->cpanel;
  extern void speed_set (gint, gfloat *, gfloat *, gfloat, gint *, gint *);

  speed_set(slidepos, &cpanel->t1d_step, &dsp->t1d.delta,  dsp->t1d.dv,
    &dsp->t1d.nsteps, &dsp->t1d.stepcntr);
}


void 
tour1dvar_set (gint jvar, ggobid *gg)
{
  gint j, jtmp, k;
  gboolean active=false;
  displayd *dsp = gg->current_display;

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
tour1d_manip_var_set (gint j, ggobid *gg)
{
  g_printerr ("set the manipulation variable; not yet implemented\n");
}

void
tour1d_varsel (gint jvar, gint button, datad *d, ggobid *gg)
{
  if (button == 1 || button == 2) {
    if (d->vcirc_ui.jcursor == GDK_HAND2) {
      tour1d_manip_var_set (jvar, gg);
      varcircles_cursor_set_default (d);

    } else {
      tour1dvar_set (jvar, gg);
    }
  }
}

void
tour1d_projdata(splotd *sp, glong **world_data, datad *d, ggobid *gg) {
  int i, j, m;
  displayd *dsp = (displayd *) sp->displayptr;
  gfloat min, max, mean, keepmin, keepmax;
  gboolean firsttime = true;
  gfloat precis = PRECISION1;
  cpaneld *cpanel = &dsp->cpanel;
  gfloat *yy;

  if (sp == NULL)
    return;

  yy = (gfloat *) g_malloc (d->nrows_in_plot * sizeof (gfloat));

  for (m=0; m<d->nrows_in_plot; m++)
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
  if (firsttime) {
    keepmin = min;
    keepmax = max;
    firsttime = false;
  }
  else {
    if (min < keepmin) keepmin = min;
    if (max > keepmax) keepmax = max;
  }

  max = 2*mean;  /* try letting the max for scaling depend on the mean */
  if (cpanel->t1d_vert) {
    for (i=0; i<d->nrows_in_plot; i++) {
      sp->planar[i].x = (glong) (precis*(-1.0+2.0*
        (sp->p1d_data.els[i]-keepmin)/(max-keepmin)));
      sp->planar[i].y = yy[i];
    }
  }
  else {
    for (i=0; i<d->nrows_in_plot; i++) {
      sp->planar[i].x = yy[i];
      sp->planar[i].y = (glong) (precis*(-1.0+2.0*
        (sp->p1d_data.els[i]-keepmin)/(max-keepmin)));
    }
  }

  g_free ((gpointer) yy);

}

void
tour1d_run(displayd *dsp, ggobid *gg)
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
  extern void copy_mat(gfloat **, gfloat **, gint, gint);
  datad *d = dsp->d;
  cpaneld *cpanel = &dsp->cpanel;

  subd_param sp; 
  discriminant_param dp;
  optimize0_param op;
  cartgini_param cgp;
  cartentropy_param cep;
  cartvariance_param cvp;
  gint method, nrows, ncols, pdim;
  gfloat *gdata;
  gboolean revert_random = false;

  gint i, j, kout, nv;

  if (!dsp->t1d.get_new_target && 
      !reached_target(dsp->t1d.nsteps, dsp->t1d.stepcntr)) {
    increment_tour(dsp->t1d.tinc, dsp->t1d.tau, &dsp->t1d.nsteps, 
      &dsp->t1d.stepcntr, dsp->t1d.dv, dsp->t1d.delta, (gint) 1);
    tour_reproject(dsp->t1d.tinc, dsp->t1d.v, dsp->t1d.v0, dsp->t1d.v1,
      dsp->t1d.u, dsp->t1d.uvevec, d->ncols, (gint) 1);
  }
  else { /* do final clean-up and get new target */
    if (!dsp->t1d.get_new_target) {
      do_last_increment(dsp->t1d.tinc, dsp->t1d.tau, (gint) 1);
      tour_reproject(dsp->t1d.tinc, dsp->t1d.v, dsp->t1d.v0, dsp->t1d.v1,
        dsp->t1d.u, dsp->t1d.uvevec, d->ncols, (gint) 1);
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
          alloc_optimize0_p(&op, dsp->t1d.nvars, 1);
          arrayf_alloc_zero (&op.data, d->nrows_in_plot, dsp->t1d.nvars); 
          for (i=0; i<d->nrows_in_plot; i++)
            for (j=0; j<dsp->t1d.nvars; j++)
              op.data.vals[i][j] = 
                d->tform.vals[d->rows_in_plot[i]][dsp->t1d.vars.els[j]];

          /* Only for testing */
          nrows  = d->nrows_in_plot;
          ncols  = dsp->t1d.nvars;
          pdim   = 1;
          gdata  = malloc (nrows*sizeof(gfloat));
       
          if (d->clusterid.els==NULL) printf ("No cluster information found\n");
          for (i=0; i<nrows; i++)
	  { if (d->clusterid.els!=NULL)
              gdata[i] = d->clusterid.els[d->rows_in_plot[i]];
            else
              gdata[i] = 0;
          }

          switch (cpanel->t1d_pp_indx)
          { case 10: /* SUB-d */
              alloc_subd_p (&sp, nrows, pdim);
              kout  = optimize0 (&op, subd, &sp);
              free_subd_p (&sp);
              break;
            case 11: /* Discriminant */
              alloc_discriminant_p (&dp, gdata, nrows, pdim);
              kout = optimize0 (&op, discriminant, &dp);
              free_discriminant_p (&dp);
              break;
            case 12: /* CartGini */
              alloc_cartgini_p (&cgp, nrows, gdata);
              kout = optimize0 (&op, cartgini, &cgp);
              free_cartgini_p (&cgp);
              break;
            case 13: /* CartEntropy */
              alloc_cartentropy_p (&cep, nrows, gdata);
              kout = optimize0 (&op, cartentropy, &cep);
              free_cartentropy_p (&cep);
              break;
            case 14: /* CartVariance */
              alloc_cartvariance_p (&cvp, nrows, gdata);
              kout = optimize0 (&op, cartvariance, &cvp);
              free_cartvariance_p (&cvp);
              break;
            case 15: /* PCA */
              kout = optimize0 (&op, pca, NULL);
            default: 
              revert_random = true;
          }
          free (gdata);
          if (!revert_random) {
            sleep(5);
            for (i=0; i<d->ncols; i++)
              for (j=0; j<d->ncols; j++)
                dsp->t1d.u1.vals[i][j] = 0.0;
            for (i=0; i<dsp->t1d.nvars; i++)
              dsp->t1d.u1.vals[0][dsp->t1d.vars.els[i]] = 
                op.proj_best.vals[i][0];
            /* if the best projection is the same as the previous one, switch 
              to a random projection */
            if (!checkequiv(dsp->t1d.u0.vals, dsp->t1d.u1.vals, d->ncols, 1)) {
              gt_basis(dsp->t1d.u1, dsp->t1d.nvars, dsp->t1d.vars, 
                d->ncols, (gint) 1);
              printf("Using random projection\n");
            }
        }
          else
            gt_basis(dsp->t1d.u1, dsp->t1d.nvars, dsp->t1d.vars, 
              d->ncols, (gint) 1);
        
        free_optimize0_p(&op);
      }
      path(dsp->t1d.u0, dsp->t1d.u1, dsp->t1d.u, d->ncols, (gint) 1, dsp->t1d.v0,
        dsp->t1d.v1, dsp->t1d.v, dsp->t1d.lambda, dsp->t1d.tv, dsp->t1d.uvevec,
        dsp->t1d.tau, dsp->t1d.tinc, &dsp->t1d.nsteps, &dsp->t1d.stepcntr, 
        &dsp->t1d.dv, dsp->t1d.delta);
      dsp->t1d.get_new_target = false;
    }
  }
  /*  tour_reproject(dsp, 2);*/
  display_tailpipe (dsp, gg);

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
    dsp->t1d.idled = gtk_idle_add_priority (G_PRIORITY_LOW,
                                   (GtkFunction) tour1d_idle_func, dsp);
    gg->tour1d.idled = 1;
  } else {
    if (dsp->t1d.idled)
      gtk_idle_remove (dsp->t1d.idled);
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
  int i, j;
  displayd *dsp = gg->current_display;

  for (i=0; i<1; i++) {
    for (j=0; j<dsp->t1d.nvars; j++) {
      dsp->t1d.u0.vals[i][j] = 0.;
      dsp->t1d.u.vals[i][j] = 0.;
    }
    dsp->t1d.u0.vals[i][i] = 1.;
    dsp->t1d.u.vals[i][i] = 1.;
  }

  dsp->t1d.get_new_target = true;

}

void tour1d_vert(cpaneld *cpanel, gboolean state)
{
  cpanel->t1d_vert = state;
}


