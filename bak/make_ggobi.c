#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <gtk/gtk.h>

#include "vars.h"

/* external functions */
extern void make_ui ();
extern void init_color_table ();
extern void read_array ();
extern void read_collabels (gchar *, gboolean);
extern void read_rowlabels (gchar *, gboolean);
extern void read_vgroups (gchar *, gboolean);
extern void read_point_glyphs (gchar *, gboolean, gboolean);
extern void read_point_colors (gchar *, gboolean, gboolean);
extern void read_erase (gchar *, gboolean, gboolean);
extern void alloc_pipeline_arrays ();
extern void init_mode (cpaneld *);
extern void copy_raw_to_tform ();
extern void update_lims ();
extern void update_world ();
extern displayd * scatterplot_new ();
extern void display_set_current (displayd *);
extern void strip_suffixes ();
/*                    */

void
init_variables() {

  xg.glyph_id.type = xg.glyph_0.type = 1;
  xg.glyph_id.size = xg.glyph_0.size = 2;

  xg.color_id = xg.color_0 = 0;
  xg.point_painting_p = true;

/*
 * scaling
*/
  xg.pan_or_zoom = PAN;
}

void
make_ggobi(gchar *data_in) {
  gint i;
  displayd * display;

  init_variables();
  init_color_table();

  xg.filename = g_strdup(data_in);
  g_printerr("(make_ggobi) filename = %s\n", xg.filename);

  strip_suffixes();

  read_array();
  read_collabels(xg.filename, true);
  read_rowlabels(xg.filename, true);
  read_vgroups(xg.filename, true);
  read_point_glyphs(xg.filename, true, true);
  read_point_colors(xg.filename, true, true);
  read_erase(xg.filename, true, true);

  alloc_pipeline_arrays();
  xg.nrows_in_plot = xg.nrows;
  for (i=0; i<xg.nrows; i++)
    xg.rows_in_plot[i] = i;

  copy_raw_to_tform ();
  update_lims ();
  update_world ();

  make_ui ();

/* Can I new the first scatterplot display before the main ui? */
  displays = null;
  display = scatterplot_new ();
  displays = g_list_append (displays, (gpointer) display);
  display_set_current (display);

  current_splot = (splotd *) g_list_nth_data (current_display->splots, 0);

/*  */

  gtk_main ();
}

