
/*
 A simple application that reads data in the original format
 of xgobi and converts it to the XML format used by ggobi.
 */


#include <gtk/gtk.h>
#include "write_xml.h"

gint
main (gint argc, gchar *argv[])
{
  ggobid *gg;
  datad *d;
  char *fileName;
  gchar *tmpname;
  gtk_init (&argc, &argv);
  gg = ggobi_alloc ();


  fileName = gg->data_in = argv[1];
  gg->data_mode = ascii_data;
 
  fileName = gg->data_in = argv[2];
  gg->data_mode = xml_data;

  gg->displays = NULL;
  globals_init (gg); /*-- variables that don't depend on the data --*/
  color_table_init (gg);


  fileset_read (fileName, gg);
  d = (datad *)g_slist_nth_data(gg->d,0);

  if(0) {
    tmpname = (gchar*) g_malloc(sizeof(gchar)*strlen(gg->fname)+5);
    sprintf (tmpname, "%s%s", gg->fname, ".tmp");
    write_xml (tmpname, gg);
    g_free (tmpname);
  }
  else {
    write_xml_stream (stdout, gg, NULL);
  }

  return(0);
}
