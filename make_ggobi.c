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

  vectors_init (&gg->line.color);
  vectors_init (&gg->line.color_now);
  vectors_init (&gg->line.color_prev);
  vectorb_init (&gg->line.hidden);
  vectorb_init (&gg->line.hidden_now);
  vectorb_init (&gg->line.hidden_prev);
  vectorb_init (&gg->line.xed_by_brush);
}

/*-- initialize variables which DO depend on the size of the data --*/
void modes_init (datad *d, ggobid* gg) {
  brush_init (d, gg);
}

gboolean
fileset_read_init (gchar *ldata_in, datad *d, ggobid *gg)
{
  gboolean ans = fileset_read (ldata_in, d, gg);
  if (ans) {
    dataset_init (d, gg, true);
  }

  return (ans);
} 

displayd *
dataset_init (datad *d, ggobid *gg, gboolean cleanup)
{
  displayd *display = NULL;

  pipeline_init (d, gg);

  if (cleanup)
    display_free_all (gg);  /*-- destroy any existing displays --*/

   /*-- initialize the first display --*/
  display = scatterplot_new (false, NULL, d, gg);
   /* Need to make certain this is the only one there.
      See
    */
  gg->displays = g_list_append (gg->displays, (gpointer) display);
  display_set_current (display, gg);
  gg->current_splot = (splotd *)
    g_list_nth_data (gg->current_display->splots, 0);

  return (display);
}

gboolean
fileset_read (gchar *ldata_in, datad *d, ggobid *gg)
{
  gboolean ok = true;
  gg->filename = g_strdup (ldata_in);
  strip_suffixes (gg);  /*-- produces gg.fname, the root name --*/

  /*
   * the varpanel has to know how many circles and labels to destroy
   * if new data is read in later
  */
  d->varpanel_ui.nvars = d->ncols;

  switch (gg->data_mode) {
   case xml:
#ifdef USE_XML
     ok = data_xml_read (gg->fname, d, gg);
#endif
     break;
   case mysql:
#ifdef USE_MYSQL
{
  extern MySQLLoginInfo DefaultMySQLInfo;
     getDefaultValuesFromFile(ldata_in);
     ok = read_mysql_data(&DefaultMySQLInfo, FALSE, gg);
}
#endif
    break;

   case binary:
     break;
   case Sprocess:
     break;

   case ascii:
     array_read (d, gg);
     d->nrows_in_plot = d->nrows;    /*-- for now --*/
     d->nrgroups = 0;                /*-- for now --*/
      
     missing_values_read (gg->fname, true, d, gg);
      
     collabels_read (gg->fname, true, d, gg);
     rowlabels_read (gg->fname, true, d, gg);
     vgroups_read (gg->fname, true, d, gg);
      
     point_glyphs_read (gg->fname, true, d, gg);
     point_colors_read (gg->fname, true, d, gg);
     hidden_read (gg->fname, true, d, gg);
    
     edges_read (gg->fname, true, d, gg);
     line_colors_read (gg->fname, true, d, gg);
     break;
  }

  return ok;  /* need to check return codes of reading routines */
}

void
pipeline_init (datad *d, ggobid *gg) 
{
  gint i;

  /*-- a handful of allocations and initializations --*/
  pipeline_arrays_alloc (d, gg);
  for (i=0; i<d->nrows; i++) {
    d->rows_in_plot[i] = i;
    d->sampled[i] = true;
  }

  /*-- some initializations --*/
  modes_init (d, gg);
  varpanel_layout_init (d, gg);
  varpanel_clear (d, gg);
  varpanel_populate (d, gg);

  /*-- run the first half of the pipeline --*/
  arrayf_copy (&d->raw, &d->tform1);
  arrayf_copy (&d->tform1, &d->tform2);

  vardata_stats_set (d, gg);

  vardata_lim_raw_gp_set (d, gg);
  vardata_lim_update (d, gg);
  tform_to_world (d, gg);

  if (d->nmissing > 0) {
    missing_lim_set (d, gg);
    missing_world_alloc (d, gg);
    missing_to_world (d, gg);
  }
}

void
make_ggobi (gchar *ldata_in, gboolean processEvents, ggobid *gg) {
  datad *d = (datad *) g_malloc (sizeof (datad));
  gg->d = g_slist_append (gg->d, d);

  /*-- some initializations --*/
  gg->displays = NULL;
  
  d->nrows = d->ncols = 0;

  globals_init (gg); /*-- variables that don't depend on the data --*/
  datad_init (d);
  color_table_init (gg);
  make_ui (gg);

  if (ldata_in != NULL) {
    if (fileset_read (ldata_in, d, gg) > 0) {
      dataset_init (d, gg, true);

    }
  } else {
#ifdef USE_MYSQL
    if(gg->data_mode == mysql) {
      GGOBI(get_mysql_login_info)(NULL, gg);
    }
#endif
  }

  if (processEvents) {
    gtk_main ();
  }
}

