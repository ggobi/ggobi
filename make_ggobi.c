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
  gg->color_id = 0;
  gg->color_0 = 4;
}

/*-- initialize variables which DO depend on the size of the data --*/
void modes_init (datad *d, ggobid* gg) {
  brush_init (d, gg);
}

gboolean
fileset_read_init (const gchar *ldata_in, DataMode data_mode, ggobid *gg)
{
  int howMany;
  gboolean ans;
  howMany = g_slist_length(gg->d);
  ans = fileset_read (ldata_in, data_mode, gg);
  if (ans) {
    datad *d;
    int n, i;
    /* Loop over the newly added datad elements
       and update them.
     */
    n = g_slist_length(gg->d);
    for(i= howMany; i < n; i++) {
       d = (datad *) g_slist_nth_data(gg->d, i);
       datad_init (d, gg,  (i + howMany) == 0);
    }
  }

  return (ans);
} 


gboolean
fileset_read (const gchar *ldata_in, DataMode data_mode, ggobid *gg)
{
  InputDescription *desc;
  gboolean ok = true;
  /*  gg->filename = g_strdup (ldata_in); */

  desc = fileset_generate(ldata_in, sessionOptions->data_mode);

  if(desc == NULL) {
    g_printerr("Cannot locate the file %s\n", ldata_in); 
    return(false);
  }

  if(desc->mode == unknown_data) {
    g_printerr ("Cannot determine the format of the data in file %s\n", desc->fileName); 
    return(false);
  }

  gg->input = desc;

  switch (desc->mode) {
    case xml_data:
#ifdef USE_XML
     ok = data_xml_read (desc, gg);
#else
    g_printerr("No support for reading XML\n");
#endif
    break;

    case mysql_data:
#ifdef USE_MYSQL
    {
      extern MySQLLoginInfo DefaultMySQLInfo;
      getDefaultValuesFromFile(ldata_in);
      ok = read_mysql_data(&DefaultMySQLInfo, FALSE, gg);
    }
#else
    g_printerr("No support for reading MySQL\n");
#endif
    break;

    case binary_data:
    g_printerr("No support for MySQL\n");
    break;

    case Sprocess_data:
    break;

    case ascii_data:
    {
      read_ascii_data(desc, gg);
    }
    break;
   default:
     g_printerr("Unknown data type in fileset_read\n");
     break;
  }

  if(ok && sessionOptions->verbose) {
    showInputDescription(desc, gg);
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
    d->sampled.els[i] = true;
  }

  /*-- some initializations --*/
  modes_init (d, gg);

  /*-- run the first half of the pipeline --*/
  arrayf_copy (&d->raw, &d->tform);

  limits_set (true, true, d);  
  vartable_limits_set (d);  /*-- does this do something here?  --*/
  vartable_stats_set (d);  /*-- does this do something here?  --*/

  tform_to_world (d, gg);

  if (d->nmissing > 0) {
    missing_lim_set (d, gg);
    missing_world_alloc (d, gg);
    missing_to_world (d, gg);
  }
}

/*
 * the first display is initialized in datad_init, so turn on
 * event handlers there as well
*/
void
make_ggobi (GGobiOptions *options, gboolean processEvents, ggobid *gg) {
  gboolean init_data = false;

  /*-- some initializations --*/
  gg->displays = NULL;
  
  globals_init (gg); /*-- variables that don't depend on the data --*/
  color_table_init (gg);
  make_ui (gg);

  if (options->data_in != NULL) {
    if (fileset_read (options->data_in, options->data_mode, gg) > 0) {
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


