#include "ggobi.h"
#include "scatterplotClass.h"
#include "externs.h"

#include <string.h>

#include "write_state.h"

#include <gtk/gtk.h>

/*
  display_datad_added_cb() in display.c
    scatterplot_wants to display_edge_menu_update()
    Hint that scatmat_edge... also needed.

  display_options_cb

  varpanel_highd needs a new method to be defined.

  viewmode_set needs a special handler. 

  motion_notify_cb in movepts_ui.c works only for scatterplot and
  scatterplot matrices
  Perhaps just don't register the event handler for other types.
  Same with button_press_cb

  world_to_raw needs methods.

display_data_added_cb should only be called for scatterplot guys.
  
   splot_draw_to_pixmap0_unbinned needs an additional pre-plot hook for drawing edges.

  tour1d tourcorr and tour2d realloc up's don't get done for anything but scatterplot. 
  
  varcircle_draw needs a hook.

  selectScatterplotX only applies to scatterplots.

*/

static void
setShowAxesLabelOption(displayd *display, gboolean active)
{
  if(display->cpanel.projection == TOUR2D)
     display_plot (display, FULL, display->ggobi);  
#ifdef ROTATION_IMPLEMENTED
  else if (display->cpanel.projection == TOUR2D3)
     display_plot (display, FULL, display->ggobi);  
#endif
}

static void
setShowAxesValuesOption(displayd *display, gboolean active)
{
  if(display->cpanel.projection == TOUR2D)
     display_plot (display, FULL, display->ggobi);  
#ifdef ROTATION_IMPLEMENTED
  else if (display->cpanel.projection == TOUR2D3)
     display_plot (display, FULL, display->ggobi);  
#endif
}

static void
setShowAxesOption(displayd *display, gboolean active)
{
  switch (display->cpanel.projection) {
    case XYPLOT:
      if (display->hrule != NULL) {
        scatterplot_show_vrule (display, active);
        scatterplot_show_hrule (display, active);
      }
    break;
    case P1PLOT:
      if (display->hrule != NULL) {
        if (display->p1d_orientation == VERTICAL)
          scatterplot_show_vrule (display, active);
        else
          scatterplot_show_hrule (display, active);
      }
    case TOUR1D:
#ifdef ROTATION_IMPLEMENTED
    case TOUR2D3:
#endif
    case TOUR2D:
    case COTOUR:
      display_plot (display, FULL, display->ggobi);
    break;
    default:
    break;
  }
}

static void
selectXVar(GtkWidget *w, displayd *display, gint jvar, ggobid *gg)
{
  datad *d = display->d;
  splotd *sp = (splotd *) display->splots->data;
  cpaneld *cpanel = &display->cpanel;

  varsel (w, cpanel, sp, jvar, VARSEL_X, -1, false, false, false, d, gg);
}

static void
varpanelRefresh(displayd *display, splotd *sp, datad *d)
{
  cpaneld *cpanel = &display->cpanel;
  gint j;

  switch (cpanel->projection) {
    case P1PLOT:
      for (j=0; j<d->ncols; j++) {
        varpanel_toggle_set_active (VARSEL_X, j, j == sp->p1dvar, d);

        varpanel_toggle_set_active (VARSEL_Y, j, false, d);
        varpanel_widget_set_visible (VARSEL_Y, j, false, d);
        varpanel_toggle_set_active (VARSEL_Z, j, false, d);
        varpanel_widget_set_visible (VARSEL_Z, j, false, d);
      }
    break;
    case XYPLOT:
      for (j=0; j<d->ncols; j++) {
        varpanel_toggle_set_active (VARSEL_X, j, 
          (j == sp->xyvars.x), d);
        varpanel_widget_set_visible (VARSEL_Y, j, true, d);
        varpanel_toggle_set_active (VARSEL_Y, j, 
          (j == sp->xyvars.y), d);

        varpanel_toggle_set_active (VARSEL_Z, j, false, d);
        varpanel_widget_set_visible (VARSEL_Z, j, false, d);
      }
    break;

    case TOUR1D:
      for (j=0; j<d->ncols; j++) {
        varpanel_toggle_set_active (VARSEL_X, j, false, d);
        varpanel_toggle_set_active (VARSEL_Y, j, false, d);
        varpanel_widget_set_visible (VARSEL_Y, j, false, d);
        varpanel_toggle_set_active (VARSEL_Z, j, false, d);
        varpanel_widget_set_visible (VARSEL_Z, j, false, d);
      }
      for (j=0; j<display->t1d.nsubset; j++) {
        varpanel_toggle_set_active (VARSEL_X,
          display->t1d.subset_vars.els[j], true, d);
      }
    break;
#ifdef ROTATION_IMPLEMENTED
    case TOUR2D3:
      for (j=0; j<d->ncols; j++) {
        varpanel_toggle_set_active (VARSEL_X, j, false, d);
        varpanel_toggle_set_active (VARSEL_Y, j, false, d);
        varpanel_widget_set_visible (VARSEL_Y, j, true, d);
        varpanel_toggle_set_active (VARSEL_Z, j, false, d);
        varpanel_widget_set_visible (VARSEL_Z, j, true, d);
      }

      varpanel_toggle_set_active (VARSEL_X,
          display->t2d3.subset_vars.els[0], true, d);
      varpanel_toggle_set_active (VARSEL_Y,
          display->t2d3.subset_vars.els[1], true, d);
      varpanel_toggle_set_active (VARSEL_Z,
          display->t2d3.subset_vars.els[2], true, d);
    break;
    break;
#endif
    case TOUR2D:
      for (j=0; j<d->ncols; j++) {
        varpanel_toggle_set_active (VARSEL_X, j, false, d);
        varpanel_toggle_set_active (VARSEL_Y, j, false, d);
        varpanel_widget_set_visible (VARSEL_Y, j, false, d);
        varpanel_toggle_set_active (VARSEL_Z, j, false, d);
        varpanel_widget_set_visible (VARSEL_Z, j, false, d);
      }
      for (j=0; j<display->t2d.nsubset; j++) {
        varpanel_toggle_set_active (VARSEL_X,
          display->t2d.subset_vars.els[j], true, d);
      }
    break;
    case COTOUR:
      for (j=0; j<d->ncols; j++) {
        varpanel_toggle_set_active (VARSEL_X, j, false, d);
        varpanel_toggle_set_active (VARSEL_Y, j, false, d);
        varpanel_widget_set_visible (VARSEL_Y, j, true, d);
        varpanel_toggle_set_active (VARSEL_Z, j, false, d);
        varpanel_widget_set_visible (VARSEL_Z, j, false, d);
      }
      for (j=0; j<display->tcorr1.nsubset; j++) {
        varpanel_toggle_set_active (VARSEL_X,
          display->tcorr1.subset_vars.els[j], true, d);
      }
      for (j=0; j<display->tcorr2.nsubset; j++) {
        varpanel_toggle_set_active (VARSEL_Y,
          display->tcorr2.subset_vars.els[j], true, d);
      }
    break;
    /*-- to pacify compiler --*/
    default:
    break;
  }
}

static gboolean
variableSelect(GtkWidget *w, displayd *display, splotd *sp, gint jvar, gint toggle, gint mouse, cpaneld *cpanel, ggobid *gg)
{
  gboolean redraw = false;
  gint jvar_prev = -1;

  switch (cpanel->projection) {
    case P1PLOT:
      redraw = p1d_varsel (sp, jvar, &jvar_prev, toggle, mouse);
      if (viewmode_get (gg) == BRUSH && cpanel->br_mode == BR_TRANSIENT)
        reinit_transient_brushing (display, gg);
    break;
    case XYPLOT:
      redraw = xyplot_varsel (sp, jvar, &jvar_prev, toggle, mouse);
      if (redraw)
        if (viewmode_get (gg) == BRUSH && cpanel->br_mode == BR_TRANSIENT)
          reinit_transient_brushing (display, gg);
    break;
    case TOUR1D:
      redraw = tour1d_varsel (w, jvar, toggle, mouse, display->d, gg);
    break;
#ifdef ROTATION_IMPLEMENTED
    case TOUR2D3:
      redraw = tour2d3_varsel (w, jvar, toggle, mouse, display->d, gg);
    break;
#endif
    case TOUR2D:
      redraw = tour2d_varsel (w, jvar, toggle, mouse, display->d, gg);
    break;
    case COTOUR:
      redraw = tourcorr_varsel (w, jvar, toggle, mouse, display->d, gg);
    break;
    /*-- to pacify compiler if we change these to an enum --*/
    default:
    break;
  }
  return(redraw);
}

static gboolean
varcircleDraw(displayd *display, gint jvar, GdkPixmap *da_pix, ggobid *gg)
{
  gint r = VAR_CIRCLE_DIAM/2;
  gint x,y, k;
  cpaneld *cpanel = &display->cpanel;
  gboolean chosen = false;

  switch (cpanel->projection) {
    case TOUR1D:
      x = (gint) (display->t1d.F.vals[0][jvar]*(gfloat)r);
      y = 0;
      gdk_draw_line (da_pix,
        gg->selvarfg_GC, r, r, r+x, r-y);

      if (jvar == display->t1d_manip_var) {
        gdk_draw_arc (da_pix, gg->manipvarfg_GC, false,
          5, 5, VAR_CIRCLE_DIAM-10, VAR_CIRCLE_DIAM-10, 150*64, 60*64);
        gdk_draw_arc (da_pix, gg->manipvarfg_GC, false,
          5, 5, VAR_CIRCLE_DIAM-10, VAR_CIRCLE_DIAM-10, 330*64, 60*64);
      }

      for (k=0; k < display->t1d.nactive; k++) {
        if (display->t1d.active_vars.els[k] == jvar) {
          chosen = true;
          break;
        }
      }
    break;
#ifdef ROTATION_IMPLEMENTED
    case TOUR2D3:
      x = (gint) (display->t2d3.F.vals[0][jvar]*(gfloat)r);
      y = (gint) (display->t2d3.F.vals[1][jvar]*(gfloat)r);
      gdk_draw_line (da_pix,
        gg->selvarfg_GC, r, r, r+x, r-y);

      if (jvar == display->t2d3_manip_var) {
        gdk_draw_arc (da_pix, gg->manipvarfg_GC, false,
          5, 5, VAR_CIRCLE_DIAM-10, VAR_CIRCLE_DIAM-10, 0*64, 360*64);
      }

      for (k=0; k<display->t2d3.nactive; k++) {
        if (display->t2d3.active_vars.els[k] == jvar) {
          chosen = true;
          break;
        }
      }
    break;
#endif
    case TOUR2D:
      x = (gint) (display->t2d.F.vals[0][jvar]*(gfloat)r);
      y = (gint) (display->t2d.F.vals[1][jvar]*(gfloat)r);
      gdk_draw_line (da_pix,
        gg->selvarfg_GC, r, r, r+x, r-y);

      if (jvar == display->t2d_manip_var) {
        gdk_draw_arc (da_pix, gg->manipvarfg_GC, false,
          5, 5, VAR_CIRCLE_DIAM-10, VAR_CIRCLE_DIAM-10, 0*64, 360*64);
      }

      for (k=0; k<display->t2d.nactive; k++) {
        if (display->t2d.active_vars.els[k] == jvar) {
          chosen = true;
          break;
        }
      }
    break;
    case COTOUR:
      /*          for (i=0; i<display->tcorr1.nactive; i++)
        if (jvar == display->tcorr1.active_vars.els[i]) {
          xvar = true;
          break;
        }*/
      /*          if (xvar) {*/
      x = (gint) (display->tcorr1.F.vals[0][jvar]*(gfloat)r);
      y = (gint) (display->tcorr2.F.vals[0][jvar]*(gfloat)r);
      gdk_draw_line (da_pix, gg->selvarfg_GC, r, r, r+x, r-y);

      if (jvar == display->tc1_manip_var) {
        gdk_draw_arc (da_pix, gg->manipvarfg_GC, false,
          5, 5, VAR_CIRCLE_DIAM-10, VAR_CIRCLE_DIAM-10, 150*64, 60*64);
        gdk_draw_arc (da_pix, gg->manipvarfg_GC, false,
          5, 5, VAR_CIRCLE_DIAM-10, VAR_CIRCLE_DIAM-10, 330*64, 60*64);
      }
      if (jvar == display->tc2_manip_var) {
        gdk_draw_arc (da_pix, gg->manipvarfg_GC, false,
          5, 5, VAR_CIRCLE_DIAM-10, VAR_CIRCLE_DIAM-10, 60*64, 60*64);
        gdk_draw_arc (da_pix, gg->manipvarfg_GC, false,
          5, 5, VAR_CIRCLE_DIAM-10, VAR_CIRCLE_DIAM-10, 240*64, 60*64);
      }

      for (k=0; k<display->tcorr1.nactive; k++) {
        if (display->tcorr1.active_vars.els[k] == jvar) {
          chosen = true;
          break;
        }
      }
      for (k=0; k<display->tcorr2.nactive; k++) {
        if (display->tcorr2.active_vars.els[k] == jvar) {
          chosen = true;
          break;
        }
      }
    break;

    /*      } 
      else {

        x = 0;
        y = (gint) (display->tcorr2.F.vals[0][jvar]*(gfloat)r);
        gdk_draw_line (da_pix,
          gg->selvarfg_GC, r, r, r+x, r-y);

      }*/

    default:
    break;
  }

  return(chosen);
}

static void
tourCorrRealloc(displayd *dsp, gint nc, datad *d)
{
    /*
     * because display_tourcorr_init_null has been performed even if
     * alloc_tourcorr has not, Fa.ncols has been initialized, and
     * dsp->tcorr1.Fa.ncols = 0.
    */
    gint old_ncols, i;
    old_ncols = dsp->tcorr1.Fa.ncols;

    if (nc >= MIN_NVARS_FOR_COTOUR) {
      if (old_ncols < MIN_NVARS_FOR_COTOUR)
        display_tourcorr_init(dsp, d->gg);

      if (dsp->d == d) {
        arrayd_add_cols (&dsp->tcorr1.Fa, nc);
        arrayd_add_cols (&dsp->tcorr1.Fz, nc);
        arrayd_add_cols (&dsp->tcorr1.F, nc);
        arrayd_add_cols (&dsp->tcorr1.Ga, nc);
        arrayd_add_cols (&dsp->tcorr1.Gz, nc);
        arrayd_add_cols (&dsp->tcorr1.G, nc);
        arrayd_add_cols (&dsp->tcorr1.Va, nc);
        arrayd_add_cols (&dsp->tcorr1.Vz, nc);
        arrayd_add_cols (&dsp->tcorr1.tv, nc);

        vectori_realloc (&dsp->tcorr1.subset_vars, nc);
        vectorb_realloc (&dsp->tcorr1.subset_vars_p, nc);
        vectori_realloc (&dsp->tcorr1.active_vars, nc);
        vectorb_realloc (&dsp->tcorr1.active_vars_p, nc);

        vectorf_realloc (&dsp->tcorr1.lambda, nc);
        vectorf_realloc (&dsp->tcorr1.tau, nc);
        vectorf_realloc (&dsp->tcorr1.tinc, nc);

        arrayd_add_cols (&dsp->tc1_manbasis, (gint) nc);
        arrayd_add_cols (&dsp->tc2_manbasis, (gint) nc);

        arrayd_add_cols (&dsp->tcorr2.Fa, nc);
        arrayd_add_cols (&dsp->tcorr2.Fz, nc);
        arrayd_add_cols (&dsp->tcorr2.F, nc);
        arrayd_add_cols (&dsp->tcorr2.Ga, nc);
        arrayd_add_cols (&dsp->tcorr2.Gz, nc);
        arrayd_add_cols (&dsp->tcorr2.G, nc);
        arrayd_add_cols (&dsp->tcorr2.Va, nc);
        arrayd_add_cols (&dsp->tcorr2.Vz, nc);
        arrayd_add_cols (&dsp->tcorr2.tv, nc);

        vectori_realloc (&dsp->tcorr2.subset_vars, nc);
        vectorb_realloc (&dsp->tcorr2.subset_vars_p, nc);
        vectori_realloc (&dsp->tcorr2.active_vars, nc);
        vectorb_realloc (&dsp->tcorr2.active_vars_p, nc);

        vectorf_realloc (&dsp->tcorr2.lambda, nc);
        vectorf_realloc (&dsp->tcorr2.tau, nc);
        vectorf_realloc (&dsp->tcorr2.tinc, nc);

        /* need to zero extra cols */
        for (i=old_ncols; i<nc; i++) {
          dsp->tcorr1.Fa.vals[0][i] = 0.0;
          dsp->tcorr1.Fz.vals[0][i] = 0.0;
          dsp->tcorr1.F.vals[0][i] = 0.0;
          dsp->tcorr1.Ga.vals[0][i] = 0.0;
          dsp->tcorr1.Gz.vals[0][i] = 0.0;
          dsp->tcorr1.G.vals[0][i] = 0.0;
          dsp->tcorr1.Va.vals[0][i] = 0.0;
          dsp->tcorr1.Vz.vals[0][i] = 0.0;
          dsp->tcorr1.tv.vals[0][i] = 0.0;

          dsp->tcorr1.subset_vars.els[i] = 0;
          dsp->tcorr1.subset_vars_p.els[i] = false;
          dsp->tcorr1.active_vars.els[i] = 0;
          dsp->tcorr1.active_vars_p.els[i] = false;

          dsp->tcorr1.lambda.els[i] = 0.0;
          dsp->tcorr1.tau.els[i] = 0.0;
          dsp->tcorr1.tinc.els[i] = 0.0;

          dsp->tcorr2.Fa.vals[0][i] = 0.0;
          dsp->tcorr2.Fz.vals[0][i] = 0.0;
          dsp->tcorr2.F.vals[0][i] = 0.0;
          dsp->tcorr2.Ga.vals[0][i] = 0.0;
          dsp->tcorr2.Gz.vals[0][i] = 0.0;
          dsp->tcorr2.G.vals[0][i] = 0.0;
          dsp->tcorr2.Va.vals[0][i] = 0.0;
          dsp->tcorr2.Vz.vals[0][i] = 0.0;
          dsp->tcorr2.tv.vals[0][i] = 0.0;

          dsp->tcorr2.subset_vars.els[i] = 0;
          dsp->tcorr2.subset_vars_p.els[i] = false;
          dsp->tcorr2.active_vars.els[i] = 0;
          dsp->tcorr2.active_vars_p.els[i] = false;

          dsp->tcorr2.lambda.els[i] = 0.0;
          dsp->tcorr2.tau.els[i] = 0.0;
          dsp->tcorr2.tinc.els[i] = 0.0;
        }
      }
    }
}

#ifdef ROTATION_IMPLEMENTED
static void
tour2d3Realloc(displayd *dsp, gint nc, datad *d)
{
  gint old_ncols, i;
  /*
   * because display_tour2d_init_null has been performed even if
   * alloc_tour2d has not, Fa.ncols has been initialized.
  */
  old_ncols = dsp->t2d3.Fa.ncols;

  if (nc >= MIN_NVARS_FOR_TOUR2D3) {
    if (old_ncols < MIN_NVARS_FOR_TOUR2D3)       
      display_tour2d3_init(dsp, d->gg);

    if (dsp->d == d) {
      arrayd_add_cols (&dsp->t2d3.Fa, nc);
      arrayd_add_cols (&dsp->t2d3.Fz, nc);
      arrayd_add_cols (&dsp->t2d3.F, nc);
      arrayd_add_cols (&dsp->t2d3.Ga, nc);
      arrayd_add_cols (&dsp->t2d3.Gz, nc);
      arrayd_add_cols (&dsp->t2d3.G, nc);
      arrayd_add_cols (&dsp->t2d3.Va, nc);
      arrayd_add_cols (&dsp->t2d3.Vz, nc);
      arrayd_add_cols (&dsp->t2d3.tv, nc);

      vectori_realloc (&dsp->t2d3.subset_vars, nc);
      vectorb_realloc (&dsp->t2d3.subset_vars_p, nc);
      vectori_realloc (&dsp->t2d3.active_vars, nc);
      vectorb_realloc (&dsp->t2d3.active_vars_p, nc);

      vectorf_realloc (&dsp->t2d3.lambda, nc);
      vectorf_realloc (&dsp->t2d3.tau, nc);
      vectorf_realloc (&dsp->t2d3.tinc, nc);

      arrayd_add_cols (&dsp->t2d3_manbasis, (gint) nc);

      /* need to zero extra cols */
      for (i=old_ncols; i<nc; i++) {
        dsp->t2d3.Fa.vals[0][i] = dsp->t2d3.Fa.vals[1][i] = 0.0;
        dsp->t2d3.Fz.vals[0][i] = dsp->t2d3.Fz.vals[1][i] = 0.0;
        dsp->t2d3.F.vals[0][i] = dsp->t2d3.F.vals[1][i] = 0.0;
        dsp->t2d3.Ga.vals[0][i] = dsp->t2d3.Ga.vals[1][i] = 0.0;
        dsp->t2d3.Gz.vals[0][i] = dsp->t2d3.Gz.vals[1][i] = 0.0;
        dsp->t2d3.G.vals[0][i] = dsp->t2d3.G.vals[1][i] = 0.0;
        dsp->t2d3.Va.vals[0][i] = dsp->t2d3.Va.vals[1][i] = 0.0;
        dsp->t2d3.Vz.vals[0][i] = dsp->t2d3.Vz.vals[1][i] = 0.0;
        dsp->t2d3.tv.vals[0][i] = dsp->t2d3.tv.vals[1][i] = 0.0;
        dsp->t2d3.subset_vars.els[i] = 0;
        dsp->t2d3.subset_vars_p.els[i] = false;
        dsp->t2d3.active_vars.els[i] = 0;
        dsp->t2d3.active_vars_p.els[i] = false;
        dsp->t2d3.lambda.els[i] = 0.0;
        dsp->t2d3.tau.els[i] = 0.0;
        dsp->t2d3.tinc.els[i] = 0.0;
      }
    }
  }
}
#endif

static void
tour2dRealloc(displayd *dsp, gint nc, datad *d)
{
  gint old_ncols, i;
  /*
   * because display_tour2d_init_null has been performed even if
   * alloc_tour2d has not, Fa.ncols has been initialized.
  */
  old_ncols = dsp->t2d.Fa.ncols;

  if (nc >= MIN_NVARS_FOR_TOUR2D) {
    if (old_ncols < MIN_NVARS_FOR_TOUR2D)       
      display_tour2d_init(dsp, d->gg);

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

      vectori_realloc (&dsp->t2d.subset_vars, nc);
      vectorb_realloc (&dsp->t2d.subset_vars_p, nc);
      vectori_realloc (&dsp->t2d.active_vars, nc);
      vectorb_realloc (&dsp->t2d.active_vars_p, nc);

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
        dsp->t2d.subset_vars.els[i] = 0;
        dsp->t2d.subset_vars_p.els[i] = false;
        dsp->t2d.active_vars.els[i] = 0;
        dsp->t2d.active_vars_p.els[i] = false;
        dsp->t2d.lambda.els[i] = 0.0;
        dsp->t2d.tau.els[i] = 0.0;
        dsp->t2d.tinc.els[i] = 0.0;
      }
    }
  }
}

static void
tour1dRealloc(displayd *dsp, gint nc, datad *d)
{
  gint old_ncols, i;
    /*
     * because display_tour1d_init_null has been performed even if
     * alloc_tour1d has not, Fa.ncols has been initialized.
    */
    old_ncols = dsp->t1d.Fa.ncols;

    if (old_ncols < MIN_NVARS_FOR_TOUR1D && nc >= MIN_NVARS_FOR_TOUR1D) {
      display_tour1d_init(dsp, d->gg);
    }

    if (dsp->d == d) {
      arrayd_add_cols (&dsp->t1d.Fa, nc);
      arrayd_add_cols (&dsp->t1d.Fz, nc);
      arrayd_add_cols (&dsp->t1d.F, nc);
      arrayd_add_cols (&dsp->t1d.Ga, nc);
      arrayd_add_cols (&dsp->t1d.Gz, nc);
      arrayd_add_cols (&dsp->t1d.G, nc);
      arrayd_add_cols (&dsp->t1d.Va, nc);
      arrayd_add_cols (&dsp->t1d.Vz, nc);
      arrayd_add_cols (&dsp->t1d.tv, nc);

      vectori_realloc (&dsp->t1d.subset_vars, nc);
      vectorb_realloc (&dsp->t1d.subset_vars_p, nc);
      vectori_realloc (&dsp->t1d.active_vars, nc);
      vectorb_realloc (&dsp->t1d.active_vars_p, nc);

      vectorf_realloc (&dsp->t1d.lambda, nc);
      vectorf_realloc (&dsp->t1d.tau, nc);
      vectorf_realloc (&dsp->t1d.tinc, nc);

      arrayd_add_cols (&dsp->t1d_manbasis, (gint) nc);

      /* need to zero extra cols */
      for (i=old_ncols; i<nc; i++) {
        dsp->t1d.Fa.vals[0][i] = 0.0;
        dsp->t1d.Fz.vals[0][i] = 0.0;
        dsp->t1d.F.vals[0][i]  = 0.0;
        dsp->t1d.Ga.vals[0][i] = 0.0;
        dsp->t1d.Gz.vals[0][i] = 0.0;
        dsp->t1d.G.vals[0][i] = 0.0;
        dsp->t1d.Va.vals[0][i] = 0.0;
        dsp->t1d.Vz.vals[0][i] = 0.0;
        dsp->t1d.tv.vals[0][i] = 0.0;
        dsp->t1d.subset_vars.els[i] = 0;
        dsp->t1d.subset_vars_p.els[i] = false;
        dsp->t1d.active_vars.els[i] = 0;
        dsp->t1d.active_vars_p.els[i] = false;
        dsp->t1d.lambda.els[i] = 0.0;
        dsp->t1d.tau.els[i] = 0.0;
        dsp->t1d.tinc.els[i] = 0.0;
      }
    }
}

void
worldToRaw(displayd *display, splotd *sp, gint pt, datad *d, ggobid *gg)
{
  cpaneld *cpanel = &display->cpanel;
  PipelineMode proj = cpanel->projection;
  gint j;

  switch (proj) {
    case P1PLOT:
      if (display->p1d_orientation == VERTICAL)
        world_to_raw_by_var (pt, sp->p1dvar, display, d, gg);
      else
        world_to_raw_by_var (pt, sp->p1dvar, display, d, gg);
    break;
    case XYPLOT:
      world_to_raw_by_var (pt, sp->xyvars.x, display, d, gg);
      world_to_raw_by_var (pt, sp->xyvars.y, display, d, gg);
    break;
    case TOUR1D:
      for (j=0; j<display->t1d.nactive; j++)
        world_to_raw_by_var (pt, display->t1d.active_vars.els[j],
          display, d, gg);
    break;
#ifdef ROTATION_IMPLEMENTED
    case TOUR2D3:
      for (j=0; j<display->t2d3.nactive; j++)
        world_to_raw_by_var (pt, display->t2d3.active_vars.els[j],
          display, d, gg);
    break;
#endif
    case TOUR2D:
      for (j=0; j<display->t2d.nactive; j++)
        world_to_raw_by_var (pt, display->t2d.active_vars.els[j],
          display, d, gg);
    break;
    case COTOUR:
      for (j=0; j<display->tcorr1.nactive; j++)
        world_to_raw_by_var (pt, display->tcorr1.active_vars.els[j],
          display, d, gg);
      for (j=0; j<display->tcorr2.nactive; j++)
        world_to_raw_by_var (pt, display->tcorr2.active_vars.els[j],
          display, d, gg);
    break;
    default:
    break;
  }
}


void
scatterplotMovePointsButtonCb(displayd *display, splotd *sp, GtkWidget *w, GdkEventButton *event, ggobid *gg)
{
  datad *d = gg->current_display->d;
    if (d->nearest_point != -1) {
      movepts_history_add (d->nearest_point, sp, d, gg);

      /*-- add the history information for the cluster here --*/
      if (gg->movepts.cluster_p) {
        clusters_set (d, gg);
        if (d->nclusters > 1) {
          gint i, k, id = d->nearest_point;
          gfloat cur_clust = d->clusterid.els[id];
          for (i=0; i<d->nrows_in_plot; i++) {
            k = d->rows_in_plot[i];
            if (k == id)
              ;
            else
              if (d->clusterid.els[k] == cur_clust)
                if (!d->hidden_now.els[k])
                  movepts_history_add (k, sp, d, gg);
          }
        }
      }

      splot_redraw (sp, QUICK, gg);  
    }
}

void
scatterplotMovePointsMotionCb(displayd *display, splotd *sp, GtkWidget *w, GdkEventMotion *event, ggobid *gg)
{
  datad *d = display->d;
  gboolean button1_p, button2_p;
  gboolean inwindow, wasinwindow;

    /*-- define wasinwindow before the new mousepos is calculated --*/
    wasinwindow = mouseinwindow (sp);
    /*-- get the mouse position and find out which buttons are pressed --*/
    mousepos_get_motion (w, event, &button1_p, &button2_p, sp);
    inwindow = mouseinwindow (sp);

    if (gg->buttondown == 0) {

      gint k = find_nearest_point (&sp->mousepos, sp, d, gg);
      d->nearest_point = k;
      if (k != d->nearest_point_prev) {
        displays_plot (NULL, QUICK, gg);
        d->nearest_point_prev = k;
      }

    } else {


      /*-- If the pointer is inside the plotting region ... --*/
      if (inwindow) {
        /*-- ... and if the pointer has moved ...--*/
        if ((sp->mousepos.x != sp->mousepos_o.x) ||
            (sp->mousepos.y != sp->mousepos_o.y))
        {
          /*
           * move the point: compute the data pipeline in reverse,
           * (then run it forward again?) and draw the plot.
          */
          if (d->nearest_point != -1) {
            move_pt (d->nearest_point, sp->mousepos.x, sp->mousepos.y,
                     sp, d, gg);
          }
          sp->mousepos_o.x = sp->mousepos.x;
          sp->mousepos_o.y = sp->mousepos.y;
        }
      } else {  /*-- if !inwindow --*/
        if (wasinwindow) {
          d->nearest_point = -1;
          splot_redraw (sp, QUICK, gg);  
        }
      }
    }
}

static void
viewmodeSet(displayd *display, ggobid *gg)
{
  if (gg->viewmode <= COTOUR)
    display->cpanel.projection = gg->viewmode;
  gg->projection = display->cpanel.projection;

  if (gg->projection != gg->prev_projection) {
    scatterplot_show_rulers (display, gg->projection);
    gg->prev_projection = gg->projection;
  }
}

static gboolean
varpanelHighd(displayd *display)
{
  gint proj = display->cpanel.projection;
#ifdef ROTATION_IMPLEMENTED
  return(proj == TOUR1D || proj == TOUR2D3 || proj == TOUR2D || proj == COTOUR);
#else
  return(proj == TOUR1D || proj == TOUR2D || proj == COTOUR);
#endif
}

displayd *
gtk_scatterplot_new(datad *d, ggobid *gg)
{
  displayd *display;
  display = scatterplot_new(false, NULL, d, gg);

  return(display);
}

void
scatterplotDisplayInit(scatterplotDisplayd *display)
{
 GTK_GGOBI_DISPLAY(display)->p1d_orientation = HORIZONTAL;
}



gboolean
binningPermitted(displayd* dpy)
{
  cpaneld *cpanel = &dpy->cpanel;
  ggobid *gg = dpy->ggobi;
  datad *e = dpy->e;

  if (projection_get(gg) == P1PLOT &&
       cpanel->p1d.type == ASH &&
       cpanel->p1d.ASH_add_lines_p)
     return(false);

  /*-- if we're drawing edges --*/
  if (e != NULL && e->edge.n > 0) {
    if (dpy->options.edges_undirected_show_p ||
        dpy->options.edges_directed_show_p ||
        dpy->options.whiskers_show_p)
    {
      return (false);
    }
  }

  return(true);
}

gboolean
cpanelSet(displayd *dpy, cpaneld *cpanel, ggobid *gg)
{
#if 0
/*XX Add the creation of the widget here! */
      GtkWidget *w;
      w = GTK_GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget;
      if(!w) {
        GTK_GGOBI_EXTENDED_DISPLAY(dpy)->cpanelWidget = w =  cpanel_scatterplot_make(gg);
      }
#endif

      cpanel_p1d_set (cpanel, gg);
      cpanel_xyplot_set (cpanel, gg);
      cpanel_tour1d_set (cpanel, gg);
#ifdef ROTATION_IMPLEMENTED
      cpanel_tour2d3_set (cpanel, gg);
#endif
      if (dpy->d->ncols >= MIN_NVARS_FOR_TOUR2D)
        cpanel_tour2d_set (cpanel, gg);
      if (dpy->d->ncols >= MIN_NVARS_FOR_COTOUR)
        cpanel_tourcorr_set (cpanel, gg);

      cpanel_brush_set (cpanel, gg);
      cpanel_scale_set (cpanel, gg);
#ifdef EDIT_EDGES_IMPLEMENTED
      cpanel_edgeedit_set (cpanel, gg);
#endif
      cpanel_identify_set (cpanel, gg);

      return(true);
}

void
displaySet(displayd *dpy, ggobid *gg)
{
  scatterplot_mode_menu_make (gg->main_accel_group,
			      (GtkSignalFunc) viewmode_set_cb, gg, true);
  gg->viewmode_item = submenu_make ("_ViewMode", 'V',
				    gg->main_accel_group);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (gg->viewmode_item),
			     gg->app.scatterplot_mode_menu); 
  submenu_insert (gg->viewmode_item, gg->main_menubar, 2);
}

static gboolean
handlesAction(displayd *dpy,  PipelineMode v)
{
    if (v != SCATMAT && v != PCPLOT)
      return(true);

    return(false);
}


static gint 
plotted(displayd *display, gint *cols, gint ncols, datad *d)
{
  gint j, k;
  splotd *sp = (splotd *) display->splots->data;  /*-- only one splot --*/
  gint projection = projection_get (display->ggobi);

  switch (projection) {
    case P1PLOT:
      for (j=0; j<ncols; j++) {
        if (sp->p1dvar == cols[j]) {
          return(sp->p1dvar);
        }
      }
    break;
    case XYPLOT:
      for (j=0; j<ncols; j++) {
        if (sp->xyvars.x == cols[j]) {
          return(sp->xyvars.x);
        }
        if (sp->xyvars.y == cols[j]) {
          return(sp->xyvars.y);
        }
      }
    break;
    case TOUR1D:
      for (j=0; j<ncols; j++) {
        for (k=0; k<display->t1d.nactive; k++) {
          if (display->t1d.active_vars.els[k] == cols[j]) {
            return(display->t1d.active_vars.els[k]);
          }
        }
      }
    break;
#ifdef ROTATION_IMPLEMENTED
    case TOUR2D3:
      for (j=0; j<ncols; j++) {
        for (k=0; k<display->t2d3.nactive; k++) {
          if (display->t2d3.active_vars.els[k] == cols[j]) {
            return(display->t2d3.active_vars.els[k]);
          }
        }
      }
    break;
#endif
    case TOUR2D:
      for (j=0; j<ncols; j++) {
        for (k=0; k<display->t2d.nactive; k++) {
          if (display->t2d.active_vars.els[k] == cols[j]) {
            return(display->t2d.active_vars.els[k]);
          }
        }
      }
    break;
    case COTOUR:
      for (j=0; j<ncols; j++) {
        for (k=0; k<display->tcorr1.nactive; k++) {
          if (display->tcorr1.active_vars.els[k] == cols[j]) {
            return(display->tcorr1.active_vars.els[k]);
          }
        }
        for (k=0; k<display->tcorr2.nactive; k++) {
          if (display->tcorr2.active_vars.els[k] == cols[j]) {
            return(display->tcorr2.active_vars.els[k]);
          }
        }
      }
    break;
  }

  return(-1);
}


static void
varpanelTooltipsReset(displayd *display, ggobid *gg, GtkWidget *wx, GtkWidget *wy, GtkWidget *wz, GtkWidget *label)
{
  gint projection = projection_get (gg);

  switch (projection) {
    case P1PLOT:
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wx,
        "Select to plot",
        NULL);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), label,
        "Click left to plot horizontally, right or middle to plot vertically",
        NULL);
    break;
    case XYPLOT:
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wx,
        "Press to select the horizontally plotted variable",
        NULL);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wy,
        "Press to select the vertically plotted variable",
        NULL);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), label,
        "Click left to select the horizontal variable, middle for vertical",
        NULL);

    break;
    case TOUR1D:
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wx,
        "Click to select a variable to be available for touring",
        NULL);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), label,
        "Click to select a variable to be available for touring",
        NULL);
    break;
#ifdef ROTATION_IMPLEMENTED
    case TOUR2D3:
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wx,
        "Click to select a variable to be available for rotation",
        NULL);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wy,
        "Click to select a variable to be available for rotation",
        NULL);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wz,
        "Click to select a variable to be available for rotation",
        NULL);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), label,
        "Click to select a variable to be available for rotation",
        NULL);
    break;
#endif
    case TOUR2D:
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wx,
        "Click to select a variable to be available for touring",
        NULL);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), label,
        "Click to select a variable to be available for touring",
        NULL);
    break;
    case COTOUR:
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wx,
        "Click to select a variable to be toured horizontally",
        NULL);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), wy,
        "Click to select a variable to be toured vertically",
        NULL);
      gtk_tooltips_set_tip (GTK_TOOLTIPS (gg->tips), label,
        "Click to select a variable to be available for touring",
        NULL);
    break;
    /*-- to pacify compiler if we change these to an enum --*/
    default:
    break;
  }
}

static gint 
plottedVarsGet(displayd *display, gint *cols, datad *d, ggobid *gg)
{
  PipelineMode mode = viewmode_get (gg);
  gint ncols = 0, k;
  splotd *sp = gg->current_splot;

  switch (mode) {
    case P1PLOT:
      cols[ncols++] = sp->p1dvar;
    break;
    case XYPLOT:
      cols[ncols++] = sp->xyvars.x;
      cols[ncols++] = sp->xyvars.y;
    break;
    case TOUR1D:
      for (k=0; k<display->t1d.nactive; k++)
        cols[ncols++] = display->t1d.active_vars.els[k];
    break;
#ifdef ROTATION_IMPLEMENTED
    case TOUR2D3:
      for (k=0; k<display->t2d3.nactive; k++)
        cols[ncols++] = display->t2d3.active_vars.els[k];
    break;
#endif
    case TOUR2D:
      for (k=0; k<display->t2d.nactive; k++)
        cols[ncols++] = display->t2d.active_vars.els[k];
    break;
    case COTOUR:
      for (k=0; k<display->tcorr1.nactive; k++)
        cols[ncols++] = display->tcorr1.active_vars.els[k];
      for (k=0; k<display->tcorr2.nactive; k++)
        cols[ncols++] = display->tcorr2.active_vars.els[k];
    break;
    default:
    break;
  }
  
  return(ncols);
}

/*
  Write out the variables in a scatterplot
  to the current node in the XML tree.
 */
static void
add_xml_scatterplot_variables(xmlNodePtr node, GList *plots, displayd *dpy)
{
  splotd *plot = (splotd *)plots->data;
  XML_addVariable(node, plot->xyvars.x, dpy->d);
  XML_addVariable(node, plot->xyvars.y, dpy->d);
}


/* Splot methods. */
static gchar *
treeLabel(splotd *splot, datad *d, ggobid *gg)
{
  gchar *buf = NULL;
  displayd *display = (displayd *) splot->displayptr; 
  cpaneld *cpanel = &display->cpanel;
  vartabled *vt, *vtx, *vty;
  gint n;

  switch (cpanel->projection) {
     case P1PLOT:
     case TOUR1D:
       vt = vartable_element_get (splot->p1dvar, d);
       n = strlen (vt->collab);
       buf = (gchar*) g_malloc(n* sizeof (gchar*));
       sprintf(buf, "%s", vt->collab);
     break;

     case XYPLOT:
       vtx = vartable_element_get (splot->xyvars.x, d);
       vty = vartable_element_get (splot->xyvars.y, d);

       n = strlen (vtx->collab) + strlen (vty->collab) + 5;
       buf = (gchar*) g_malloc (n * sizeof (gchar*));
       sprintf (buf, "%s v %s", vtx->collab, vty->collab);
     break;

     case TOUR2D:
       n = strlen ("in grand tour");
       buf = (gchar*) g_malloc (n * sizeof (gchar*));
       sprintf (buf, "%s", "in grand tour");
     break;

#ifdef ROTATION_IMPLEMENTED
     case TOUR2D3:
       n = strlen ("in rotation");
       buf = (gchar*) g_malloc (n * sizeof (gchar*));
       sprintf (buf, "%s", "in grand tour");
     break;
#endif

     case COTOUR:
       n = strlen ("in correlation tour");
       buf = (gchar*) g_malloc (n * sizeof (gchar*));
       sprintf (buf, "%s", "in correlation tour");
     break;
     default:
     break;
  }
  return(buf);
}


static void
subPlaneToScreen(splotd *sp, displayd *dpy, datad *d, ggobid *gg)
{
      ash_baseline_set (&sp->p1d.ash_baseline, sp);
      ash_baseline_set (&sp->tour1d.ash_baseline, sp);
}

static void
worldToPlane(splotd *sp, datad *d, ggobid *gg)
{
  cpaneld *cpanel = &(sp->displayptr->cpanel);

  switch (cpanel->projection) {
    case P1PLOT:
      p1d_reproject (sp, d->world.vals, d, gg);
    break;

    case XYPLOT:
      xy_reproject (sp, d->world.vals, d, gg);
    break;

    case TOUR1D:
      tour1d_projdata (sp, d->world.vals, d, gg);
    break;

#ifdef ROTATION_IMPLEMENTED
    case TOUR2D3:
      tour2d3_projdata(sp, d->world.vals, d, gg);
    break;
#endif
    case TOUR2D:
      tour2d_projdata(sp, d->world.vals, d, gg);
    break;

    case COTOUR:
      tourcorr_projdata(sp, d->world.vals, d, gg);
    break;

    default:
    break;
  }
}

static gboolean
drawCase(splotd *sp, gint m, datad *d, ggobid *gg)
{
  gboolean draw_case = true;
  gint proj = projection_get (gg);

  switch (proj) {
    case P1PLOT:
      if (d->missing.vals[m][sp->p1dvar])
        draw_case = false;
    break;
    case XYPLOT:
      if (d->missing.vals[m][sp->xyvars.x])
        draw_case = false;
      else if (d->missing.vals[m][sp->xyvars.y])
        draw_case = false;
    break;
    case TOUR1D:
      if (d->missing.vals[m][sp->displayptr->t1d.active_vars.els[m]])
        draw_case = false;
    break;
#ifdef ROTATION_IMPLEMENTED
    case TOUR2D3:
      if (d->missing.vals[m][sp->displayptr->t2d3.active_vars.els[m]])
        draw_case = false;
    break;
#endif
    case TOUR2D:
      if (d->missing.vals[m][sp->displayptr->t2d.active_vars.els[m]])
        draw_case = false;
    break;

    case COTOUR:
      if (d->missing.vals[m][sp->displayptr->tcorr1.active_vars.els[m]])
        draw_case = false;
      else if (d->missing.vals[m][sp->displayptr->tcorr2.active_vars.els[m]])
        draw_case = false;
    break;
  }

  return(draw_case);
}

static gboolean
drawEdge(splotd *sp, gint m, datad *d, datad *e, ggobid *gg) 
{
  gboolean draw_edge = true;
  gint proj = projection_get (gg);

  switch (proj) {
    case P1PLOT:
      if (e->missing.vals[m][sp->p1dvar])
        draw_edge = false;
    break;
    case XYPLOT:
      if (e->missing.vals[m][sp->xyvars.x])
        draw_edge = false;
      else if (e->missing.vals[m][sp->xyvars.y])
        draw_edge = false;
    break;
    case TOUR1D:
      if (e->missing.vals[m][sp->displayptr->t1d.active_vars.els[m]])
        draw_edge = false;
    break;

#ifdef ROTATION_IMPLEMENTED
    case TOUR2D3:
      if (e->missing.vals[m][sp->displayptr->t2d3.active_vars.els[m]])
        draw_edge = false;
    break;
#endif
    case TOUR2D:
      if (e->missing.vals[m][sp->displayptr->t2d.active_vars.els[m]])
        draw_edge = false;
    break;

    case COTOUR:
      if (e->missing.vals[m][sp->displayptr->tcorr1.active_vars.els[m]])
        draw_edge = false;
      else if (e->missing.vals[m][sp->displayptr->tcorr2.active_vars.els[m]])
        draw_edge = false;
    break;
  }
  return(draw_edge);
}

void
scatter1DAddPlotLabels(splotd *sp, GdkDrawable *drawable, GdkGC *gc)
{
  gint lbearing, rbearing, width, ascent, descent;
  GtkStyle *style = gtk_widget_get_style (sp->da);
  vartabled *vt;
  datad *d = sp->displayptr->d;
      vt = vartable_element_get (sp->p1dvar, d);
      gdk_text_extents (
#if GTK_MAJOR_VERSION == 2
        gtk_style_get_font (style),
#else
        style->font,
#endif
        vt->collab_tform, strlen (vt->collab_tform),
        &lbearing, &rbearing, &width, &ascent, &descent);
      gdk_draw_string (drawable,
#if GTK_MAJOR_VERSION == 2
        gtk_style_get_font (style),
#else
        style->font,
#endif
        gc,
        sp->max.x/2 - width/2,  /*-- center --*/
        sp->max.y - 5,
        vt->collab_tform);
}

void
scatterXYAddPlotLabels(splotd *sp, GdkDrawable *drawable, GdkGC *gc)
{
  gint lbearing, rbearing, width, ascent, descent;
  GtkStyle *style = gtk_widget_get_style (sp->da);

  vartabled *vtx, *vty;
  datad *d = sp->displayptr->d;

      /*-- xyplot: right justify the label --*/
      vtx = vartable_element_get (sp->xyvars.x, d);
      gdk_text_extents (
#if GTK_MAJOR_VERSION == 2
        gtk_style_get_font (style),
#else
        style->font,
#endif
        vtx->collab_tform, strlen (vtx->collab_tform),
        &lbearing, &rbearing, &width, &ascent, &descent);
      gdk_draw_string (drawable,
#if GTK_MAJOR_VERSION == 2
        gtk_style_get_font (style),
#else
        style->font,
#endif
        gc,
        sp->max.x - width - 5,  /*-- right justify --*/
        sp->max.y - 5,
        vtx->collab_tform);

      vty = vartable_element_get (sp->xyvars.y, d);
      gdk_text_extents (
#if GTK_MAJOR_VERSION == 2
        gtk_style_get_font (style),
#else
        style->font,
#endif
        vty->collab_tform, strlen (vty->collab_tform),
        &lbearing, &rbearing, &width, &ascent, &descent);
      gdk_draw_string (drawable,
#if GTK_MAJOR_VERSION == 2
        gtk_style_get_font (style),
#else
        style->font,
#endif
        gc,
        5, 5 + ascent + descent,
        vty->collab_tform);
}

static void
addPlotLabels(splotd *sp, GdkDrawable *drawable, ggobid *gg)
{
/* Same as scatmat... */
    cpaneld *cpanel = &(sp->displayptr->cpanel);
    if(cpanel->projection == XYPLOT)
      scatterXYAddPlotLabels(sp, drawable, gg->plot_GC);
    else if(cpanel->projection == P1PLOT)
    scatter1DAddPlotLabels(sp, drawable, gg->plot_GC);



}

static void
withinDrawToUnbinned(splotd *sp, gint m, GdkDrawable *drawable, GdkGC *gc)
{
  displayd *display = sp->displayptr;
  cpaneld *cpanel = &display->cpanel;
  gint proj = cpanel->projection;
  icoords *baseline;

  /*-- add ash baseline to p1d or tour1d --*/
  if ((proj == TOUR1D && cpanel->t1d.ASH_add_lines_p) ||
      (proj == P1PLOT &&
       cpanel->p1d.type == ASH &&
       cpanel->p1d.ASH_add_lines_p))
  {
    baseline = (proj == TOUR1D) ? &sp->tour1d.ash_baseline :
                                  &sp->p1d.ash_baseline;

    if (display->p1d_orientation == HORIZONTAL)
      gdk_draw_line (drawable, gc,
        sp->screen[m].x, sp->screen[m].y,
        sp->screen[m].x, baseline->y);
    else
      gdk_draw_line (drawable, gc,
        sp->screen[m].x, sp->screen[m].y,
        baseline->x, sp->screen[m].y);
  }
}

void
addMarkupCues(splotd *sp, GdkDrawable *drawable, ggobid *gg)
{
/* See splot_add_markup_to_pixmap */
  displayd *display = sp->displayptr;
  datad *e = display->e;
  if (display->options.edges_undirected_show_p ||
      display->options.edges_arrowheads_show_p ||
      display->options.edges_directed_show_p)
      if (e->nearest_point != -1)
        splot_nearest_edge_highlight (sp, e->nearest_point, true, gg);
}

void
scatterplotDisplayClassInit(GtkGGobiScatterplotDisplayClass *klass)
{
  klass->parent_class.createWithVars = scatterplot_new_with_vars;
  klass->parent_class.create = scatterplot_new;
  klass->parent_class.show_edges_p = true;
  klass->parent_class.binningPermitted = binningPermitted;

  klass->parent_class.cpanel_set = cpanelSet;
  klass->parent_class.display_set = displaySet;

  klass->parent_class.handles_action = handlesAction;

  klass->parent_class.variable_plotted_p = plotted;

  klass->parent_class.varpanel_tooltips_set = varpanelTooltipsReset;
  klass->parent_class.plotted_vars_get = plottedVarsGet;

  klass->parent_class.titleLabel = "scatterplot display";
  klass->parent_class.treeLabel = "Scatterplot";
  klass->parent_class.ruler_ranges_set = ruler_ranges_set;
  klass->parent_class.xml_describe = add_xml_scatterplot_variables;

  klass->parent_class.varpanel_highd = varpanelHighd;
  klass->parent_class.varpanel_refresh = varpanelRefresh;
  klass->parent_class.variable_select = variableSelect;
  klass->parent_class.viewmode_set = viewmodeSet;
  klass->parent_class.move_points_motion_cb = scatterplotMovePointsMotionCb;
  klass->parent_class.move_points_button_cb = scatterplotMovePointsButtonCb;

  klass->parent_class.tour1d_realloc = tour1dRealloc;
#ifdef ROTATION_IMPLEMENTED
  klass->parent_class.tour2d3_realloc = tour2d3Realloc;
#endif
  klass->parent_class.tour2d_realloc = tour2dRealloc;
  klass->parent_class.tourcorr_realloc = tourCorrRealloc;

  klass->parent_class.xml_describe = add_xml_scatterplot_variables;

  klass->parent_class.set_show_axes_option = setShowAxesOption;
  klass->parent_class.set_show_axes_label_option = setShowAxesLabelOption;
  klass->parent_class.set_show_axes_values_option = setShowAxesValuesOption;

  klass->parent_class.world_to_raw = worldToRaw;

  klass->parent_class.select_X = selectXVar;
  klass->parent_class.varcircle_draw = varcircleDraw;
}

static gint
splotVariablesGet(splotd *sp, gint *cols, datad *d)
{
  cols[0] = sp->xyvars.x;
  cols[1] = sp->xyvars.y;
  return(2);
}

void
scatterSPlotClassInit(GtkGGobiScatterSPlotClass *klass)
{
   klass->parent_class.within_draw_to_unbinned = withinDrawToUnbinned;
   klass->parent_class.tree_label = treeLabel;
   klass->parent_class.sub_plane_to_screen = subPlaneToScreen;
   klass->parent_class.world_to_plane = worldToPlane;
   klass->parent_class.draw_case_p = drawCase;
   klass->parent_class.draw_edge_p = drawEdge;
   klass->parent_class.add_plot_labels = addPlotLabels;
   klass->parent_class.within_draw_to_unbinned = withinDrawToUnbinned;
   klass->parent_class.add_markup_cues = addMarkupCues;

   klass->parent_class.plotted_vars_get = splotVariablesGet;
}
