#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"

void
variables_init () {
  xg.glyph_id.type = xg.glyph_0.type = 1;
  xg.glyph_id.size = xg.glyph_0.size = 2;
  xg.color_id = xg.color_0 = 0;
  brush_init ();
}

void
make_ggobi(gchar *data_in) {
  gint i;
  displayd *display;

  xg.filename = g_strdup (data_in);
  g_printerr("(make_ggobi) filename = %s\n", xg.filename);

  strip_suffixes ();

  array_read ();
  xg.nrows_in_plot = xg.nrows;
  xg.nlinkable = xg.nrows;
  xg.nrgroups = 0;

  /*-- read the .missing file, if there is one --*/
  missing_values_read (xg.filename, true);

  pipeline_arrays_alloc ();
  for (i=0; i<xg.nrows; i++)
    xg.rows_in_plot[i] = i;
  raw_to_tform_copy ();

  vardata_stats_set ();

  variables_init ();
  color_table_init ();

  collabels_read (xg.filename, true);
  rowlabels_read (xg.filename, true);
  vgroups_read (xg.filename, true);
  point_glyphs_read (xg.filename, true);
  point_colors_read (xg.filename, true);
  erase_read (xg.filename, true);

  segments_read (xg.filename, true);
  line_colors_read (xg.filename, true);

  vardata_lim_raw_gp_set ();
  vardata_lim_update ();
  tform_to_world ();

  if (xg.nmissing > 0) {
    missing_lim_set ();
    missing_world_alloc ();
    missing_to_world ();
  }

  make_ui ();

  displays = NULL;
  display = scatterplot_new (false);
  displays = g_list_append (displays, (gpointer) display);
  display_set_current (display);
  current_splot = (splotd *) g_list_nth_data (current_display->splots, 0);

  gtk_main ();
}

