#include <math.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"

#ifdef USE_XML
#include "read_xml.h"
#endif

#ifdef USE_MYSQL
#include "read_mysql.h"
#endif


/*-- initialize variables which don't depend on the size of the data --*/
void globals_init (ggobid *gg) {
  gg->glyph_id.type = gg->glyph_0.type = FILLED_CIRCLE;
  gg->glyph_id.size = gg->glyph_0.size = 3;
  gg->color_id = gg->color_0 = 0;

  /*-- initialize arrays to NULL --*/
  arrayf_init (&gg->raw);
  arrayf_init (&gg->tform1);
  arrayf_init (&gg->tform2);
  arrayl_init (&gg->world);
  arrayl_init (&gg->jitdata);

  arrays_init (&gg->missing);
  arrayl_init (&gg->missing_world);
  arrayl_init (&gg->missing_jitter);

  vectori_init (&gg->clusterid);

  vectors_init (&gg->line.color);
  vectors_init (&gg->line.color_now);
  vectors_init (&gg->line.color_prev);
  vectorb_init (&gg->line.hidden);
  vectorb_init (&gg->line.hidden_now);
  vectorb_init (&gg->line.hidden_prev);
}

/*-- initialize variables which DO depend on the size of the data --*/
void modes_init (ggobid* gg) {
  brush_init (gg);
}


gboolean
fileset_read_init (gchar *ldata_in, ggobid *gg)
{
  gboolean ans = fileset_read (ldata_in, gg);
  if (ans) {
    dataset_init (gg, true);
  }

  return (ans);
} 

displayd *
dataset_init (ggobid *gg, gboolean cleanup)
{
  displayd *display = NULL;

  pipeline_init (gg);

  if (cleanup)
    display_free_all (gg);  /*-- destroy any existing displays --*/

  if (gg->ncols > 1) {
     /*-- initialize the first display --*/
    display = scatterplot_new (false, NULL, gg);
     /* Need to make certain this is the only one there.
        See
      */
    gg->displays = g_list_append (gg->displays, (gpointer) display);
    display_set_current (display, gg);
    gg->current_splot = (splotd *)
    g_list_nth_data (gg->current_display->splots, 0);
  }

  return (display);
}

gboolean
fileset_read (gchar *ldata_in, ggobid *gg)
{
  gboolean ok = true;
  gg->filename = g_strdup (ldata_in);
  strip_suffixes (gg);  /*-- produces gg.fname, the root name --*/

  /*-- the varpanel has to know how many circles and labels to destroy --*/
  gg->varpanel_ui.nvars = gg->ncols;

  switch (gg->data_mode) {
   case xml:
#ifdef USE_XML
     ok = data_xml_read (gg->fname, gg);
#endif
     break;
   case mysql:
#ifdef USE_MYSQL
     ok = read_mysql_data (NULL, gg);
#endif
    break;

   case binary:
     break;

   case ascii:
     array_read (gg);
     gg->nrows_in_plot = gg->nrows;  /*-- for now --*/
     gg->nrgroups = 0;               /*-- for now --*/
      
     missing_values_read (gg->fname, true, gg);
      
     collabels_read (gg->fname, true, gg);
     rowlabels_read (gg->fname, true, gg);
     vgroups_read (gg->fname, true, gg);
      
     point_glyphs_read (gg->fname, true, gg);
     point_colors_read (gg->fname, true, gg);
     hidden_read (gg->fname, true, gg);
    
     segments_read (gg->fname, true, gg);
     line_colors_read (gg->fname, true, gg);
     break;
  }

  return ok;  /* need to check return codes of reading routines */
}

void
pipeline_init (ggobid *gg) 
{
  gint i;

  /*-- a handful of allocations and initializations --*/
  pipeline_arrays_alloc (gg);
  for (i=0; i<gg->nrows; i++) {
    gg->rows_in_plot[i] = i;
    gg->sampled[i] = true;
  }

  /*-- some initializations --*/
  modes_init (gg);
  varpanel_layout_init (gg);
  varpanel_clear (gg);
  varpanel_populate (gg);

  /*-- run the first half of the pipeline --*/
  arrayf_copy (&gg->raw, &gg->tform1);
  arrayf_copy (&gg->tform1, &gg->tform2);

  vardata_stats_set (gg);

  vardata_lim_raw_gp_set (gg);
  vardata_lim_update (gg);
  tform_to_world (gg);

  if (gg->nmissing > 0) {
    missing_lim_set (gg);
    missing_world_alloc (gg);
    missing_to_world (gg);
  }
}

void
make_ggobi (gchar *ldata_in, gboolean processEvents, ggobid *gg) {
  /*-- some initializations --*/
  gg->displays = NULL;
  gg->nrows = gg->ncols = 0;

  globals_init (gg); /*-- variables that don't depend on the data --*/
  color_table_init (gg);
  make_ui (gg);

  if (ldata_in != NULL) {
    if (fileset_read (ldata_in, gg)) {
      dataset_init (gg, true);
    }
  } else {
#ifdef USE_MYSQL
    if (gg->data_mode == mysql) {
      GGOBI(get_mysql_login_info)(gg);
    }
#endif
  }

  if (processEvents) {
    gtk_main ();
  }
}

