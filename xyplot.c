/*-- xyplot.c --*/
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
#include "vars.h"
#include "externs.h"

RedrawStyle
xyplot_activate (gint state, displayd *display, ggobid *gg)
{
  GList *slist;
  splotd *sp;
  datad *d = display->d;
  gboolean reset = false;

  if (state) {

    for (slist = display->splots; slist; slist = slist->next) {
      sp = (splotd *) slist->data;
      if (sp->xyvars.x >= d->ncols) {
        reset = true;
        sp->xyvars.x = (sp->xyvars.y == 0) ? 1 : 0;
      }
      if (sp->xyvars.y >= d->ncols) {
        reset = true;
        sp->xyvars.y = (sp->xyvars.x == 0) ? 1 : 0;
      }
    }
    if (reset)
      varpanel_refresh (gg);
  } else {
    /*
     * Turn cycling off when leaving the mode, but don't worry
     * about turning it on when re-entering.
    */
    GtkWidget *w = widget_find_by_name (gg->control_panel[XYPLOT], 
                                        "XYPLOT:cycle_toggle");
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(w), off);
  }

  return NONE;
}   

gboolean
xyplot_varsel (splotd *sp, gint jvar, gint *jvar_prev, gint button)
{
  gboolean redraw = true;

  if (button == 1) {
    if (jvar == sp->xyvars.x)
      redraw = false;
    else if (jvar == sp->xyvars.y) {
      sp->xyvars.y = sp->xyvars.x;
      *jvar_prev = sp->xyvars.x;
    } else {
      *jvar_prev = sp->xyvars.x;
    }
    sp->xyvars.x = jvar;
  } else if (button == 2 || button == 3) {
    if (jvar == sp->xyvars.y)
      redraw = false;
    else if (jvar == sp->xyvars.x) {
      sp->xyvars.x = sp->xyvars.y;
      *jvar_prev = sp->xyvars.y;
    } else {
      *jvar_prev = sp->xyvars.y;
    }
    sp->xyvars.y = jvar;
  }

  return redraw;
}

void
xy_reproject (splotd *sp, glong **world_data, datad *d, ggobid *gg)
{
/*
 * Project the data down from the ncols_used-dimensional world_data[]
 * to the 2-dimensional array planar[].
*/
  gint i, m;
  gint jx = sp->xyvars.x;
  gint jy = sp->xyvars.y;

  for (i=0; i<d->nrows; i++) {
    m = d->rows_in_plot[i];

    sp->planar[m].x = world_data[m][jx];  /*-- regular or missings --*/
    sp->planar[m].y = world_data[m][jy];
  }
}

/*--------------------------------------------------------------------*/
/*                            Cycling                                 */
/*--------------------------------------------------------------------*/

void
cycle_fixedx (splotd *sp, displayd *display, datad *d, ggobid *gg)
{
  cpaneld *cpanel = &display->cpanel;
  gint varno, jvar_prev;

  if (cpanel->xyplot.cycle_dir == 1) {
    varno = sp->xyvars.y + 1;

    if (varno == sp->xyvars.x)
       varno++;

    if (varno == d->ncols) {
      varno = 0;
      if (varno == sp->xyvars.x)
         varno++;
    }
  } else {
    varno = sp->xyvars.y - 1;

    if (varno == sp->xyvars.x)
       varno--;

    if (varno < 0) {
      varno = d->ncols-1;
      if (varno == sp->xyvars.x)
         varno--;
    }
  }

  if (varno != sp->xyvars.y) {
    jvar_prev = sp->xyvars.y;
    if (xyplot_varsel (sp, varno, &jvar_prev, 2)) {
      varpanel_refresh (gg);
      display_tailpipe (display, FULL, gg);
    }
  }
}

void
cycle_fixedy (splotd *sp, displayd *display, datad *d, ggobid *gg)
{
  cpaneld *cpanel = &display->cpanel;
  gint varno, jvar_prev;

  if (cpanel->xyplot.cycle_dir == 1) {
    varno = sp->xyvars.x + 1;

    if (varno == sp->xyvars.y)
       varno++;

    if (varno == d->ncols) {
      varno = 0;
      if (varno == sp->xyvars.y)
        varno++;
    }
  } else {
    varno = sp->xyvars.x - 1;

    if (varno == sp->xyvars.y)
       varno--;

    if (varno < 0) {
      varno = d->ncols-1;
      if (varno == sp->xyvars.y)
        varno--;
    }
  }

  if (varno != sp->xyvars.x) {
    jvar_prev = sp->xyvars.x;
    if (xyplot_varsel (sp, varno, &jvar_prev, 1))
      varpanel_refresh (gg);
      display_tailpipe (display, FULL, gg);
  }
}

/*
 * The question is:  cycle over all y vs x, ie, the entire
 * off-diagonal scatterplot matrix, or cycle over only one
 * triangle of the matrix.  It's more efficient just to use
 * the upper triangle, so let's try to make that work properly.
 *
 * y is always > x, unless we're starting from a projection that
 * was determined by variable selection, not by cycling.  (And
 * users can select plots during cycling, too.)
*/
void
cycle_xy (splotd *sp, displayd *display, datad *d, ggobid *gg)
{
  cpaneld *cpanel = &display->cpanel;
  gint jx, jy;
  gint jvar_prev;
  gboolean redraw = false;

  jx = sp->xyvars.x;
  jy = sp->xyvars.y;

  if (cpanel->xyplot.cycle_dir == 1) {

    /* case 1: x is maxed out. */
    if ((jx == d->ncols-1) || (jx == d->ncols-2 && jy == d->ncols-1) ) {
      jx = 0;
      jy = jx+1;
    /* 2: this can occur due to variable selection, but not due to cycling */
    } else if (jy < jx) {
      jy = jx+1;
    /* y is maxed out, but not x */
    } else if (jy == d->ncols-1) {
      jx++;
      jy = 0;
    } else jy++;

  } else {

    /* case 1: y is at a minimum, or x and y together are at a minimum */
    if ( jy == jx+1 ) {
      if (jx == 0) {
        jx = d->ncols - 2;
      } else {
        jx--;
      }
      jy = d->ncols - 1;
    /* 2: this can occur due to variable selection, but not due to cycling */
    } else if (jy < jx) {
      jy = d->ncols-1;
    /* 3: just decrement y */
    } else jy--;
  }

  if (jx != sp->xyvars.x) {
    jvar_prev = sp->xyvars.x;
    redraw = xyplot_varsel (sp, jx, &jvar_prev, 1);
  }
  if (jy != sp->xyvars.y) {
    jvar_prev = sp->xyvars.y;
    redraw = redraw | xyplot_varsel (sp, jy, &jvar_prev, 2);
  }

  if (redraw) {
    varpanel_refresh (gg);
    display_tailpipe (display, FULL, gg);
  }
}

gint
xycycle_func (ggobid *gg)
{
  displayd *display = gg->current_display;
  datad *d = gg->current_display->d;
  splotd *sp = gg->current_splot;
  cpaneld *cpanel = &display->cpanel;
  
  switch (cpanel->xyplot.cycle_axis) {
    case XFIXED:
      cycle_fixedx (sp, display, d, gg);
    break;
    case YFIXED:
      cycle_fixedy (sp, display, d, gg);
    break;
    default:
      cycle_xy (sp, display, d, gg);
  }

  return true;
}

