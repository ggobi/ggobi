
/*
 A simple application that reads data in the original format
 of xgobi and converts it to the XML format used by ggobi.
 */


#include "ggobi.h"
#include "externs.h"
#include "write_xml.h"

gint
main (gint argc, gchar *argv[])
{
  ggobid *gg;
  datad *d;

  gchar *tmpname;
  gtk_init (&argc, &argv);
  gg = ggobi_alloc ();
  d = g_malloc (sizeof (datad));
  gg->d = g_slist_append (gg->d, d);

  gg->data_mode = ascii;
  d->nrows_in_plot = -1;
  d->nrows = d->ncols = 0;
  gg->displays = NULL;
  globals_init (gg); /*-- variables that don't depend on the data --*/
  color_table_init (gg);

  gg->data_in = argv[1];

  fileset_read (gg->data_in, d, gg);
  if(0) {
    tmpname = (gchar*) g_malloc(sizeof(gchar)*strlen(gg->fname)+5);
    sprintf (tmpname, "%s%s", gg->fname, ".tmp");
    write_xml (tmpname, d, gg);
    g_free (tmpname);
  }
  else
    write_xml_stream (stdout, d, gg, NULL);


  return(0);
}
