/*-- make_ggobi.c --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
*/
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
fileset_read_init (gchar *ldata_in, ggobid *gg)
{
  gboolean ans = fileset_read (ldata_in, gg);
  if (ans) {
    GSList *l;
    datad *d;
    gboolean firstd = true;
    for (l = gg->d; l; l = l->next) {
      d = (datad *) l->data;
      datad_init (d, gg, firstd);
      firstd = false;
    }
  }

  return (ans);
} 


gboolean
fileset_read (gchar *ldata_in, ggobid *gg)
{
  gboolean ok = true;
  gg->filename = g_strdup (ldata_in);
  strip_suffixes (gg);  /*-- produces gg.fname, the root name --*/

  switch (gg->data_mode) {
    case xml_data:
#ifdef USE_XML
      ok = data_xml_read (gg->fname, gg);
#endif
    break;

    case mysql_data:
#ifdef USE_MYSQL
    {
      extern MySQLLoginInfo DefaultMySQLInfo;
      getDefaultValuesFromFile(ldata_in);
      ok = read_mysql_data(&DefaultMySQLInfo, FALSE, gg);
    }
#endif
    break;

    case binary_data:
    break;

    case Sprocess_data:
    break;

    case ascii_data:
    {
      datad *d; /* datad_new (gg);*/
#ifdef USE_CLASSES
      d  = new datad(gg);
#else
      d = datad_new(NULL, gg);
#endif
      array_read (d, gg);
      d->nrows_in_plot = d->nrows;    /*-- for now --*/
      d->nrgroups = 0;                /*-- for now --*/
      missing_values_read (gg->fname, true, d, gg);
      
      collabels_read (gg->fname, true, d, gg);
      rowlabels_read (gg->fname, true, d, gg);
      
      point_glyphs_read (gg->fname, true, d, gg);
      point_colors_read (gg->fname, true, d, gg);
      hidden_read (gg->fname, true, d, gg);
    
      edges_read (gg->fname, true, d, gg);
      line_colors_read (gg->fname, true, d, gg);
    }
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

  /*-- run the first half of the pipeline --*/
  arrayf_copy (&d->raw, &d->tform);

  vartable_stats_set (d, gg);
  vartable_lim_update (d, gg);
  tform_to_world (d, gg);

  if (d->nmissing > 0) {
    missing_lim_set (d, gg);
    missing_world_alloc (d, gg);
    missing_to_world (d, gg);
  }
}

void
make_ggobi (gchar *ldata_in, gboolean processEvents, ggobid *gg) {
  gboolean init_data = false;

  /*-- some initializations --*/
  gg->displays = NULL;
  
  globals_init (gg); /*-- variables that don't depend on the data --*/
  color_table_init (gg);
  make_ui (gg);

  if (ldata_in != NULL) {
    if (fileset_read (ldata_in, gg) > 0) {
      init_data = true;
    }
  } else {
#ifdef USE_MYSQL
    if(gg->data_mode == mysql) {
      GGOBI(get_mysql_login_info)(NULL, gg);
    }
#endif
  }

  if (init_data) {
    GSList *l;
    datad *d;
    gboolean firstd = true;
    for (l = gg->d; l; l = l->next) {
      d = (datad *) l->data;
      datad_init (d, gg, firstd);
      firstd = false;
    }

    /*-- destroy and rebuild the menu every time data is read in --*/
    display_menu_build (gg);
  }

  if (processEvents) {
    gtk_main ();
  }
}


