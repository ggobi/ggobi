/* xmlConvert.c */
/*
 * ggobi
 * Copyright (C) AT&T, Duncan Temple Lang, Dianne Cook 1999-2005
 *
 * ggobi is free software; you may use, redistribute, and/or modify it
 * under the terms of the Common Public License, which is distributed
 * with the source code and displayed on the ggobi web site, 
 * www.ggobi.org.  For more information, contact the authors:
 *
 *   Deborah F. Swayne   dfs@research.att.com
 *   Di Cook             dicook@iastate.edu
 *   Duncan Temple Lang  duncan@wald.ucdavis.edu
 *   Andreas Buja        andreas.buja@wharton.upenn.edu
*/

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

  fileset_read(fileName, sessionOptions->data_type, NULL, gg);

  memset(&info, '\0', sizeof(XmlWriteInfo));
  info.useDefault = true;
  write_xml_stream (stdout, gg, NULL, &info);

  return(0);
}
