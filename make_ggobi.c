#include <math.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"


/*-- initialize variables which don't depend on the size of the data --*/
void globals_init () {
  gg.glyph_id.type = gg.glyph_0.type = FILLED_CIRCLE;
  gg.glyph_id.size = gg.glyph_0.size = 3;
  gg.color_id = gg.color_0 = 0;

  /*-- initialize arrays to NULL --*/
  arrayf_init (&gg.raw);
  arrayf_init (&gg.tform1);
  arrayf_init (&gg.tform2);
  arrayl_init (&gg.world);
  arrayl_init (&gg.jitter);

  arrays_init (&gg.missing);
  arrayl_init (&gg.missing_world);
  arrayl_init (&gg.missing_jitter);

  vectori_init (&gg.clusterid);
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
    dataset_init(&gg);
  }

 return (ans);
} 

void
dataset_init(ggobid *gg)
{
    displayd *display;

    pipeline_init ();


    /*-- initialize the first display --*/
    display = scatterplot_new (false, NULL);
    /* Need to make certain this is the only one there.
       See
     */
    gg->displays = g_list_append (gg->displays, (gpointer) display);
    display_set_current (display);
    gg->current_splot = (splotd *)
      g_list_nth_data (gg->current_display->splots, 0);
}


gboolean
fileset_read (gchar *ldata_in)
{
  gg.filename = g_strdup (ldata_in);
  strip_suffixes ();  /*-- produces gg.fname, the root name --*/

  array_read ();
  gg.nrows_in_plot = gg.nrows;  /*-- for now --*/
  gg.nlinkable = gg.nrows;      /*-- for now --*/
  gg.nrgroups = 0;              /*-- for now --*/

  missing_values_read (gg.fname, true);

  collabels_read (gg.fname, true);
  rowlabels_read (gg.fname, true);
  vgroups_read (gg.fname, true);

  point_glyphs_read (gg.fname, true);
  point_colors_read (gg.fname, true);
  hidden_read (gg.fname, true);

  segments_read (gg.fname, true);
  line_colors_read (gg.fname, true);

  return true;  /* need to check return codes of reading routines */
}

void
pipeline_init () 
{
  gint i;

  /*-- a handful of allocations and initializations --*/
  pipeline_arrays_alloc ();
  for (i=0; i<gg.nrows; i++) {
    gg.rows_in_plot[i] = i;
    gg.sampled[i] = true;
  }

  /*-- some initializations --*/
  modes_init ();
  varpanel_layout_init ();
  varpanel_populate ();

  /*-- run the first half of the pipeline --*/
  arrayf_copy (&gg.raw, &gg.tform1);
  arrayf_copy (&gg.tform1, &gg.tform2);

  vardata_stats_set ();

  vardata_lim_raw_gp_set ();
  vardata_lim_update ();
  tform_to_world ();

  if (gg.nmissing > 0) {
    missing_lim_set ();
    missing_world_alloc ();
    missing_to_world ();
  }
}

void
make_ggobi (gchar *ldata_in, gboolean processEvents) {
  displayd *display;

  /*-- some initializations --*/
  gg.displays = NULL;
  gg.nrows = gg.ncols = 0;

  globals_init (); /*-- variables that don't depend on the data --*/
  color_table_init ();
  make_ui ();

  if (ldata_in != NULL) {
    if (fileset_read (ldata_in)) {
      pipeline_init ();

      display_free_all ();  /*-- destroy any existing displays --*/

      /*-- initialize the first display --*/
      display = scatterplot_new (false, NULL);
      gg.displays = g_list_append (gg.displays, (gpointer) display);

      display_set_current (display);
      gg.current_splot = (splotd *)
        g_list_nth_data (gg.current_display->splots, 0);
    }
  }

  if (processEvents) {
    gtk_main ();
  }
}

