#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"

/*-- initialize variables which don't depend on the size of the data --*/
void globals_init () {
  xg.glyph_id.type = xg.glyph_0.type = FILLED_CIRCLE;
  xg.glyph_id.size = xg.glyph_0.size = 3;
  xg.color_id = xg.color_0 = 0;
}

/*-- initialize variables which DO depend on the size of the data --*/
void modes_init () {
  brush_init ();
}

gboolean
fileset_read (gchar *data_in)
{
  xg.filename = g_strdup (data_in);
  strip_suffixes ();

  array_read ();
  xg.nrows_in_plot = xg.nrows;  /*-- for now --*/
  xg.nlinkable = xg.nrows;      /*-- for now --*/
  xg.nrgroups = 0;              /*-- for now --*/

  missing_values_read (xg.filename, true);

  collabels_read (xg.filename, true);
  rowlabels_read (xg.filename, true);
  vgroups_read (xg.filename, true);

  point_glyphs_read (xg.filename, true);
  point_colors_read (xg.filename, true);
  erase_read (xg.filename, true);

  segments_read (xg.filename, true);
  line_colors_read (xg.filename, true);

  return true;  /* need to check return codes of reading routines */
}

void
pipeline_init () 
{
  gint i;

  /*-- a handful of allocations and initializations --*/
  pipeline_arrays_alloc ();
  for (i=0; i<xg.nrows; i++)
    xg.rows_in_plot[i] = i;

  /*-- some initializations --*/
  modes_init ();
  varpanel_layout_init ();
  varpanel_populate ();

  /*-- run the first half of the pipeline --*/
  raw_to_tform_copy ();
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
make_ggobi (gchar *data_in) {
  displayd *display;
g_printerr ("(make_ggobi) data_in = %s\n", data_in);

  /*-- some initializations --*/
  displays = NULL;
  xg.nrows = xg.ncols = 0;

  globals_init (); /*-- variables that don't depend on the data --*/
  color_table_init ();
  make_ui ();

  if (data_in != NULL) {
    if (fileset_read (data_in)) {
      pipeline_init ();

      /*-- initialize the first display --*/
      display = scatterplot_new (false);
      displays = g_list_append (displays, (gpointer) display);
      display_set_current (display);
      current_splot = (splotd *) g_list_nth_data (current_display->splots, 0);
    }
  }

  gtk_main ();
}

