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
#include <float.h>
extern gint _finite (gdouble);
#endif

#include "vars.h"
#include "externs.h"

#include "tour1d_pp.h"

/* */
#ifdef __cplusplus
extern "C" {
#endif
extern gint finite (gdouble);  /*-- not defined on all unixes --*/
extern gdouble erf (gdouble);  /*-- not defined on all unixes --*/
#ifdef __cplusplus
}
#endif
/* */

#define T1DON true
#define T1DOFF false

void
display_tour1d_init_null (displayd *dsp, ggobid *gg)
{
  arrayd_init_null(&dsp->t1d.Fa);
  arrayd_init_null(&dsp->t1d.Fz);
  arrayd_init_null(&dsp->t1d.F);

  arrayd_init_null(&dsp->t1d.Ga);
  arrayd_init_null(&dsp->t1d.Gz);
  arrayd_init_null(&dsp->t1d.G);

  arrayd_init_null(&dsp->t1d.Va);
  arrayd_init_null(&dsp->t1d.Vz);

  arrayd_init_null(&dsp->t1d.tv);

  vectori_init_null(&dsp->t1d.active_vars);
  vectorf_init_null(&dsp->t1d.lambda);
  vectorf_init_null(&dsp->t1d.tau);
  vectorf_init_null(&dsp->t1d.tinc);

  /* manipulation controls */
  arrayd_init_null(&dsp->t1d_manbasis);
}

void
alloc_tour1d (displayd *dsp, ggobid *gg)
{
  datad *d = dsp->d;
  gint nc = d->ncols;

  arrayd_alloc(&dsp->t1d.Fa, 1, nc);
  arrayd_alloc(&dsp->t1d.Fz, 1, nc);
  arrayd_alloc(&dsp->t1d.F, 1, nc);

  arrayd_alloc(&dsp->t1d.Ga, 1, nc);
  arrayd_alloc(&dsp->t1d.Gz, 1, nc);
  arrayd_alloc(&dsp->t1d.G, 1, nc);

  arrayd_alloc(&dsp->t1d.Va, 1, nc);
  arrayd_alloc(&dsp->t1d.Vz, 1, nc);

  arrayd_alloc(&dsp->t1d.tv, 1, nc);

  vectori_alloc(&dsp->t1d.active_vars, nc);
  vectorf_alloc(&dsp->t1d.lambda, nc);
  vectorf_alloc(&dsp->t1d.tau, nc);
  vectorf_alloc(&dsp->t1d.tinc, nc);

  /* manipulation controls */
  arrayd_alloc(&dsp->t1d_manbasis, 2, nc);
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
      arrayd_delete_cols (&dsp->t1d.Fa, nc, cols);
      arrayd_delete_cols (&dsp->t1d.Fz, nc, cols);
      arrayd_delete_cols (&dsp->t1d.F, nc, cols);
      arrayd_delete_cols (&dsp->t1d.Ga, nc, cols);
      arrayd_delete_cols (&dsp->t1d.Gz, nc, cols);
      arrayd_delete_cols (&dsp->t1d.G, nc, cols);
      arrayd_delete_cols (&dsp->t1d.Va, nc, cols);
      arrayd_delete_cols (&dsp->t1d.Vz, nc, cols);
      arrayd_delete_cols (&dsp->t1d.tv, nc, cols);

      vectori_delete_els (&dsp->t1d.active_vars, nc, cols);
      vectorf_delete_els (&dsp->t1d.lambda, nc, cols);
      vectorf_delete_els (&dsp->t1d.tau, nc, cols);
      vectorf_delete_els (&dsp->t1d.tinc, nc, cols);

      arrayd_delete_cols (&dsp->t1d_manbasis, (gint) nc, cols);
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
    GtkGGobiExtendedDisplayClass *klass;
    dsp = (displayd *) l->data;

    if(!GTK_IS_GGOBI_EXTENDED_DISPLAY(dsp))
      continue;
    klass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(dsp)->klass);
    if(klass->tour1d_realloc)
        klass->tour1d_realloc(dsp, nc, d);
  }
}

void
free_tour1d(displayd *dsp)
{
  vectori_free(&dsp->t1d.active_vars);
  vectorf_free(&dsp->t1d.lambda);
  vectorf_free(&dsp->t1d.tau);
  vectorf_free(&dsp->t1d.tinc);

  arrayd_free(&dsp->t1d.Fa, 0, 0);
  arrayd_free(&dsp->t1d.Fz, 0, 0);
  arrayd_free(&dsp->t1d.F, 0, 0);

  arrayd_free(&dsp->t1d.Ga, 0, 0);
  arrayd_free(&dsp->t1d.Gz, 0, 0);
  arrayd_free(&dsp->t1d.F, 0, 0);

  arrayd_free(&dsp->t1d.Va, 0, 0);
  arrayd_free(&dsp->t1d.Vz, 0, 0);
  arrayd_free(&dsp->t1d.tv, 0, 0);

  arrayd_free(&dsp->t1d_manbasis, 0, 0);
}

void 
display_tour1d_init (displayd *dsp, ggobid *gg) 
{
  gint i, j;
  datad *d = dsp->d;
  cpaneld *cpanel = &dsp->cpanel;
  gint nc = d->ncols;

  alloc_tour1d(dsp, gg);

    /* Initialize starting subset of active variables */
  if (nc < 8) {
    dsp->t1d.nactive = nc;
    for (j=0; j<nc; j++)
      dsp->t1d.active_vars.els[j] = j;
  }
  else {
    dsp->t1d.nactive = 3;
    dsp->t1d.active_vars.els[0] = 0;
    dsp->t1d.active_vars.els[1] = 1;
    dsp->t1d.active_vars.els[2] = 2;
    for (j=3; j<nc; j++)
      dsp->t1d.active_vars.els[j] = 0;

  }

  /* declare starting base as first p chosen variables */
  for (i=0; i<1; i++)
    for (j=0; j<nc; j++)
      dsp->t1d.Fa.vals[i][j] = dsp->t1d.Fz.vals[i][j] = 
        dsp->t1d.F.vals[i][j] = 
        dsp->t1d.Ga.vals[i][j] = dsp->t1d.Gz.vals[i][j] = 0.0;

  for (i=0; i<1; i++)
  {
    dsp->t1d.Fz.vals[i][dsp->t1d.active_vars.els[i]] =
      dsp->t1d.Fa.vals[i][dsp->t1d.active_vars.els[i]] = 
      dsp->t1d.F.vals[i][dsp->t1d.active_vars.els[i]] =
      dsp->t1d.Ga.vals[i][dsp->t1d.active_vars.els[i]] = 
      dsp->t1d.Gz.vals[i][dsp->t1d.active_vars.els[i]] = 1.0;
  }

  dsp->t1d.dist_az = 0.0;
  dsp->t1d.delta = cpanel->t1d.step*M_PI_2/10.0;
  dsp->t1d.tang = 0.0;

  dsp->t1d.idled = 0;
  dsp->t1d.get_new_target = true;

  /* manip */
  dsp->t1d_manip_var = 0;

  /* pp */
  dsp->t1d.target_selection_method = 0;
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

  cpanel->t1d.slidepos = slidepos;
  speed_set(slidepos, &cpanel->t1d.step, &dsp->t1d.delta);
}

void tour1d_pause (cpaneld *cpanel, gboolean state, ggobid *gg) {
  cpanel->t1d.paused = state;

  tour1d_func (!cpanel->t1d.paused, gg->current_display, gg);

  if (cpanel->t1d.paused) {
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

  for (j=0; j<dsp->t1d.nactive; j++)
    if (jvar == dsp->t1d.active_vars.els[j])
      active = true;

  /* deselect var if t1d.nactive > 2 */
  if (active) {
    if (dsp->t1d.nactive > 1) {
      for (j=0; j<dsp->t1d.nactive; j++) {
        if (jvar == dsp->t1d.active_vars.els[j]) 
          break;
      }
      if (j<dsp->t1d.nactive-1) {
        for (k=j; k<dsp->t1d.nactive-1; k++){
          dsp->t1d.active_vars.els[k] = dsp->t1d.active_vars.els[k+1];
        }
      }
      dsp->t1d.nactive--;

      if (!gg->tour1d.fade_vars) /* set current position without sel var */
      {
        gt_basis(dsp->t1d.Fa, dsp->t1d.nactive, dsp->t1d.active_vars, 
          d->ncols, (gint) 1);
        arrayd_copy(&dsp->t1d.Fa, &dsp->t1d.F);
/*      copy_mat(dsp->t1d.F.vals, dsp->t1d.Fa.vals, d->ncols, 1);*/
      }
    }
  }
  else { /* not active, so add the variable */
    if (jvar > dsp->t1d.active_vars.els[dsp->t1d.nactive-1]) {
      dsp->t1d.active_vars.els[dsp->t1d.nactive] = jvar;
    }
    else if (jvar < dsp->t1d.active_vars.els[0]) {
      for (j=dsp->t1d.nactive; j>0; j--) {
          dsp->t1d.active_vars.els[j] = dsp->t1d.active_vars.els[j-1];
      }
      dsp->t1d.active_vars.els[0] = jvar;
    }
    else {
      gint jtmp = dsp->t1d.nactive;
      for (j=0; j<dsp->t1d.nactive-1; j++) {
        if (jvar > dsp->t1d.active_vars.els[j] && jvar < dsp->t1d.active_vars.els[j+1]) {
          jtmp = j+1;
          break;
        }
      }
      for (j=dsp->t1d.nactive-1;j>=jtmp; j--) 
          dsp->t1d.active_vars.els[j+1] = dsp->t1d.active_vars.els[j];
      dsp->t1d.active_vars.els[jtmp] = jvar;
    }
    dsp->t1d.nactive++;
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
  displayd *dsp = gg->current_display;
  gchar *label = g_strdup("PP index: (0.0) 0.0000 (0.0)");

/*-- any button --*/
  if (d->vcirc_ui.jcursor == GDK_HAND2) {
    tour1d_manip_var_set (jvar, button, gg);
    varcircles_cursor_set_default (d);

  } else {
    tour1dvar_set (jvar, gg);

    if (dsp->t1d_window != NULL && GTK_WIDGET_VISIBLE (dsp->t1d_window)) {
      realloc_optimize0_p(&dsp->t1d_pp_op, dsp->t1d.nactive, 
        dsp->t1d.active_vars);

      t1d_pp_reinit(gg);
      label = g_strdup_printf ("PP index: (%3.1f) %5.3f (%3.1f)",
      dsp->t1d_indx_min, dsp->t1d_ppindx_mat[dsp->t1d_ppindx_count], 
      dsp->t1d_indx_max);
      gtk_label_set_text(GTK_LABEL(dsp->t1d_pplabel),label);
    }
  }
}

void
tour1d_projdata(splotd *sp, greal **world_data, datad *d, ggobid *gg)
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
      /*yy[i] += (gint)(dsp->t1d.F.vals[0][j]*world_data[i][j]);*/
      yy[i] += (gfloat)(dsp->t1d.F.vals[0][j]*world_data[i][j]);
    }
  }

  do_ash1d (yy, d->nrows_in_plot,
            cpanel->t1d.nbins, cpanel->t1d.nASHes,
            sp->p1d.spread_data.els, &min, &max, &mean);
/*
  if (sp->tour1d.firsttime) {
    sp->tour1d.keepmin = 0.0;    let this be zero for consistency
    sp->tour1d.keepmax = max;    ... and this isn't currently used
    sp->tour1d.firsttime = false;   ... so this isn't needed
  }
*/

  max = 2*mean;  /* try letting the max for scaling depend on the mean */
  if (cpanel->t1d.vert) {
    for (i=0; i<d->nrows_in_plot; i++) {
      sp->planar[i].x = (greal) (precis*(-1.0+2.0*
        sp->p1d.spread_data.els[i]/max));
        /*(sp->p1d_data.els[i]-min)/(max-min)));*/
      sp->planar[i].y = yy[i];
    }
  }
  else {
    for (i=0; i<d->nrows_in_plot; i++) {
      sp->planar[i].x = yy[i];
      sp->planar[i].y = (greal) (precis*(-1.0+2.0*
        sp->p1d.spread_data.els[i]/max));
        /*(sp->p1d_data.els[i]-min)/(max-min)));*/
    }
  }

  g_free ((gpointer) yy);
}

void
tour1d_run(displayd *dsp, ggobid *gg)
{
  datad *d = dsp->d;
  cpaneld *cpanel = &dsp->cpanel;
  /*  static gint count = 0;*/
  gboolean revert_random = false;
  gint pathprob = 0;

  gint i, j, nv;

  if (!dsp->t1d.get_new_target && 
      !reached_target(dsp->t1d.tang, dsp->t1d.dist_az, 
        dsp->t1d.target_selection_method,&dsp->t1d.ppval, &dsp->t1d.oppval)) {
    increment_tour(dsp->t1d.tinc, dsp->t1d.tau, dsp->t1d.dist_az, 
      dsp->t1d.delta, &dsp->t1d.tang, (gint) 1);
    tour_reproject(dsp->t1d.tinc, dsp->t1d.G, dsp->t1d.Ga, dsp->t1d.Gz,
      dsp->t1d.F, dsp->t1d.Va, d->ncols, (gint) 1);

    /* plot pp indx */
    if (dsp->t1d_ppda != NULL) {

      dsp->t1d.oppval = dsp->t1d.ppval;
      revert_random = t1d_switch_index(cpanel->t1d.pp_indx, 
        0, gg);
      /*      count++;
      if (count == 10) {
      count = 0;*/
        t1d_ppdraw(dsp->t1d.ppval, gg);
/*      }*/
    }

  }
  else { /* do final clean-up and get new target */
    if (dsp->t1d.get_new_target) {
      if (dsp->t1d.target_selection_method == 1)
      {
        dsp->t1d_pp_op.index_best = dsp->t1d.ppval;
/*      dsp->t1d.oppval = dsp->t1d.ppval;*/
        for (j=0; j<dsp->t1d.nactive; j++) 
          dsp->t1d_pp_op.proj_best.vals[0][j] = 
            dsp->t1d.F.vals[0][dsp->t1d.active_vars.els[j]];
      }
    }
    else 
    {
      if (dsp->t1d.target_selection_method == 1)
/*      t1d_ppdraw(dsp->t1d.ppval, gg)*/;
      else
      {
        do_last_increment(dsp->t1d.tinc, dsp->t1d.tau, 
          dsp->t1d.dist_az, (gint) 1);
        tour_reproject(dsp->t1d.tinc, dsp->t1d.G, dsp->t1d.Ga, dsp->t1d.Gz,
          dsp->t1d.F, dsp->t1d.Va, d->ncols, (gint) 1);
      }
    }
    arrayd_copy(&dsp->t1d.F, &dsp->t1d.Fa);
    /*    copy_mat(dsp->t1d.Fa.vals, dsp->t1d.F.vals, d->ncols, 1);*/
    nv = 0;
    for (i=0; i<d->ncols; i++)
      if (fabs(dsp->t1d.Fa.vals[0][i]) > 0.01) {
        nv++;
      }
    if (nv == 1 && dsp->t1d.nactive == 1) /* only generate new dir if num of
                                           active/used variables is > 2 */
      dsp->t1d.get_new_target = true;
    else {
      if (dsp->t1d.target_selection_method == 0) {
        gt_basis(dsp->t1d.Fz, dsp->t1d.nactive, dsp->t1d.active_vars, 
          d->ncols, (gint) 1);
      }
      else if (dsp->t1d.target_selection_method == 1) {
        /* pp guided tour  */
        /* get new target according to the selected pp index */
        for (i=0; i<d->ncols; i++)
          dsp->t1d.Fz.vals[0][i] = 0.0;
        dsp->t1d.Fz.vals[0][dsp->t1d.active_vars.els[0]]=1.0;

        dsp->t1d.oppval = -999.0;
        revert_random = t1d_switch_index(cpanel->t1d.pp_indx, 
          dsp->t1d.target_selection_method, gg);

        if (!revert_random) {
          for (i=0; i<dsp->t1d.nactive; i++) {
#ifdef WIN32
            if (_finite((gdouble)dsp->t1d_pp_op.proj_best.vals[0][i]) != 0)
#else
            if (finite((gdouble)dsp->t1d_pp_op.proj_best.vals[0][i]) != 0)
#endif
              dsp->t1d.Fz.vals[0][dsp->t1d.active_vars.els[i]] = 
                dsp->t1d_pp_op.proj_best.vals[0][i];
          }

          /* if the best projection is the same as the previous one, switch 
              to a random projection */
          if (!checkequiv(dsp->t1d.Fa.vals, dsp->t1d.Fz.vals, d->ncols, 1)) {
    /*            g_printerr ("Using random projection\n");*/
            gt_basis(dsp->t1d.Fz, dsp->t1d.nactive, dsp->t1d.active_vars, 
              d->ncols, (gint) 1);
            for (j=0; j<dsp->t1d.nactive; j++)
              dsp->t1d_pp_op.proj_best.vals[0][j] = 
                dsp->t1d.Fz.vals[0][dsp->t1d.active_vars.els[j]];
              /*              dsp->t1d.ppval = -999.0;*/
            revert_random = t1d_switch_index(cpanel->t1d.pp_indx, 
              dsp->t1d.target_selection_method, gg);
          }
          /*t1d_ppdraw(dsp->t1d.ppval, gg);*/
  /*          count = 0;*/
#ifndef WIN32
          sleep(2);
#else
          Sleep(2);
#endif
        }
        else /* Use random target */
        {
          gt_basis(dsp->t1d.Fz, dsp->t1d.nactive, dsp->t1d.active_vars, 
            d->ncols, (gint) 1);
        }
        
      }
      pathprob = path(dsp->t1d.Fa, dsp->t1d.Fz, dsp->t1d.F, d->ncols, 
        (gint) 1, dsp->t1d.Ga, dsp->t1d.Gz, dsp->t1d.G, 
        dsp->t1d.lambda, dsp->t1d.tv, dsp->t1d.Va,
        dsp->t1d.Vz, dsp->t1d.tau, dsp->t1d.tinc, 
        &dsp->t1d.dist_az, &dsp->t1d.tang);
      if (pathprob == 0) 
        dsp->t1d.get_new_target = false;
      else if (pathprob == 1) { /* problems with Fa so need to force a jump */
        tour1d_scramble(gg);
        pathprob = path(dsp->t1d.Fa, dsp->t1d.Fz, dsp->t1d.F, d->ncols, 
          (gint) 1, dsp->t1d.Ga,
          dsp->t1d.Gz, dsp->t1d.G, dsp->t1d.lambda, dsp->t1d.tv, dsp->t1d.Va,
          dsp->t1d.Vz,  dsp->t1d.tau, dsp->t1d.tinc, 
          &dsp->t1d.dist_az, &dsp->t1d.tang);
      }
      else if (pathprob == 2 || pathprob == 3) { /* problems with Fz,
                                      so will force a new choice of Fz */
        dsp->t1d.get_new_target = true;
      }
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
  gboolean doit = !cpanel->t1d.paused;

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
}

void tour1d_reinit(ggobid *gg)
{
  gint i, j;
  displayd *dsp = gg->current_display;
  datad *d = dsp->d;

  for (i=0; i<1; i++) {
    for (j=0; j<d->ncols; j++) {
      dsp->t1d.Fa.vals[i][j] = 0.;
      dsp->t1d.F.vals[i][j] = 0.;
    }
    dsp->t1d.Fa.vals[i][dsp->t1d.active_vars.els[i]] = 1.;
    dsp->t1d.F.vals[i][dsp->t1d.active_vars.els[i]] = 1.;
  }

  dsp->t1d.get_new_target = true;

  display_tailpipe (dsp, FULL, gg);

  varcircles_refresh (d, gg);

  if (dsp->t1d_window != NULL && GTK_WIDGET_VISIBLE (dsp->t1d_window)) 
    t1d_pp_reinit(gg);
}

void tour1d_scramble(ggobid *gg)
{
  int i, j;
  displayd *dsp = gg->current_display;
  datad *d = dsp->d;
  gint nc = d->ncols;

  for (i=0; i<1; i++)
    for (j=0; j<nc; j++)
      dsp->t1d.Fa.vals[i][j] = dsp->t1d.Fz.vals[i][j] = 
        dsp->t1d.F.vals[i][j] = dsp->t1d.Ga.vals[i][j] = 
        dsp->t1d.Gz.vals[i][j] = 0.0;

  gt_basis(dsp->t1d.Fa, dsp->t1d.nactive, dsp->t1d.active_vars, 
    d->ncols, (gint) 1);
  arrayd_copy(&dsp->t1d.Fa, &dsp->t1d.F);
  /*  copy_mat(dsp->t1d.F.vals, dsp->t1d.Fa.vals, d->ncols, 1);*/

  dsp->t1d.get_new_target = true;

  display_tailpipe (dsp, FULL, gg);

  varcircles_refresh (d, gg);

  if (dsp->t1d_window != NULL && GTK_WIDGET_VISIBLE (dsp->t1d_window)) 
    t1d_pp_reinit(gg);
}

void tour1d_vert(cpaneld *cpanel, gboolean state)
{
  cpanel->t1d.vert = state;
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
  gint n1vars = dsp->t1d.nactive;
  gfloat ftmp, tol = 0.01; 
  gboolean dontdoit = false;

  dsp->t1d_phi = 0.;

  /* gets mouse position */
  if (cpanel->t1d.vert) 
    dsp->t1d_pos = dsp->t1d_pos_old = p2;
  else
    dsp->t1d_pos = dsp->t1d_pos_old = p1;

  /* initializes indicator for manip var being one of existing vars */
  dsp->t1d_manipvar_inc = false;

  /* need to turn off tour */
  if (!cpanel->t1d.paused)
    tour1d_func(T1DOFF, gg->current_display, gg);

  /* check if manip var is one of existing vars */
  /* n1vars, n2vars is the number of variables, excluding the
     manip var in hor and vert directions */
  for (j=0; j<dsp->t1d.nactive; j++)
    if (dsp->t1d.active_vars.els[j] == dsp->t1d_manip_var) {
      dsp->t1d_manipvar_inc = true;
      n1vars--;
    }

  /* make manip basis, from existing projection */
  /* 0 will be the remainder of the projection, and
     1 will be the indicator vector for the manip var */
  for (j=0; j<d->ncols; j++) {
    dsp->t1d_manbasis.vals[0][j] = dsp->t1d.F.vals[0][j];
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
  gint actual_nxvars = dsp->t1d.nactive;
  gint j;
  gboolean offscreen = false;

  /* check if off the plot window */
  if (p1 > sp->max.x || p1 < 0 ||
      p2 > sp->max.y || p2 <0)
    offscreen = true;

  if (dsp->t1d_manipvar_inc)
    actual_nxvars = dsp->t1d.nactive-1;

  if (!offscreen) {
    dsp->t1d_pos_old = dsp->t1d_pos;
  
    dsp->t1d_pos = p1;

    if (actual_nxvars > 0)
    {
      if (cpanel->t1d.vert)
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
        dsp->t1d.F.vals[0][j] = xcosphi * dsp->t1d_manbasis.vals[0][j] + 
         xsinphi * dsp->t1d_manbasis.vals[1][j];
    }
 
    display_tailpipe (dsp, FULL, gg);
    varcircles_refresh (d, gg);
  }
  else {
    disconnect_motion_signal (sp);
    arrayd_copy(&dsp->t1d.F, &dsp->t1d.Fa);
    /*    copy_mat(dsp->t1d.Fa.vals, dsp->t1d.F.vals, d->ncols, 1);*/
    dsp->t1d.get_new_target = true;
    if (!cpanel->t1d.paused)
      tour1d_func(T1DON, gg->current_display, gg);
  }
}

void
tour1d_manip_end(splotd *sp) 
{
  displayd *dsp = (displayd *) sp->displayptr;
  cpaneld *cpanel = &dsp->cpanel;
  ggobid *gg = GGobiFromSPlot(sp);

  disconnect_motion_signal (sp);

  arrayd_copy(&dsp->t1d.F, &dsp->t1d.Fa);
  /*  copy_mat(dsp->t1d.Fa.vals, dsp->t1d.F.vals, d->ncols, 1);*/
  dsp->t1d.get_new_target = true;

  /* need to turn on tour? */
  if (!cpanel->t1d.paused) {
    tour1d_pause(cpanel, T1DOFF, gg);

    /*-- whenever motion stops, we need a FULL redraw --*/
    display_tailpipe (gg->current_display, FULL, gg);
  }
}

#undef T1DON
#undef T1DOFF

