
/*
 A simple application that reads data in the original format
 of xgobi and converts it to the XML format used by ggobi.
 */


#include "ggobi.h"
#include "externs.h"
#include "write_xml.h"

int
main(int argc, char *argv[])
{

 ggobid *gg;
 gchar *tmpname;
  gtk_init(&argc, &argv);
  gg = ggobi_alloc();

  gg->data_mode = ascii;
  gg->nrows_in_plot = -1;
  gg->file_read_type = read_all;
  gg->displays = NULL;
  gg->nrows = gg->ncols = 0;
  globals_init (gg); /*-- variables that don't depend on the data --*/
  color_table_init (gg);

  gg->data_in = argv[1];

  fileset_read(gg->data_in, gg);
  if(0) {
    tmpname = (gchar*) g_malloc(sizeof(gchar)*strlen(gg->fname)+5);
    sprintf(tmpname, "%s%s", gg->fname, ".tmp");
    write_xml(tmpname, gg);
    g_free(tmpname);
  }
  else
    write_xml_stream(stdout, gg, NULL);


return(0);
}
