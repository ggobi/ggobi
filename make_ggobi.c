#include <math.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/*#include <pthread.h>*/

#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"

/*pthread_t tour2d_tid;*/
/*extern void * tour_thread ();*/


/*-- initialize variables which don't depend on the size of the data --*/
void globals_init () {
  xg.glyph_id.type = xg.glyph_0.type = FILLED_CIRCLE;
  xg.glyph_id.size = xg.glyph_0.size = 3;
  xg.color_id = xg.color_0 = 0;

  /*-- initialize arrays to NULL --*/
  arrayf_init (&xg.raw);
  arrayf_init (&xg.tform1);
  arrayf_init (&xg.tform2);
  arrayl_init (&xg.world);
  arrayl_init (&xg.jitter);

  arrays_init (&xg.missing);
  arrayl_init (&xg.missing_world);
  arrayl_init (&xg.missing_jitter);
}

/*-- initialize variables which DO depend on the size of the data --*/
void modes_init () {
  brush_init ();
}

gboolean
fileset_read_init (gchar *ldata_in)
{
  gboolean ans = fileset_read (ldata_in);
  if (ans) {
    dataset_init(&xg);
  }

 return (ans);
} 

void
dataset_init(xgobid *xg)
{
    displayd *display;

    pipeline_init ();


    /*-- initialize the first display --*/
    display = scatterplot_new (false);
    /* Need to make certain this is the only one there.
       See
     */
    xg->displays = g_list_append (xg->displays, (gpointer) display);
    display_set_current (display);
    xg->current_splot = (splotd *)
      g_list_nth_data (xg->current_display->splots, 0);
}


gboolean
fileset_read (gchar *ldata_in)
{
  xg.filename = g_strdup (ldata_in);
  strip_suffixes ();  /*-- produces xg.fname, the root name --*/

  array_read ();
  xg.nrows_in_plot = xg.nrows;  /*-- for now --*/
  xg.nlinkable = xg.nrows;      /*-- for now --*/
  xg.nrgroups = 0;              /*-- for now --*/

  missing_values_read (xg.fname, true);

  collabels_read (xg.fname, true);
  rowlabels_read (xg.fname, true);
  vgroups_read (xg.fname, true);

  point_glyphs_read (xg.fname, true);
  point_colors_read (xg.fname, true);
  hidden_read (xg.fname, true);

  segments_read (xg.fname, true);
  line_colors_read (xg.fname, true);

  return true;  /* need to check return codes of reading routines */
}

void
pipeline_init () 
{
  gint i;

  /*-- a handful of allocations and initializations --*/
  pipeline_arrays_alloc ();
  for (i=0; i<xg.nrows; i++) {
    xg.rows_in_plot[i] = i;
    xg.sampled[i] = true;
  }

  /*-- some initializations --*/
  modes_init ();
  varpanel_layout_init ();
  varpanel_populate ();

  /*-- run the first half of the pipeline --*/
  arrayf_copy (&xg.raw, &xg.tform1);
  arrayf_copy (&xg.tform1, &xg.tform2);

  vardata_stats_set ();

  vardata_lim_raw_gp_set ();
  vardata_lim_update ();
  tform_to_world ();

  if (xg.nmissing > 0) {
    missing_lim_set ();
    missing_world_alloc ();
    missing_to_world ();
  }
}

void
make_ggobi (gchar *ldata_in, gboolean processEvents) {
  displayd *display;

  /*-- some initializations --*/
  xg.displays = NULL;
  xg.nrows = xg.ncols = 0;

  globals_init (); /*-- variables that don't depend on the data --*/
  color_table_init ();
  make_ui ();

  if (ldata_in != NULL) {
    if (fileset_read (ldata_in)) {
      pipeline_init ();

      display_free_all ();  /*-- destroy any existing displays --*/

      /*-- initialize the first display --*/
      display = scatterplot_new (false);
      xg.displays = g_list_append (xg.displays, (gpointer) display);
      display_set_current (display);
      xg.current_splot = (splotd *)
        g_list_nth_data (xg.current_display->splots, 0);
    }
  }

/*  pthread_create (&tour2d_tid, NULL, tour_thread, NULL);*/

  if (processEvents) {
/*    gdk_threads_enter ();*/
    gtk_main ();
/*    gdk_threads_leave ();*/
  }
}

