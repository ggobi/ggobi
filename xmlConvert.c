
/*
 A simple application that reads data in the original format
 of xgobi and converts it to the XML format used by ggobi.
 */

#include <gtk/gtk.h>
#include "write_xml.h"

#include <string.h>

gint
main (gint argc, gchar *argv[])
{
  ggobid *gg;
  gchar *fileName;
  XmlWriteInfo info;

  initSessionOptions(argc, argv);

  gtk_init (&argc, &argv);
  gg = ggobi_alloc (gg);

  /*parse_command_line(&argc, argv, gg);*/
  parse_command_line(&argc, argv);

  fileName = argv[1];

  gg->displays = NULL;
  globals_init (gg); /*-- variables that don't depend on the data --*/
  special_colors_init(gg);

  fileset_read (fileName, gg);

  memset(&info, '\0', sizeof(XmlWriteInfo));
  info.useDefault = true;
  write_xml_stream (stdout, gg, NULL, &info);

  return(0);
}
