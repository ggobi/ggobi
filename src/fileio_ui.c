/*-- io.c -- routines that support the file io user interface --*/
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


#include <string.h>
#ifdef USE_STRINGS_H
#include <strings.h>
#endif

#include <unistd.h>

#include <gtk/gtk.h>

#include "vars.h"
#include "externs.h"

#include "writedata.h"
#include "write_xml.h"

#define READ_FILESET   0
#define EXTEND_FILESET 1
#define WRITE_FILESET  2

GSList *
get_file_filters(GGobiSession *gg)
{
  GSList *filters = NULL;
  GtkFileFilter *unknown_filter = gtk_file_filter_new();
  
  gtk_file_filter_set_name(unknown_filter, "any");
  gtk_file_filter_add_pattern(unknown_filter, "*");
  filters = g_slist_append(filters, unknown_filter);
  
  GSList *factories = gg->data_factories;
  
  for (; factories; factories = factories->next) {
    GGobiDataFactory *factory = GGOBI_DATA_FACTORY(factories->data);
    const gchar* mime_type = ggobi_data_factory_get_mime_type(factory);
    //const gchar* content_type = g_content_type_from_mime_type(mime_type);
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_add_mime_type(filter, mime_type);
    gtk_file_filter_set_name(filter, mime_type);
    // FIXME: above should set name to:
    //g_content_type_get_description(content_type));
    filters = g_slist_prepend(filters, filter);
  }
  
  return g_slist_reverse(filters);
}

void
filesel_ok (GtkWidget * chooser)
{
  extern const gchar *key_get (void);
  GGobiSession *gg;
  guint action, len;
  gchar *uri, *filename;
  
  gg = (GGobiSession *) g_object_get_data (G_OBJECT (chooser), key_get ());
  uri = gtk_file_chooser_get_uri (GTK_FILE_CHOOSER (chooser));
  action = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (chooser), "action"));
  len = strlen (uri);

  switch (action) {
  case READ_FILESET:
    {
      if (load_data (uri, gg))
      /*-- destroy and rebuild the menu every time data is read in --*/
        display_menu_build (gg);
    }
    break;
  case EXTEND_FILESET:  /*-- not yet enabled --*/
    break;
  case WRITE_FILESET:
    switch (gg->save.format) {
    case XMLDATA:
    {
      XmlWriteInfo *info = g_new0(XmlWriteInfo, 1);

      /*-- if uri already contains ".xml", then don't add it --*/
      if (len >= 4 && g_strncasecmp (&uri[len - 4], ".xml", 4) == 0)
        filename = g_strdup (uri);
      else
        filename = g_strdup_printf ("%s.xml", uri);

      info->useDefault = true;
      write_xml ((const gchar *) filename, gg, info);
      g_free (filename);
      g_free(info);
    }
    break;
    case CSVDATA:
      /*-- if uri already contains ".csv", then don't add it --*/
      if (len >= 4 && g_strncasecmp (&uri[len - 4], ".csv", 4) == 0)
        filename = g_strdup (uri);
      else
        filename = g_strdup_printf ("%s.csv", uri);

      g_printerr ("writing %s\n", filename);
      write_csv ((const gchar *) filename, gg);
      g_free (filename);
      break;
    case BINARYDATA:    /*-- not yet implemented --*/
      break;
    case MYSQL_DATA:    /*-- never will be implemented --*/
      break;
    }
    break;
  }

  g_free (uri);
}

static void
filename_get_configure (GtkWidget * chooser, guint type, GGobiSession * gg)
{
  extern const gchar *key_get (void);

  const gchar *key = key_get ();
  g_object_set_data (G_OBJECT (chooser), "action", GINT_TO_POINTER (type));
  g_object_set_data (G_OBJECT (chooser), key, gg);
}

GtkWidget *
createOutputFileSelectionDialog (const gchar * title)
{
  GtkWidget *chooser;
  chooser = gtk_file_chooser_dialog_new (title, NULL,
                                         GTK_FILE_CHOOSER_ACTION_SAVE,
                                         GTK_STOCK_CANCEL,
                                         GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE,
                                         GTK_RESPONSE_ACCEPT, NULL);
  return (chooser);
}

GtkWidget *
createInputFileSelectionDialog (gchar * title, GGobiSession * gg)
{
  GtkWidget *chooser;
  GSList *filters, *l;

  filters = get_file_filters(gg);

  chooser =
    gtk_file_chooser_dialog_new (title, NULL, GTK_FILE_CHOOSER_ACTION_OPEN,
                                 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                 GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

  for (l = filters; l; l = l->next) {
    GtkFileFilter *filter = l->data;
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), filter);
  }
  g_slist_free (filters);
  
  return (chooser);
}

void configure_file_chooser(GtkWidget *chooser, GGobiSession *gg)
{
  if (G_IS_FILE(gg->data_source)) {
    GFile *file = gg->data_source;
    GFile *parent = g_file_get_parent(file);
    gchar *filename = g_file_get_path(parent);
    if (filename) {
      gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER (chooser), filename);
      g_free(filename);
    } else g_warning("Could not get filename from file input source");
  }
}

/*--------------------------------------------------------------------------*/
/*                    reading files                                         */
/*--------------------------------------------------------------------------*/

void
filename_get_r (GGobiSession * gg)
{
  GtkWidget *chooser;
  chooser = createInputFileSelectionDialog ("Read ggobi data", gg);

  configure_file_chooser(chooser, gg);

  filename_get_configure (chooser, READ_FILESET, gg);

  if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT)
    filesel_ok (chooser);
  gtk_widget_destroy (chooser);
}

/*--------------------------------------------------------------------------*/
/*                    writing files                                         */
/*--------------------------------------------------------------------------*/

void
filename_get_w (GtkWidget * w, GGobiSession * gg)
{
  GtkWidget *chooser;
  const gchar *title;

  if (gg->save.format == XMLDATA)
    title = "Specify base name for new xml file";
  else if (gg->save.format == CSVDATA)
    title = "Specify base name for new csv file";
  else 
    title = "Specify base name"; // Should not happen

  /* Here we would get from somewhere a list of datads in the save
   * window.  If writing in csv, loop through the list, opening the
   * chooser once for each csv file.   I think I'll stick with one
   * file for the csv mode for the time being ...
   */

  chooser = createOutputFileSelectionDialog (title);

  configure_file_chooser(chooser, gg);

  filename_get_configure (chooser, WRITE_FILESET, gg);

  if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT)
    filesel_ok (chooser);
  gtk_widget_destroy (chooser);
}
