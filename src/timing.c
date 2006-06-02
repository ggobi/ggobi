#include <gtk/gtk.h>
#include "vars.h"
#include "externs.h"
#include "ggobi-data.h"

/*
 * t = current_time
 * open a dataset and default scatterplot
 * print (current_time - t); reset t
 * move brush from top left to bottom right (in 20 steps maybe?)
 * print (current_time - t); reset t
 * open a tour with all variables and run 10 tour steps
 * print (current_time - t); reset t
*/

void
set_time (ggobid *gg) {
  gdk_flush();
  g_get_current_time(&gg->time);
  //g_printerr ("(set time) %d %d\n", gg->time.tv_sec, gg->time.tv_usec);
  gdk_flush();
}

void
print_time_interval(gchar *where, ggobid *gg) {
  GTimeVal now;
  glong tdiff_usec;

  gdk_flush();
  g_get_current_time(&now);

  tdiff_usec = (G_USEC_PER_SEC*now.tv_sec + now.tv_usec) -
    (G_USEC_PER_SEC*gg->time.tv_sec + gg->time.tv_usec);

  g_printerr ("TIME(%s) %.2f msec\n", 
              where, (gfloat)tdiff_usec/1000);
  gdk_flush();
}


void
time_brushing (ggobid *gg) {
  displayd *display = gg->current_display;
  splotd *sp = gg->current_splot;
  gint k;
  gint nsteps = 20;

  ggobi_full_viewmode_set (NULL_PMODE, BRUSH, gg);
  set_time(gg);
  for (k=nsteps; k>0; k--) {
    brush_set_pos (sp->max.x/k, sp->max.y/k, sp);
    brush_once_and_redraw (false, sp, display, gg);
  }

  print_time_interval("brushing", gg);
  set_time(gg);
}


void
time_touring (ggobid *gg) {
  displayd *dsp = gg->current_display;
  GGobiStage *d = dsp->d;
  gint k;
  gint nsteps = 10;
  extern void tour2d_run(displayd *dsp, ggobid *gg);
  extern gboolean tour2d_subset_var_set (gint, GGobiStage *, displayd *, ggobid *);
  extern void tour2d_active_var_set (gint, GGobiStage *, displayd *, ggobid *);

  if (ggobi_stage_get_n_cols(d) < 3)
    return;

  ggobi_full_viewmode_set (TOUR2D, NULL_IMODE, gg);
 
  tour2d_pause (&dsp->cpanel, on, dsp, gg);

  set_time(gg);
  for (k=0; k<nsteps; k++)
    tour2d_run(dsp, gg);
  print_time_interval("tour_default_variables", gg);

  // By default, three variables are selected.  Select the rest.
  gg->tour2d.fade_vars = false;
  for (k=3; k<ggobi_stage_get_n_cols(d); k++) {
    tour2d_subset_var_set(k, d, dsp, gg);
    tour2d_active_var_set (k, d, dsp, gg);
  }
  varcircles_visibility_set (dsp, gg);

  set_time(gg);
  for (k=0; k<nsteps; k++)
    tour2d_run(dsp, gg);
  print_time_interval("tour_all_variables", gg);

}

void
run_timing_tests (ggobid *gg) {
  print_time_interval("init", gg);
  time_brushing(gg);
  time_touring(gg);
  exit(0);
}
