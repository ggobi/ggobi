/* p1d.c */
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/
/*
 * The 1d plots use the values of world_data (or missing_world_data)
 * for the variable of interest, but they use tform (or missing)
 * to do the spreading calculations, which are done in floating point.
 *
 * At the very end, to deal with jittering, they remove the jitter
 * from the selected variable and add it to the spread direction.
*/

#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"

/*
 * min and max for 'forget it' dotplot's texturing axis;
 * they're defined so as to locate the plot in the center of
 * the window, with the texturing values returned on a range
 * of [0,100].
 *
 * The min and max are calculated on the fly for the ash.
*/
#define FORGETITAXIS_MIN -100.
#define FORGETITAXIS_MAX 200.

RedrawStyle
p1d_activate (gint state, displayd *display, ggobid *gg)
{
  GList *slist;
  splotd *sp;
  datad *d = display->d;

  if (state) {
    for (slist = display->splots; slist; slist = slist->next) {
      sp = (splotd *) slist->data;
      if (sp->p1dvar >= d->ncols)
        sp->p1dvar = 0;
    }
    varpanel_refresh (display, gg);
  } else {
    /*
     * Turn cycling off when leaving the mode, but don't worry
     * for now about turning it on when re-entering.
    */
    GtkWidget *w = widget_find_by_name (gg->control_panel[P1PLOT], 
                                        "P1PLOT:cycle_toggle");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(w), off);
  }

  return NONE;
}   


void
p1d_spread_var (displayd *display, gfloat *yy, splotd *sp, datad *d,
  ggobid *gg)
{
/*
 * Set up the next dot plot.
*/
  gint i;
  gfloat del = 1.;
  gint option = 1, stages = 3;
  gfloat min, max, mean;
  cpaneld *cpanel = &display->cpanel;

  if (sp->p1d.spread_data.nels != d->nrows)
    vectorf_realloc (&sp->p1d.spread_data, d->nrows);

  switch (cpanel->p1d.type) {
    case TEXTURE:
      sp->p1d.lim.min = FORGETITAXIS_MIN ;
      sp->p1d.lim.max = FORGETITAXIS_MAX ;

      textur (yy, sp->p1d.spread_data.els, d->nrows_in_plot,
        option, del, stages, gg);
    break;

    case ASH:
      do_ash1d (yy, d->nrows_in_plot,
               cpanel->p1d.nbins, cpanel->p1d.nASHes,
               sp->p1d.spread_data.els, &min, &max, &mean);
      /*
       * Instead of using the returned minimum, set the minimum to 0.
       * This scales the plot so that the baseline (also set to 0) is
       * within the range, the connecting lines look terrific, and the
       * plot makes more sense.
      */
      sp->p1d.lim.min = 0.0; 
      sp->p1d.lim.max = max;
      sp->p1d.mean = mean;
    break;   

    case DOTPLOT:
      sp->p1d.lim.min = FORGETITAXIS_MIN ;
      sp->p1d.lim.max = FORGETITAXIS_MAX ;
      for (i=0; i<d->nrows_in_plot; i++)
        sp->p1d.spread_data.els[i] = 50; /*-- halfway between _MIN and _MAX --*/
    break;   
  }
}

void
p1d_reproject (splotd *sp, greal **world_data, datad *d, ggobid *gg)
{
/*
 * Project the y variable down from the ncols-dimensional world_data[]
 * to the 2-dimensional array planar[]; get the x variable directly
 * from p1d_data[].
*/
  gint i, m, jvar = 0;
  gfloat rdiff, ftmp;
  gfloat precis = PRECISION1;
  displayd *display = (displayd *) sp->displayptr;
  gfloat *yy;

  if (sp == NULL)
    return;

  yy = (gfloat *) g_malloc (d->nrows_in_plot * sizeof (gfloat));
  jvar = sp->p1dvar;

  /*
   * in order to have jittering in the direction of the variable
   * instead of in the direction of the "spreading variable", we
   * have to apply the ASH (in particular) after the jitter has
   * been added in.  That is, we have to ASH the world data instead
   * of the tform data.  By some unexpected miracle, all the scaling
   * still works.
  */
  for (i=0; i<d->nrows_in_plot; i++)
    yy[i] = d->world.vals[d->rows_in_plot.els[i]][jvar];
    /*yy[i] = d->tform.vals[d->rows_in_plot.els[i]][jvar];*/

  p1d_spread_var (display, yy, sp, d, gg);

  /* Then project it */
  rdiff = sp->p1d.lim.max - sp->p1d.lim.min;
  for (i=0; i<d->nrows_in_plot; i++) {
    m = d->rows_in_plot.els[i];

    /*
     * Use p1d_data[i] not [m] because p1d_data[] is populated
     * only up to d->nrows_in_plot
    */
    ftmp = -1.0 + 2.0*(sp->p1d.spread_data.els[i] - sp->p1d.lim.min)/rdiff;

    if (display->p1d_orientation == VERTICAL) {
      sp->planar[m].x = (glong) (precis * ftmp);
      sp->planar[m].y = (glong) world_data[m][jvar];
    } else {
      sp->planar[m].x = (glong) world_data[m][jvar];
      sp->planar[m].y = (glong) (precis * ftmp);
    }
  }

  g_free ((gpointer) yy);
}

gboolean
p1d_varsel (splotd *sp, gint jvar, gint *jprev, gint toggle, gint mouse)
{
  gboolean redraw = true;
  displayd *display = (displayd *) sp->displayptr;
  gint orientation = display->p1d_orientation;
  gboolean allow = true;

  if(GTK_IS_GGOBI_EXTENDED_DISPLAY(display)) {
     allow = GTK_GGOBI_EXTENDED_DISPLAY_CLASS(GTK_OBJECT(display)->klass)->allow_reorientation;
  }

  /*-- if button == -1, don't change orientation. That protects
       changes made during cycling --*/
  if (allow && mouse > 0) 
    display->p1d_orientation = (mouse == 1) ? HORIZONTAL : VERTICAL;

  redraw = (orientation != display->p1d_orientation) || (jvar != sp->p1dvar);

  *jprev = sp->p1dvar;
  sp->p1dvar = jvar;

  if (orientation != display->p1d_orientation)
    scatterplot_show_rulers (display, P1PLOT);

  return redraw;
}

/*---------------------------------------------------------------------*/

void
ash_baseline_set (icoords *baseline, splotd *sp)
{
  greal ftmp, precis = (greal) PRECISION1;
  greal pl, gtmp;
  gint iscr;

/*
  ftmp = -1 + 2.0 * (0 - sp->p1d.lim.min)/
                    (sp->p1d.lim.max - sp->p1d.lim.min);
*/
  ftmp = -1 /* and the rest of the usual expression is 0 now */;
  pl = (greal) (precis * ftmp);

/*-- HORIZONTAL --*/
  gtmp = pl - sp->pmid.y;
  iscr = (gint) (gtmp * sp->iscale.y / precis);
  iscr += (sp->max.y / 2);

  baseline->y = iscr;
            
/*-- VERTICAL --*/
  gtmp = pl - sp->pmid.x;
  iscr = (gint) (gtmp * sp->iscale.x / precis);
  iscr += (sp->max.x / 2);

  baseline->x = iscr;
}

/*--------------------------------------------------------------------*/
/*                            Cycling                                 */
/*--------------------------------------------------------------------*/


gint
p1dcycle_func (ggobid *gg)
{
  displayd *display = gg->current_display;
  datad *d = gg->current_display->d;
  splotd *sp = gg->current_splot;
  cpaneld *cpanel = &display->cpanel;

  gint varno, jvar_prev;

  if (cpanel->p1d.cycle_dir == 1) {
    varno = sp->p1dvar + 1;

    if (varno == d->ncols) {
      varno = 0;
    }
  } else {
    varno = sp->p1dvar - 1;

    if (varno < 0) {
      varno = d->ncols-1;
    }
  }

  if (varno != sp->p1dvar) {
    jvar_prev = sp->p1dvar;
    if (p1d_varsel (sp, varno, &jvar_prev, -1, -1)) {
      varpanel_refresh (display, gg);
      display_tailpipe (display, FULL, gg);
    }
  }
  
  return true;
}

