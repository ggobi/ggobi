/* tour2d3.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/
#ifdef ROTATION_IMPLEMENTED

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

#define T2D3ON  true
#define T2D3OFF false

void
display_tour2d3_init_null (displayd *dsp, ggobid *gg)
{
  arrayd_init_null(&dsp->t2d3.Fa);
  arrayd_init_null(&dsp->t2d3.Fz);
  arrayd_init_null(&dsp->t2d3.F);

  arrayd_init_null(&dsp->t2d3.Ga);
  arrayd_init_null(&dsp->t2d3.Gz);
  arrayd_init_null(&dsp->t2d3.G);

  arrayd_init_null(&dsp->t2d3.Va);
  arrayd_init_null(&dsp->t2d3.Vz);

  arrayd_init_null(&dsp->t2d3.tv);

  vectori_init_null(&dsp->t2d3.subset_vars);
  vectorb_init_null(&dsp->t2d3.subset_vars_p);
  vectori_init_null(&dsp->t2d3.active_vars);
  vectorb_init_null(&dsp->t2d3.active_vars_p);

  vectorf_init_null(&dsp->t2d3.lambda);
  vectorf_init_null(&dsp->t2d3.tau);
  vectorf_init_null(&dsp->t2d3.tinc);

  /* manipulation variables */
  arrayd_init_null(&dsp->t2d3_Rmat1);
  arrayd_init_null(&dsp->t2d3_Rmat2);
  arrayd_init_null(&dsp->t2d3_mvar_3dbasis);
  arrayd_init_null(&dsp->t2d3_manbasis);
}

void
alloc_tour2d3 (displayd *dsp, ggobid *gg)
{
  datad *d = dsp->d;
  gint nc = d->ncols;

  /* first index is the projection dimensions, second dimension is ncols */
  arrayd_alloc(&dsp->t2d3.Fa, 2, nc);
  arrayd_alloc(&dsp->t2d3.Fz, 2, nc);
  arrayd_alloc(&dsp->t2d3.F, 2, nc);

  arrayd_alloc(&dsp->t2d3.Ga, 2, nc);
  arrayd_alloc(&dsp->t2d3.Gz, 2, nc);
  arrayd_alloc(&dsp->t2d3.G, 2, nc);

  arrayd_alloc(&dsp->t2d3.Va, 2, nc);
  arrayd_alloc(&dsp->t2d3.Vz, 2, nc);

  arrayd_alloc(&dsp->t2d3.tv, 2, nc);

  vectori_alloc(&dsp->t2d3.subset_vars, nc);
  vectorb_alloc_zero(&dsp->t2d3.subset_vars_p, nc);
  vectori_alloc(&dsp->t2d3.active_vars, nc);
  vectorb_alloc_zero(&dsp->t2d3.active_vars_p, nc);

  vectorf_alloc(&dsp->t2d3.lambda, nc);
  vectorf_alloc_zero(&dsp->t2d3.tau, nc);
  vectorf_alloc_zero(&dsp->t2d3.tau, nc);
  vectorf_alloc(&dsp->t2d3.tinc, nc);

  /* manipulation variables */
  arrayd_alloc(&dsp->t2d3_Rmat1, 3, 3);
  arrayd_alloc(&dsp->t2d3_Rmat2, 3, 3);
  arrayd_alloc(&dsp->t2d3_mvar_3dbasis, 3, 3);
  arrayd_alloc(&dsp->t2d3_manbasis, 3, nc);
}

/*-- eliminate the nc columns contained in *cols --*/
void
tour2d3_realloc_down (gint nc, gint *cols, datad *d, ggobid *gg)
{
  displayd *dsp;
  GList *l;
  for (l=gg->displays; l; l=l->next) {
    dsp = (displayd *) l->data;
    if (dsp->d == d) {
      arrayd_delete_cols (&dsp->t2d3.Fa, nc, cols);
      arrayd_delete_cols (&dsp->t2d3.Fz, nc, cols);
      arrayd_delete_cols (&dsp->t2d3.F, nc, cols);
      arrayd_delete_cols (&dsp->t2d3.Ga, nc, cols);
      arrayd_delete_cols (&dsp->t2d3.Gz, nc, cols);
      arrayd_delete_cols (&dsp->t2d3.G, nc, cols);
      arrayd_delete_cols (&dsp->t2d3.Va, nc, cols);
      arrayd_delete_cols (&dsp->t2d3.Vz, nc, cols);
      arrayd_delete_cols (&dsp->t2d3.tv, nc, cols);

      vectori_delete_els (&dsp->t2d3.subset_vars, nc, cols);
      vectorb_delete_els (&dsp->t2d3.subset_vars_p, nc, cols);
      vectori_delete_els (&dsp->t2d3.active_vars, nc, cols);
      vectorb_delete_els (&dsp->t2d3.active_vars_p, nc, cols);

      vectorf_delete_els (&dsp->t2d3.lambda, nc, cols);
      vectorf_delete_els (&dsp->t2d3.tau, nc, cols);
      vectorf_delete_els (&dsp->t2d3.tinc, nc, cols);

      arrayd_delete_cols (&dsp->t2d3_manbasis, (gint) nc, cols);
    }
  }
}

/*-- append columns for a total of nc columns --*/
/*-- we don't know for certain that tour has been initialized, do we? --*/
void
tour2d3_realloc_up (gint nc, datad *d, ggobid *gg)
{
  displayd *dsp;
  GList *l;

  for (l=gg->displays; l; l=l->next) {
    GtkGGobiExtendedDisplayClass *klass;
    dsp = (displayd *) l->data;

    if(!GTK_IS_GGOBI_EXTENDED_DISPLAY(dsp))
      continue;
    klass = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(dsp)->klass);
    if(klass->tour2d3_realloc)
        klass->tour2d3_realloc(dsp, nc, d);
  }
}

void
free_tour2d3(displayd *dsp)
{
  vectori_free(&dsp->t2d3.subset_vars);
  vectorb_free(&dsp->t2d3.subset_vars_p);
  vectori_free(&dsp->t2d3.active_vars);
  vectorb_free(&dsp->t2d3.active_vars_p);

  vectorf_free(&dsp->t2d3.lambda);
  vectorf_free(&dsp->t2d3.tau);
  vectorf_free(&dsp->t2d3.tinc);

  arrayd_free(&dsp->t2d3.Fa, 0, 0);
  arrayd_free(&dsp->t2d3.Fz, 0, 0);
  arrayd_free(&dsp->t2d3.F, 0, 0);

  arrayd_free(&dsp->t2d3.Ga, 0, 0);
  arrayd_free(&dsp->t2d3.Gz, 0, 0);
  arrayd_free(&dsp->t2d3.G, 0, 0);

  arrayd_free(&dsp->t2d3.Va, 0, 0);
  arrayd_free(&dsp->t2d3.Vz, 0, 0);
  arrayd_free(&dsp->t2d3.tv, 0, 0);

  arrayd_free(&dsp->t2d3_Rmat1, 0, 0);
  arrayd_free(&dsp->t2d3_Rmat2, 0, 0);
  arrayd_free(&dsp->t2d3_mvar_3dbasis, 0, 0);
  arrayd_free(&dsp->t2d3_manbasis, 0, 0);
}

void 
display_tour2d3_init (displayd *dsp, ggobid *gg) {
  gint i, j;
  datad *d = dsp->d;
  cpaneld *cpanel = &dsp->cpanel;
  gint nc = d->ncols;

  if (nc < MIN_NVARS_FOR_TOUR2D3)
    return;

  alloc_tour2d3(dsp, gg);
 
/*
 * subset_vars.els[0] = VARSEL_X
 * subset_vars.els[1] = VARSEL_Y
 * subset_vars.els[2] = VARSEL_Z
*/
  dsp->t2d3.nsubset = dsp->t2d3.nactive = 3;
  for (j=0; j<nc; j++) {
    dsp->t2d3.subset_vars.els[j] = dsp->t2d3.active_vars.els[j] = 0;
    dsp->t2d3.subset_vars_p.els[j] = dsp->t2d3.active_vars_p.els[j] = false;
  }
  for (j=0; j<3; j++) {
    dsp->t2d3.subset_vars.els[j] = dsp->t2d3.active_vars.els[j] = j;
    dsp->t2d3.subset_vars_p.els[j] = dsp->t2d3.active_vars_p.els[j] = true;
  }

  /* declare starting base as first 2 chosen variables */
  for (i=0; i<2; i++)
    for (j=0; j<nc; j++)
      dsp->t2d3.Fa.vals[i][j] = dsp->t2d3.Fz.vals[i][j] = 
        dsp->t2d3.F.vals[i][j] = dsp->t2d3.Ga.vals[i][j] = 
        dsp->t2d3.Gz.vals[i][j] = 0.0;

  for (i=0; i<2; i++) {
    dsp->t2d3.Fz.vals[i][dsp->t2d3.active_vars.els[i]] =
      dsp->t2d3.Fa.vals[i][dsp->t2d3.active_vars.els[i]] = 
      dsp->t2d3.F.vals[i][dsp->t2d3.active_vars.els[i]] =
      dsp->t2d3.Ga.vals[i][dsp->t2d3.active_vars.els[i]] = 
      dsp->t2d3.Gz.vals[i][dsp->t2d3.active_vars.els[i]] = 1.0;
  }

  dsp->t2d3.dist_az = 0.0;
  dsp->t2d3.delta = cpanel->t2d3.step*M_PI_2/10.0;
  dsp->t2d3.tang = 0.0;

  dsp->t2d3.idled = 0;
  dsp->t2d3.get_new_target = true;

  /* manip */
  dsp->t2d3_manip_var = 0;
}

void tour2d3_speed_set(gint slidepos, ggobid *gg) {
  displayd *dsp = gg->current_display; 
  cpaneld *cpanel = &dsp->cpanel;

  cpanel->t2d3.slidepos = slidepos;
  speed_set(slidepos, &cpanel->t2d3.step, &dsp->t2d3.delta);
}

void tour2d3_pause (cpaneld *cpanel, gboolean state, ggobid *gg) {
  cpanel->t2d3.paused = state;

  tour2d3_func (!cpanel->t2d3.paused, gg->current_display, gg);

  if (cpanel->t2d3.paused) {
    /*-- whenever motion stops, we need a FULL redraw --*/
    display_tailpipe (gg->current_display, FULL, gg);
  }
}

/*-- add/remove jvar to/from the subset of variables that <may> be active --*/
gboolean
tour2d3_subset_var_set (gint jvar, gint button, datad *d,
  displayd *dsp, ggobid *gg)
{
  gboolean in_subset = dsp->t2d3.subset_vars_p.els[jvar];
  gint j, k;
  gboolean changed = false;
  gint xyz;

  /*-- require exactly 3 variables in the subset --*/
  if (in_subset) {               /*-- handle a swap --*/

    if (dsp->t2d3.subset_vars.els[button] == jvar)
/**/  return false;

    switch (button) {
      case VARSEL_X:
        xyz = (jvar == dsp->t2d3.subset_vars.els[VARSEL_Y]) ?
          VARSEL_Y : VARSEL_Z;
      break;
      case VARSEL_Y:
        xyz = (jvar == dsp->t2d3.subset_vars.els[VARSEL_X]) ?
          VARSEL_X : VARSEL_Z;
      break;
      case VARSEL_Z:
        xyz = (jvar == dsp->t2d3.subset_vars.els[VARSEL_X]) ?
          VARSEL_X : VARSEL_Y;
      break;
      default:
/**/    return false;
      break;
    }
    dsp->t2d3.subset_vars.els[xyz] = dsp->t2d3.subset_vars.els[button];
    dsp->t2d3.subset_vars.els[button] = jvar;
    changed = true;
  } else {
    dsp->t2d3.subset_vars.els[button] = jvar;
    changed = true;
  }

  /*-- reset subset_vars_p based on subset_vars: unlike other tour modes --*/
  if (changed) {
    for (j=0; j<d->ncols; j++)
      dsp->t2d3.subset_vars_p.els[j] = false;
    for (j=0; j<3; j++) {
      k = dsp->t2d3.subset_vars.els[j];
      dsp->t2d3.subset_vars_p.els[k] = true;
    }
  }

  return changed;
}

/*-- add or remove jvar from the set of active variables --*/
void 
tour2d3_active_var_set (gint jvar, datad *d, displayd *dsp, ggobid *gg)
{
  gint j, jtmp, k;
  gboolean active = dsp->t2d3.active_vars_p.els[jvar];

  /* deselect var if t2d3.nactive > 2 */
  if (active) {
    if (dsp->t2d3.nactive > 2) {
      for (j=0; j<dsp->t2d3.nactive; j++) {
        if (jvar == dsp->t2d3.active_vars.els[j]) 
          break;
      }
      if (j<dsp->t2d3.nactive-1) {
        for (k=j; k<dsp->t2d3.nactive-1; k++) {
          dsp->t2d3.active_vars.els[k] = dsp->t2d3.active_vars.els[k+1];
        }
      }
      dsp->t2d3.nactive--;
 
      gt_basis(dsp->t2d3.Fa, dsp->t2d3.nactive, dsp->t2d3.active_vars, 
        d->ncols, (gint) 2);
      arrayd_copy(&dsp->t2d3.Fa, &dsp->t2d3.F);

      dsp->t2d3.active_vars_p.els[jvar] = false;
    }
  }
  else { /* not active, so add the variable */
    if (jvar > dsp->t2d3.active_vars.els[dsp->t2d3.nactive-1]) {
      dsp->t2d3.active_vars.els[dsp->t2d3.nactive] = jvar;
    }
    else if (jvar < dsp->t2d3.active_vars.els[0]) {
      for (j=dsp->t2d3.nactive; j>0; j--) {
          dsp->t2d3.active_vars.els[j] = dsp->t2d3.active_vars.els[j-1];
      }
      dsp->t2d3.active_vars.els[0] = jvar;
    }
    else {
      for (j=0; j<dsp->t2d3.nactive-1; j++) {
        if (jvar > dsp->t2d3.active_vars.els[j] &&
            jvar < dsp->t2d3.active_vars.els[j+1])
        {
          jtmp = j+1;
          break;
        }
      }
      for (j=dsp->t2d3.nactive-1;j>=jtmp; j--) 
          dsp->t2d3.active_vars.els[j+1] = dsp->t2d3.active_vars.els[j];
      dsp->t2d3.active_vars.els[jtmp] = jvar;
    }
    dsp->t2d3.nactive++;
    dsp->t2d3.active_vars_p.els[jvar] = true;
  }

  dsp->t2d3.get_new_target = true;
}

static void
tour2d3_manip_var_set (gint j, ggobid *gg)
{
  displayd *dsp = gg->current_display;

  dsp->t2d3_manip_var = j;    
}

gboolean
tour2d3_varsel (GtkWidget *w, gint jvar, gint button, datad *d, ggobid *gg)
{
  displayd *dsp = gg->current_display;
  gboolean changed = true;

  if (GTK_IS_TOGGLE_BUTTON(w)) {
    /*
     * add/remove jvar to/from the subset of variables that <may> be active
    */

    changed = tour2d3_subset_var_set(jvar, button, d, dsp, gg);
    if (changed) {
      varcircles_visibility_set (dsp, gg);

      /*-- now add/remove the variable to/from the active set, too --*/
      tour2d3_active_var_set (jvar, d, dsp, gg);
    }

  } else if (GTK_IS_DRAWING_AREA(w)) {
    
    if (d->vcirc_ui.jcursor == GDK_HAND2) {
      tour2d3_manip_var_set (jvar, gg);
      varcircles_cursor_set_default (d);
    }

/*  There's no such thing as setting variables active or inactive
    by clicking on the variable circles for this mode.
    } else {
      tour2d3_active_var_set (jvar, d, dsp, gg);
    }
*/
  }

  return changed;
}

void
tour2d3_projdata(splotd *sp, greal **world_data, datad *d, ggobid *gg)
{
  gint i, j, m;
  displayd *dsp = (displayd *) sp->displayptr;
  greal precis = (greal) PRECISION1;
  greal tmpf, maxx, maxy;

  if (sp->tour2d3.initmax) {
    sp->tour2d3.maxscreen = precis;
    sp->tour2d3.initmax = false;
  }

  tmpf = precis/sp->tour2d3.maxscreen;
  maxx = sp->tour2d3.maxscreen;
  maxy = sp->tour2d3.maxscreen;
  for (m=0; m<d->nrows_in_plot; m++)
  {
    i = d->rows_in_plot[m];
    sp->planar[i].x = 0;
    sp->planar[i].y = 0;
    for (j=0; j<d->ncols; j++)
    {
      sp->planar[i].x += (greal)(dsp->t2d3.F.vals[0][j]*world_data[i][j]);
      sp->planar[i].y += (greal)(dsp->t2d3.F.vals[1][j]*world_data[i][j]);
    }
    sp->planar[i].x *= tmpf;
    sp->planar[i].y *= tmpf;
    if (fabs(sp->planar[i].x) > maxx)
      maxx = fabs(sp->planar[i].x);
    if (fabs(sp->planar[i].y) > maxy)
      maxy = fabs(sp->planar[i].y);
  }

  if ((maxx > precis) || (maxy > precis)) {
    sp->tour2d3.maxscreen = (maxx > maxy) ? maxx : maxy;
    tmpf = precis/tmpf;
  }
}

void tour2d3_scramble(ggobid *gg)
{
  int i, j;
  displayd *dsp = gg->current_display;
  datad *d = dsp->d;
  gint nc = d->ncols;

  for (i=0; i<2; i++)
    for (j=0; j<nc; j++)
      dsp->t2d3.Fa.vals[i][j] = dsp->t2d3.Fz.vals[i][j] = 
        dsp->t2d3.F.vals[i][j] = dsp->t2d3.Ga.vals[i][j] = 
        dsp->t2d3.Gz.vals[i][j] = 0.0;

  gt_basis(dsp->t2d3.Fa, dsp->t2d3.nactive, dsp->t2d3.active_vars, 
    d->ncols, (gint) 2);
  arrayd_copy(&dsp->t2d3.Fa, &dsp->t2d3.F);

  dsp->t2d3.tau.els[0] = 0.0;
  dsp->t2d3.tau.els[1] = 0.0;
  dsp->t2d3.get_new_target = true;

  display_tailpipe (dsp, FULL, gg);

  varcircles_refresh (d, gg);
}

void
tour2d3_run(displayd *dsp, ggobid *gg)
{
  datad *d = dsp->d;
  gint i, nv;
  gint k;
  gboolean chosen;
  gfloat eps = .01;
  gint pathprob = 0;

  if (!dsp->t2d3.get_new_target && 
      !reached_target(dsp->t2d3.tang, dsp->t2d3.dist_az,
       dsp->t2d3.target_selection_method, &dsp->t2d3.ppval, &dsp->t2d3.oppval))
  {
    increment_tour(dsp->t2d3.tinc, dsp->t2d3.tau, dsp->t2d3.dist_az, 
      dsp->t2d3.delta, &dsp->t2d3.tang, (gint) 2);
    tour_reproject(dsp->t2d3.tinc, dsp->t2d3.G, dsp->t2d3.Ga, dsp->t2d3.Gz, 
      dsp->t2d3.F, dsp->t2d3.Va, d->ncols, (gint) 2);

  }
  else { /* do final clean-up and get new target */
    if (dsp->t2d3.tau.els[0] > 0.0 || dsp->t2d3.tau.els[1] > 0.0) {
      do_last_increment(dsp->t2d3.tinc, dsp->t2d3.tau, 
        dsp->t2d3.dist_az, (gint) 2);
      tour_reproject(dsp->t2d3.tinc, dsp->t2d3.G, dsp->t2d3.Ga,
        dsp->t2d3.Gz,
        dsp->t2d3.F, dsp->t2d3.Va, d->ncols, (gint) 2);
    }

    nv = 0;
    for (i=0; i<d->ncols; i++) {
      chosen = false;
      for (k=0; k<dsp->t2d3.nactive; k++) {
        if (dsp->t2d3.active_vars.els[k] == i) {
          chosen = true;
          break;
        }
      }
      if (!chosen) {
        if (fabs(dsp->t2d3.F.vals[0][i]) < eps && 
          fabs(dsp->t2d3.F.vals[1][i]) < eps)
          dsp->t2d3.F.vals[0][i] = dsp->t2d3.F.vals[1][i] = 0.0;
        if (fabs(dsp->t2d3.F.vals[0][i]) > eps || 
          fabs(dsp->t2d3.F.vals[1][i]) > eps)
        {
          nv++;
        }
      }
    }
    arrayd_copy(&dsp->t2d3.F, &dsp->t2d3.Fa);
    if (nv == 0 && dsp->t2d3.nactive <= 2) /* only generate new dir if num of
                                           active/used variables is > 2 -
                                           this code allows for motion to
                                           continue while a variable is 
                                           fading out. */
      dsp->t2d3.get_new_target = true;
    else {
      gt_basis(dsp->t2d3.Fz, dsp->t2d3.nactive, dsp->t2d3.active_vars, 
        d->ncols, (gint) 2);

      pathprob = path(dsp->t2d3.Fa, dsp->t2d3.Fz, dsp->t2d3.F, d->ncols, 
        (gint) 2, dsp->t2d3.Ga,
        dsp->t2d3.Gz, dsp->t2d3.G, dsp->t2d3.lambda, dsp->t2d3.tv, dsp->t2d3.Va,
        dsp->t2d3.Vz, dsp->t2d3.tau, dsp->t2d3.tinc, 
        &dsp->t2d3.dist_az, &dsp->t2d3.tang);
      if (pathprob == 0) 
        dsp->t2d3.get_new_target = false;
      else if (pathprob == 1) { /* problems with Fa so need to force a jump */
        tour2d3_scramble(gg);
        pathprob = path(dsp->t2d3.Fa, dsp->t2d3.Fz, dsp->t2d3.F, d->ncols, 
          (gint) 2, dsp->t2d3.Ga,
          dsp->t2d3.Gz, dsp->t2d3.G, dsp->t2d3.lambda, dsp->t2d3.tv,
          dsp->t2d3.Va,
          dsp->t2d3.Vz, dsp->t2d3.tau, dsp->t2d3.tinc, 
          &dsp->t2d3.dist_az, &dsp->t2d3.tang);
      }
      else if (pathprob == 2 || pathprob == 3) { /* problems with Fz,
                                    so will force a new choice of Fz */
        dsp->t2d3.get_new_target = true;
      }
    }
  }
  
  display_tailpipe (dsp, FULL_1PIXMAP, gg);
  varcircles_refresh (d, gg);
}

void
tour2d3_do_step(displayd *dsp, ggobid *gg)
{
  tour2d3_run(dsp, gg);
}

gint
tour2d3_idle_func (displayd *dsp)
{
  ggobid *gg = GGobiFromDisplay (dsp);
  cpaneld *cpanel = &dsp->cpanel;
  gboolean doit = !cpanel->t2d3.paused;

  if (doit) {
    tour2d3_run (dsp, gg);
    gdk_flush ();
  }

  return (doit);
}

void tour2d3_func (gboolean state, displayd *dsp, ggobid *gg)
{
  if (state) {
    if (dsp->t2d3.idled == 0) {
      dsp->t2d3.idled = gtk_idle_add_priority (G_PRIORITY_LOW,
                                   (GtkFunction) tour2d3_idle_func, dsp);
    }
    gg->tour2d3.idled = 1;
  } else {
    if (dsp->t2d3.idled != 0) {
      gtk_idle_remove (dsp->t2d3.idled);
      dsp->t2d3.idled = 0;
    }
    gg->tour2d3.idled = 0;
  }
}

void tour2d3_reinit(ggobid *gg)
{
  int i, j;
  displayd *dsp = gg->current_display;
  datad *d = dsp->d;
  gint nc = d->ncols;
  splotd *sp = gg->current_splot;

  for (i=0; i<2; i++)
    for (j=0; j<nc; j++)
      dsp->t2d3.Fa.vals[i][j] = dsp->t2d3.Fz.vals[i][j] = 
        dsp->t2d3.F.vals[i][j] = dsp->t2d3.Ga.vals[i][j] = 
        dsp->t2d3.Gz.vals[i][j] = 0.0;

  for (i=0; i<2; i++) {
    dsp->t2d3.Fz.vals[i][dsp->t2d3.active_vars.els[i]] =
      dsp->t2d3.Fa.vals[i][dsp->t2d3.active_vars.els[i]] = 
      dsp->t2d3.F.vals[i][dsp->t2d3.active_vars.els[i]] =
      dsp->t2d3.Ga.vals[i][dsp->t2d3.active_vars.els[i]] = 
      dsp->t2d3.Gz.vals[i][dsp->t2d3.active_vars.els[i]] = 1.0;
  }

  dsp->t2d3.tau.els[0] = 0.0;
  dsp->t2d3.tau.els[1] = 0.0;
  dsp->t2d3.get_new_target = true;
  sp->tour2d3.initmax = true;

  display_tailpipe (dsp, FULL, gg);
  varcircles_refresh (d, gg);
}

/* Variable manipulation */
void
tour2d3_manip_init(gint p1, gint p2, splotd *sp) 
{
  displayd *dsp = (displayd *) sp->displayptr;
  datad *d = dsp->d;
  cpaneld *cpanel = &dsp->cpanel;
  ggobid *gg = GGobiFromSPlot(sp);
  gint j, k;
  gint n1vars = dsp->t2d3.nactive;
  gfloat tol = 0.05; 
  gdouble dtmp1;

  /* need to turn off tour */
  if (!cpanel->t2d3.paused)
    tour2d3_func(T2D3OFF, gg->current_display, gg);

  /* If de-selected variables are still fading out of the tour
     we will need to take them out before starting manipulation */
  for (j=0; j<d->ncols; j++)
    if (dsp->t2d3.active_vars_p.els[j] == false) {
       if (dsp->t2d3.F.vals[0][j] > 0.0) 
         dsp->t2d3.F.vals[0][j] = 0.0;
       if (dsp->t2d3.F.vals[1][j] > 0.0)
         dsp->t2d3.F.vals[1][j] = 0.0;
    }
  norm(dsp->t2d3.F.vals[0],d->ncols);
  norm(dsp->t2d3.F.vals[1],d->ncols);
  if (!gram_schmidt(dsp->t2d3.F.vals[0], dsp->t2d3.F.vals[1],
    d->ncols))
#ifdef EXCEPTION_HANDLING
    g_printerr("");/*t2d3.F[0] equivalent to t2d3.F[1]\n");*/
#else
      ;
#endif
  
  dsp->t2d3_manipvar_inc = false;
  dsp->t2d3_pos1 = dsp->t2d3_pos1_old = p1;
  dsp->t2d3_pos2 = dsp->t2d3_pos2_old = p2;
  /* check if manip var is one of existing vars */
  /* n1vars, n2vars is the number of variables, excluding the
     manip var in hor and vert directions */
  for (j=0; j<dsp->t2d3.nactive; j++)
    if (dsp->t2d3.active_vars.els[j] == dsp->t2d3_manip_var) {
      dsp->t2d3_manipvar_inc = true;
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
      dsp->t2d3_manbasis.vals[0][j] = dsp->t2d3.F.vals[0][j];
      dsp->t2d3_manbasis.vals[1][j] = dsp->t2d3.F.vals[1][j];
      dsp->t2d3_manbasis.vals[2][j] = 0.;
    }
    dsp->t2d3_manbasis.vals[2][dsp->t2d3_manip_var] = 1.;

    for (j=0; j<3; j++) {
      for (k=0; k<3; k++)
        dsp->t2d3_mvar_3dbasis.vals[j][k] = 0.;
      dsp->t2d3_mvar_3dbasis.vals[j][j] = 1.;
    }

    norm(dsp->t2d3_manbasis.vals[0], d->ncols); /* this is just in case */
    norm(dsp->t2d3_manbasis.vals[1], d->ncols); /* it seems to work ok */
    norm(dsp->t2d3_manbasis.vals[2], d->ncols); /* without normalizing here */

    /* Check if column 3 (2) of manbasis is effectively equal to 
       column 1 (0) or 2(1). If they are then we'll have to randomly
       generate a new column 3. If not then we orthonormalize column 3
       on the other two. */
    while (!gram_schmidt(dsp->t2d3_manbasis.vals[0], 
           dsp->t2d3_manbasis.vals[2], d->ncols))
    {
      gt_basis(dsp->t2d3.tv, dsp->t2d3.nactive, dsp->t2d3.active_vars, 
        d->ncols, (gint) 1);
      for (j=0; j<d->ncols; j++) 
        dsp->t2d3_manbasis.vals[2][j] = dsp->t2d3.tv.vals[0][j];
    }
    while (!gram_schmidt(dsp->t2d3_manbasis.vals[1], 
           dsp->t2d3_manbasis.vals[2], d->ncols))
    {
      gt_basis(dsp->t2d3.tv, dsp->t2d3.nactive, dsp->t2d3.active_vars, 
        d->ncols, (gint) 1);
      for (j=0; j<d->ncols; j++) 
        dsp->t2d3_manbasis.vals[2][j] = dsp->t2d3.tv.vals[0][j];
    }
    while (!gram_schmidt(dsp->t2d3_manbasis.vals[0],
           dsp->t2d3_manbasis.vals[1], d->ncols))
    {
      gt_basis(dsp->t2d3.tv, dsp->t2d3.nactive, dsp->t2d3.active_vars, 
        d->ncols, (gint) 1);
      for (j=0; j<d->ncols; j++) 
        dsp->t2d3_manbasis.vals[1][j] = dsp->t2d3.tv.vals[0][j];
    }

    /* This is innocuous, if the vectors are orthnormal nothing gets changed.
       But it protects against the case when vectors 0,1 were not
       orthonormal and a new vector 1 was generated, it checks the o.n.
       of all 3 vectors again. */
    gram_schmidt(dsp->t2d3_manbasis.vals[0],  dsp->t2d3_manbasis.vals[1],
      d->ncols);
    gram_schmidt(dsp->t2d3_manbasis.vals[0],  dsp->t2d3_manbasis.vals[2],
      d->ncols);
    gram_schmidt(dsp->t2d3_manbasis.vals[1],  dsp->t2d3_manbasis.vals[2],
      d->ncols);

    /*    ftmp = 0.0;
    while (ftmp < tol) {
    if ((fabs(inner_prod(dsp->t2d3_manbasis.vals[0],dsp->t2d3_manbasis.vals[2],
       d->ncols))>1.0-tol) || 
       (fabs(inner_prod(dsp->t2d3_manbasis.vals[1],
       dsp->t2d3_manbasis.vals[2],d->ncols))>1.0-tol))
    {
      gt_basis(dsp->t2d3.tv, dsp->t2d3.nactive, dsp->t2d3.active_vars, 
        d->ncols, (gint) 1);
      for (j=0; j<d->ncols; j++) 
        dsp->t2d3_manbasis.vals[2][j] = dsp->t2d3.tv.vals[0][j];
      g_printerr("0 manbasis2: ");
        for (i=0; i<3; i++)
          g_printerr("%f ",dsp->t2d3_manbasis.vals[2][i]);
      g_printerr("\n");
      if (!gram_schmidt(dsp->t2d3_manbasis.vals[0],  dsp->t2d3_manbasis.vals[2],
        d->ncols)) 
        g_printerr("t2d3_manbasis[0] equivalent to t2d3_manbasis[2]\n");
      if (!gram_schmidt(dsp->t2d3_manbasis.vals[1],  dsp->t2d3_manbasis.vals[2],
        d->ncols))
        g_printerr("t2d3_manbasis[1] equivalent to t2d3_manbasis[2]\n");

        g_printerr("1 manbasis0: ");
        for (i=0; i<3; i++)
          g_printerr("%f ",dsp->t2d3_manbasis.vals[0][i]);
        g_printerr("\n");
        g_printerr("1 manbasis1: ");
        for (i=0; i<3; i++)
          g_printerr("%f ",dsp->t2d3_manbasis.vals[1][i]);
        g_printerr("\n");
        g_printerr("1 manbasis2: ");
        for (i=0; i<3; i++)
          g_printerr("%f ",dsp->t2d3_manbasis.vals[2][i]);
        g_printerr("\n");
      ftmp = calc_norm (dsp->t2d3_manbasis.vals[2], d->ncols);
    }
    else if (fabs(inner_prod(dsp->t2d3_manbasis.vals[0],
      dsp->t2d3_manbasis.vals[1],d->ncols))>1.0-tol) 
    {
      printf("1 = 0\n");
      gt_basis(dsp->t2d3.tv, dsp->t2d3.nactive, dsp->t2d3.active_vars, 
        d->ncols, (gint) 1);
      for (j=0; j<d->ncols; j++) 
        dsp->t2d3_manbasis.vals[1][j] = dsp->t2d3.tv.vals[0][j];
      if (!gram_schmidt(dsp->t2d3_manbasis.vals[0],  dsp->t2d3_manbasis.vals[1],
		   d->ncols))
        g_printerr("t2d3_manbasis[0] equivalent to t2d3_manbasis[1]\n"); * this might not be necessary *
      if (!gram_schmidt(dsp->t2d3_manbasis.vals[0],  dsp->t2d3_manbasis.vals[2],
        d->ncols))
        g_printerr("t2d3_manbasis[0] equivalent to t2d3_manbasis[2]\n");
      if (!gram_schmidt(dsp->t2d3_manbasis.vals[1],  dsp->t2d3_manbasis.vals[2],
        d->ncols))
        g_printerr("t2d3_manbasis[1] equivalent to t2d3_manbasis[2]\n");
      ftmp = calc_norm (dsp->t2d3_manbasis.vals[1], d->ncols);
    }      
    else {
      printf("ok\n");
      if (!gram_schmidt(dsp->t2d3_manbasis.vals[0],  dsp->t2d3_manbasis.vals[2],
        d->ncols))
        g_printerr("t2d3_manbasis[0] equivalent to t2d3_manbasis[2]\n");
      if (!gram_schmidt(dsp->t2d3_manbasis.vals[1],  dsp->t2d3_manbasis.vals[2],
        d->ncols))
        g_printerr("t2d3_manbasis[1] equivalent to t2d3_manbasis[2]\n");
      ftmp = calc_norm (dsp->t2d3_manbasis.vals[2], d->ncols);
    }
    }*/

    /*    while (ftmp < tol) {
	  }*/

    dsp->t2d3_no_dir_flag = false;
    if (cpanel->t2d3.manip_mode == MANIP_RADIAL)
    { /* check if variable is currently visible in plot */
      if ((dsp->t2d3.F.vals[0][dsp->t2d3_manip_var]*
        dsp->t2d3.F.vals[0][dsp->t2d3_manip_var] +
        dsp->t2d3.F.vals[1][dsp->t2d3_manip_var]*
        dsp->t2d3.F.vals[1][dsp->t2d3_manip_var]) < tol)
        dsp->t2d3_no_dir_flag = true; /* no */
      else
      { /* yes: set radial manip direction to be current direction
             of contribution */
        dsp->t2d3_rx = (gfloat) dsp->t2d3.F.vals[0][dsp->t2d3_manip_var];
        dsp->t2d3_ry = (gfloat) dsp->t2d3.F.vals[1][dsp->t2d3_manip_var];
        dtmp1 = sqrt(dsp->t2d3_rx*dsp->t2d3_rx+dsp->t2d3_ry*dsp->t2d3_ry);
        dsp->t2d3_rx /= dtmp1;
        dsp->t2d3_ry /= dtmp1;
      }
    }
  }

}

void
tour2d3_manip(gint p1, gint p2, splotd *sp, ggobid *gg) 
{
  displayd *dsp = (displayd *) sp->displayptr;
  datad *d = dsp->d;
  cpaneld *cpanel = &dsp->cpanel;
  gint actual_nvars = dsp->t2d3.nactive;
  gboolean offscreen = false;
  gfloat phi, cosphi, sinphi, ca, sa, cosm, cospsi, sinpsi;
  gfloat distx, disty, x1, x2, y1, y2;
  gfloat denom = (gfloat) MIN(sp->max.x, sp->max.y)/2.;
  gfloat tol = 0.01;
  gdouble dtmp1, dtmp2;
  gfloat len_motion;
  gint i,j,k;

  /* check if off the plot window */
  if (p1 > sp->max.x || p1 < 0 ||
      p2 > sp->max.y || p2 < 0)
    offscreen = true;

  if (dsp->t2d3_manipvar_inc)
    actual_nvars = dsp->t2d3.nactive-1;

  if (!offscreen) {
    dsp->t2d3_pos1_old = dsp->t2d3_pos1;
    dsp->t2d3_pos2_old = dsp->t2d3_pos2;
  
    dsp->t2d3_pos1 = p1;
    dsp->t2d3_pos2 = p2;

    if (actual_nvars > 1)
    {
      if (cpanel->t2d3.manip_mode != MANIP_ANGULAR)
      {
        if (cpanel->t2d3.manip_mode == MANIP_OBLIQUE) 
        {
          distx = dsp->t2d3_pos1 - dsp->t2d3_pos1_old;
          disty = dsp->t2d3_pos2_old - dsp->t2d3_pos2;
        }
        else if (cpanel->t2d3.manip_mode == MANIP_VERT) 
        {
          distx = 0.;
          disty = dsp->t2d3_pos2_old - dsp->t2d3_pos2;
        }
        else if (cpanel->t2d3.manip_mode == MANIP_HOR) 
        {
          distx = dsp->t2d3_pos1 - dsp->t2d3_pos1_old;
          disty = 0.;
        }
        else if (cpanel->t2d3.manip_mode == MANIP_RADIAL) 
        {
          if (dsp->t2d3_no_dir_flag)
          {
            distx = dsp->t2d3_pos1 - dsp->t2d3_pos1_old;
            disty = dsp->t2d3_pos2_old - dsp->t2d3_pos2;
            dsp->t2d3_rx = distx;
            dsp->t2d3_ry = disty; 
            dtmp1 = sqrt(dsp->t2d3_rx*dsp->t2d3_rx+dsp->t2d3_ry*dsp->t2d3_ry);
            dsp->t2d3_rx /= dtmp1;
            dsp->t2d3_ry /= dtmp1;
            dsp->t2d3_no_dir_flag = false;
          }
          distx = (dsp->t2d3_rx*(dsp->t2d3_pos1 - dsp->t2d3_pos1_old) + 
            dsp->t2d3_ry*(dsp->t2d3_pos2_old - dsp->t2d3_pos2))*dsp->t2d3_rx;
          disty = (dsp->t2d3_rx*(dsp->t2d3_pos1 - dsp->t2d3_pos1_old) + 
            dsp->t2d3_ry*(dsp->t2d3_pos2_old - dsp->t2d3_pos2))*dsp->t2d3_ry;
        }
        dtmp1 = (gdouble) (distx*distx+disty*disty);
        len_motion = (gfloat) sqrt(dtmp1);

        if (len_motion < tol) /* just in case, maybe not necessary */
        {
          dsp->t2d3_Rmat2.vals[0][0] = 1.0;
          dsp->t2d3_Rmat2.vals[0][1] = 0.0;
          dsp->t2d3_Rmat2.vals[0][2] = 0.0;
          dsp->t2d3_Rmat2.vals[1][0] = 0.0;
          dsp->t2d3_Rmat2.vals[1][1] = 1.0;
          dsp->t2d3_Rmat2.vals[1][2] = 0.0;
          dsp->t2d3_Rmat2.vals[2][0] = 0.0;
          dsp->t2d3_Rmat2.vals[2][1] = 0.0;
          dsp->t2d3_Rmat2.vals[2][2] = 1.0;
        }
        else
        {
          phi = len_motion / denom;
     
          ca = distx/len_motion;
          sa = disty/len_motion;
      
          cosphi = (gfloat) cos((gdouble) phi);
          sinphi = (gfloat) sin((gdouble) phi);
          cosm = 1.0 - cosphi;
          dsp->t2d3_Rmat2.vals[0][0] = ca*ca*cosphi + sa*sa;
          dsp->t2d3_Rmat2.vals[0][1] = -cosm*ca*sa;
          dsp->t2d3_Rmat2.vals[0][2] = sinphi*ca;
          dsp->t2d3_Rmat2.vals[1][0] = -cosm*ca*sa;
          dsp->t2d3_Rmat2.vals[1][1] = sa*sa*cosphi + ca*ca;
          dsp->t2d3_Rmat2.vals[1][2] = sinphi*sa;
          dsp->t2d3_Rmat2.vals[2][0] = -sinphi*ca;
          dsp->t2d3_Rmat2.vals[2][1] = -sinphi*sa;
          dsp->t2d3_Rmat2.vals[2][2] = cosphi;
        }
      }
      else 
      { /* angular constrained manipulation */
        if (dsp->t2d3_pos1_old != sp->max.x/2 && 
          dsp->t2d3_pos2_old != sp->max.y/2 &&
          dsp->t2d3_pos1 != sp->max.x/2 && 
          dsp->t2d3_pos2 != sp->max.y/2)
        {
          x1 = dsp->t2d3_pos1_old - sp->max.x/2;
          y1 = dsp->t2d3_pos2_old - sp->max.y/2;
          dtmp1 = sqrt(x1*x1+y1*y1);
          x1 /= dtmp1;
          y1 /= dtmp1;
          x2 = dsp->t2d3_pos1 - sp->max.x/2;
          y2 = dsp->t2d3_pos2 - sp->max.y/2;
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
        dsp->t2d3_Rmat2.vals[0][0] = cospsi;
        dsp->t2d3_Rmat2.vals[0][1] = sinpsi;
        dsp->t2d3_Rmat2.vals[0][2] = 0.;
        dsp->t2d3_Rmat2.vals[1][0] = -sinpsi;
        dsp->t2d3_Rmat2.vals[1][1] = cospsi;
        dsp->t2d3_Rmat2.vals[1][2] = 0.;
        dsp->t2d3_Rmat2.vals[2][0] = 0.;
        dsp->t2d3_Rmat2.vals[2][1] = 0.;
        dsp->t2d3_Rmat2.vals[2][2] = 1.;
      }

      /* Set up the rotation matrix in the 3D manip space */
      for (i=0; i<3; i++) 
        for (j=0; j<3; j++)
        {
          dtmp1 = 0.;
          for (k=0; k<3; k++)
            dtmp1 += (dsp->t2d3_mvar_3dbasis.vals[i][k]*
              dsp->t2d3_Rmat2.vals[k][j]);
          dsp->t2d3_Rmat1.vals[i][j] = dtmp1;
        }
      arrayd_copy(&dsp->t2d3_Rmat1, &dsp->t2d3_mvar_3dbasis);

      norm(dsp->t2d3_mvar_3dbasis.vals[0],3); /* just in case */
      norm(dsp->t2d3_mvar_3dbasis.vals[1],3); /* seems to work ok without */
      norm(dsp->t2d3_mvar_3dbasis.vals[2],3); /* this */
      if (!gram_schmidt(dsp->t2d3_mvar_3dbasis.vals[0], 
        dsp->t2d3_mvar_3dbasis.vals[1], 3))
#ifdef EXCEPTION_HANDLING
        g_printerr("");/*t2d3_mvar[0] equivalent to t2d3_mvar[1]\n");*/
#else
        ;
#endif
      if (!gram_schmidt(dsp->t2d3_mvar_3dbasis.vals[0], 
        dsp->t2d3_mvar_3dbasis.vals[2], 3))
#ifdef EXCEPTION_HANDLING
          g_printerr("");/*t2d3_mvar[0] equivalent to t2d3_mvar[2]\n");*/
#else
          ;
#endif
      if (!gram_schmidt(dsp->t2d3_mvar_3dbasis.vals[1], 
        dsp->t2d3_mvar_3dbasis.vals[2], 3))
#ifdef EXCEPTION_HANDLING
        g_printerr("");/*t2d3_mvar[1] equivalent to t2d3_mvar[2]\n");*/
#else
          ;
#endif

      /* Generate the projection of the data corresponding to 
         the 3D rotation in the manip space. */
      for (j=0; j<d->ncols; j++)
      {
        dsp->t2d3.F.vals[0][j] = 
          dsp->t2d3_manbasis.vals[0][j]*dsp->t2d3_mvar_3dbasis.vals[0][0] +
          dsp->t2d3_manbasis.vals[1][j]*dsp->t2d3_mvar_3dbasis.vals[0][1] +
          dsp->t2d3_manbasis.vals[2][j]*dsp->t2d3_mvar_3dbasis.vals[0][2];
        dsp->t2d3.F.vals[1][j] = 
          dsp->t2d3_manbasis.vals[0][j]*dsp->t2d3_mvar_3dbasis.vals[1][0] +
          dsp->t2d3_manbasis.vals[1][j]*dsp->t2d3_mvar_3dbasis.vals[1][1] +
          dsp->t2d3_manbasis.vals[2][j]*dsp->t2d3_mvar_3dbasis.vals[1][2];
      }
      norm(dsp->t2d3.F.vals[0], d->ncols);
      norm(dsp->t2d3.F.vals[1], d->ncols);
      /*      if (calc_norm(dsp->t2d3.F.vals[0], d->ncols)>1.01) {
	g_printerr("1 F0 out of bounds\n");
        g_printerr("F0: ");
        for (i=0; i<3; i++)
          g_printerr("%f ",dsp->t2d3.F.vals[0][i]);
        g_printerr("\n");
        g_printerr("F1: ");
        for (i=0; i<3; i++)
          g_printerr("%f ",dsp->t2d3.F.vals[1][i]);
        g_printerr("\n");
        g_printerr("manbasis0: ");
        for (i=0; i<3; i++)
          g_printerr("%f ",dsp->t2d3_manbasis.vals[0][i]);
        g_printerr("\n");
        g_printerr("manbasis1: ");
        for (i=0; i<3; i++)
          g_printerr("%f ",dsp->t2d3_manbasis.vals[1][i]);
        g_printerr("\n");
        g_printerr("manbasis2: ");
        for (i=0; i<3; i++)
          g_printerr("%f ",dsp->t2d3_manbasis.vals[2][i]);
        g_printerr("\n");
        g_printerr("m3dvar0: ");
        for (i=0; i<3; i++)
          g_printerr("%f ",dsp->t2d3_mvar_3dbasis.vals[0][i]);
        g_printerr("\n");
        g_printerr("m3dvar1: ");
        for (i=0; i<3; i++)
          g_printerr("%f ",dsp->t2d3_mvar_3dbasis.vals[1][i]);
        g_printerr("\n");
        g_printerr("m3dvar2: ");
        for (i=0; i<3; i++)
          g_printerr("%f ",dsp->t2d3_mvar_3dbasis.vals[2][i]);
        g_printerr("\n");
        g_printerr("distx %f disty %f\n",distx,disty);
      }
      if (calc_norm(dsp->t2d3.F.vals[1], d->ncols)>1.01) 
      g_printerr("1 F1 out of bounds\n");*/
      if (!gram_schmidt(dsp->t2d3.F.vals[0], dsp->t2d3.F.vals[1], d->ncols))
#ifdef EXCEPTION_HANDLING
        g_printerr("");/*t2d3.F[0] equivalent to t2d3.F[2]\n");*/
#else
        ;
#endif

      /*      if (calc_norm(dsp->t2d3.F.vals[0], d->ncols)>1.0) 
	g_printerr("F0 out of bounds\n");
      if (calc_norm(dsp->t2d3.F.vals[1], d->ncols)>1.0) 
	g_printerr("F1 out of bounds\n");
      */
    }

    display_tailpipe (dsp, FULL, gg);
    varcircles_refresh (d, gg);
  }
}

void
tour2d3_manip_end(splotd *sp) 
{
  displayd *dsp = (displayd *) sp->displayptr;
  cpaneld *cpanel = &dsp->cpanel;
  ggobid *gg = GGobiFromSPlot(sp);

  disconnect_motion_signal (sp);

  arrayd_copy(&dsp->t2d3.F, &dsp->t2d3.Fa);
  dsp->t2d3.get_new_target = true;

  /* need to turn on tour? */
  if (!cpanel->t2d3.paused) {
    tour2d3_func (T2D3ON, gg->current_display, gg);

    /*-- whenever motion stops, we need a FULL redraw --*/
    display_tailpipe (gg->current_display, FULL, gg);
  }
}

#undef T2D3ON
#undef T2D3OFF

#endif
