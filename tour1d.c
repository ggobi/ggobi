/* tour1d.c */

#include <gtk/gtk.h>
#ifdef USE_STRINGS_H
#include <strings.h>
#endif

#include <math.h>

#include "vars.h"
#include "externs.h"

void 
set_tour1dvar(ggobid *gg, gint jvar)
{
  gint j, jtmp, k;
  gboolean selected=false;
  displayd *dsp = gg->current_display;
  extern void zero_tau(displayd *, ggobid *);
  extern void zero_tinc(displayd *, ggobid *);
  extern void init_basis(displayd *, ggobid *);

  for (j=0; j<dsp->ntour_vars; j++)
    if (jvar == dsp->tour_vars.vals[j])
      selected = true;

  /* deselect var if ntour_vars > 2 */
  if (selected) {
    if (dsp->ntour_vars > 2) {
      for (j=0; j<dsp->ntour_vars; j++) {
        if (jvar == dsp->tour_vars.vals[j]) 
          break;
      }
      if (j<dsp->ntour_vars-1) {
        for (k=j; k<dsp->ntour_vars-1; k++){
          dsp->tour_vars.vals[k] = dsp->tour_vars.vals[k+1];
	}
      }
      dsp->ntour_vars--;
    }
  }
  else { /* not selected, so add the variable */
    if (jvar > dsp->tour_vars.vals[dsp->ntour_vars-1]) {
      dsp->tour_vars.vals[dsp->ntour_vars] = jvar;
    }
    else if (jvar < dsp->tour_vars.vals[0]) {
      for (j=dsp->ntour_vars; j>0; j--) {
          dsp->tour_vars.vals[j] = dsp->tour_vars.vals[j-1];
      }
      dsp->tour_vars.vals[0] = jvar;
    }
    else {
      for (j=0; j<dsp->ntour_vars-1; j++) {
        if (jvar > dsp->tour_vars.vals[j] && jvar < dsp->tour_vars.vals[j+1]) {
          jtmp = j+1;
          break;
	}
      }
      for (j=dsp->ntour_vars-1;j>=jtmp; j--) 
          dsp->tour_vars.vals[j+1] = dsp->tour_vars.vals[j];
      dsp->tour_vars.vals[jtmp] = jvar;
    }
    dsp->ntour_vars++;
  }

  dsp->tour_get_new_target = true;
}

void
tour1d_varsel (ggobid *gg, gint jvar, gint button)
{
  if (button == 1 || button == 2) {
    set_tour1dvar(gg, jvar);
  }
}

void
tour1d_projdata(splotd *sp, glong **world_data, datad *d, ggobid *gg) {
  int i, j, m;
  displayd *dsp = (displayd *) sp->displayptr;
  gfloat min, max, keepmin, keepmax;
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
      yy[i] += (gint)(dsp->u.vals[0][j]*world_data[i][j]);
    }
  }
  do_ash1d (yy, d->nrows_in_plot,
       cpanel->t1d_nbins, cpanel->t1d_nASHes,
       sp->p1d_data, &min, &max);
  if (firsttime) {
    keepmin = min;
    keepmax = max;
    firsttime = false;
  }
  else {
    if (min < keepmin) keepmin = min;
    if (max > keepmax) keepmax = max;
  }
  if (cpanel->t1d_vert) {
    for (i=0; i<d->nrows_in_plot; i++) {
      sp->planar[i].x = (glong) (precis*(-1.0+2.0*
        (sp->p1d_data[i]-keepmin)/(max-keepmin)));
      sp->planar[i].y = yy[i];
    }
  }
  else {
    for (i=0; i<d->nrows_in_plot; i++) {
      sp->planar[i].x = yy[i];
      sp->planar[i].y = (glong) (precis*(-1.0+2.0*
        (sp->p1d_data[i]-keepmin)/(max-keepmin)));
    }
  }

  g_free ((gpointer) yy);

}

void
tour1d_run(displayd *dsp, ggobid *gg)
{
  extern gboolean reached_target(displayd *);
  extern void increment_tour(displayd *, gint);
  extern void do_last_increment(displayd *, gint);
  extern void gt_basis(displayd *, ggobid *, gint);
  extern void path(displayd *, gint);
  extern void tour_reproject(displayd *, gint);
  extern void copy_mat(gfloat **, gfloat**, gint, gint);
  datad *d = dsp->d;
  gint i;

  /*  printf("u \n");
  for (i=0; i<d->ncols; i++)
    printf("%f ",dsp->u[0][i]);
  printf("\n");*/

  if (!dsp->tour_get_new_target && !reached_target(dsp)) {
    increment_tour(dsp, 1);
    tour_reproject(dsp, 1);
  }
  else { /* do final clean-up and get new target */
    if (!dsp->tour_get_new_target)
      do_last_increment(dsp, (gint) 1);
    copy_mat(dsp->u0.vals, dsp->u.vals, d->ncols, 1);
    tour_reproject(dsp, 1);
    gt_basis(dsp, gg, (gint) 1);
    path(dsp, (gint) 1);
    dsp->tour_get_new_target = false;
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
tour1d_idle_func (ggobid *gg)
{
  displayd *dsp = gg->current_display;
  cpaneld *cpanel = &dsp->cpanel;
  gboolean doit = !cpanel->tour_paused_p;

  if (doit) {
    tour1d_run(dsp, gg);
    gdk_flush ();
  }

  return (doit);
}

void tour1d_func (gboolean state, ggobid *gg)
{
  displayd *dsp = gg->current_display; 

  if (state) {
    dsp->tour_idled = gtk_idle_add_priority (G_PRIORITY_LOW,
                                   (GtkFunction) tour1d_idle_func, gg);
    gg->tour1d.idled = 1;
  } else {
    gtk_idle_remove (dsp->tour_idled);
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
    for (j=0; j<dsp->ntour_vars; j++) {
      dsp->u0.vals[i][j] = 0.;
      dsp->u.vals[i][j] = 0.;
    }
    dsp->u0.vals[i][i] = 1.;
    dsp->u.vals[i][i] = 1.;
  }

  dsp->tour_get_new_target = true;

}

void tour1d_vert(cpaneld *cpanel, gboolean state)
{
  cpanel->t1d_vert = state;
}


