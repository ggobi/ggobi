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
#include "ggobi-input-source-file.h"

#include "plugin.h"

#define READ_FILESET   0
#define EXTEND_FILESET 1
#define WRITE_FILESET  2

// FIXME: eventually the GGobiDataFactory's should exist within a GGobi context
// so that we don't have to instantiate them just to check the supported modes
GSList *
get_file_filters(ggobid *gg)
{
  GType *factory_types;
  guint n_factory_types, i;
  GSList *filters = NULL;
  GtkFileFilter *unknown_filter = gtk_file_filter_new();
  
  gtk_file_filter_set_name(unknown_filter, "any");
  gtk_file_filter_add_pattern(unknown_filter, "*");
  filters = g_slist_append(filters, unknown_filter);
  
  factory_types = g_type_children(GGOBI_TYPE_DATA_FACTORY, &n_factory_types);
  
  for (i = 0; i < n_factory_types; i++) {
    GObject *factory = g_object_new(factory_types[i], NULL);
    GSList *factory_modes = ggobi_data_factory_get_supported_modes(
      GGOBI_DATA_FACTORY(factory));
    for (GSList *modes = factory_modes; modes; modes = modes->next) {
      GtkFileFilter *filter = gtk_file_filter_new();
      GSList *factory_exts = ggobi_data_factory_get_file_exts_for_mode(
        GGOBI_DATA_FACTORY(factory), modes->data);
      for (GSList *exts = factory_exts; exts; exts = exts->next) {
        gchar *pattern = g_strconcat("*.", exts->data, NULL);
        gtk_file_filter_add_pattern(filter, pattern);
        g_free(pattern);
      }
      gtk_file_filter_set_name(filter, modes->data);
      filters = g_slist_append(filters, filter);
      g_slist_foreach(factory_exts, (GFunc)g_free, NULL);
      g_slist_free(factory_exts);
    }
    g_slist_foreach(factory_modes, (GFunc)g_free, NULL);
    g_slist_free(factory_modes);
    g_object_unref(factory);
  }
  
  return filters;
}

void
filesel_ok (GtkWidget * chooser)
{
  extern const gchar *key_get (void);
  ggobid *gg;
  guint action, len;
  gchar *uri, *filename;
  gboolean firsttime;

  gg = (ggobid *) g_object_get_data (G_OBJECT (chooser), key_get ());
  uri = gtk_file_chooser_get_uri (GTK_FILE_CHOOSER (chooser));
  action = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (chooser), "action"));
  len = strlen (uri);

  switch (action) {
  case READ_FILESET:
    {
      gchar *mode_name;
      GtkWidget *combo;
      GtkFileFilter *filter;
      // FIXME: GTK+ 2.10 has a built-in entry, but it's not labeled "URL" like ours...
      GtkWidget *entry = g_object_get_data(G_OBJECT (chooser), "URLEntry");
      const gchar *url = gtk_entry_get_text(GTK_ENTRY(entry));
      
      if (url && strlen(url)) {
        g_free(uri);
        uri = g_strdup(url);
      }
      
      combo = (GtkWidget *) g_object_get_data (G_OBJECT (chooser),
                                         "InputTypeCombo");
      filter = gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(chooser));
      //mode_name = gtk_file_filter_get_name(filter);
      mode_name = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combo));
      if (!strcmp(mode_name, "any")) {
        g_free(mode_name);
        mode_name = NULL;
      }

      firsttime = (g_slist_length (gg->d) == 0);
      if (load_data (uri, mode_name, gg))
      /*-- destroy and rebuild the menu every time data is read in --*/
        display_menu_build (gg);

      /*
       * If this is the first data read in, we need a call to
       * full_viewmode_set to initialize the mode and projection,
       * and to make the Options menu appear.
       */
      if (firsttime) {
        gg->pmode = XYPLOT;
        ggobi_full_viewmode_set (XYPLOT, DEFAULT_IMODE, gg);
      }
      
      g_free(mode_name);
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
filename_get_configure (GtkWidget * chooser, guint type, ggobid * gg)
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
createInputFileSelectionDialog (gchar * title, ggobid * gg)
{
  GtkWidget *chooser, *combo, *hbox, *lbl;
  GSList *filters, *l;

  filters = get_file_filters(gg);

  chooser =
    gtk_file_chooser_dialog_new (title, NULL, GTK_FILE_CHOOSER_ACTION_OPEN,
                                 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                 GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

  hbox = gtk_hbox_new (false, 5);

  lbl = gtk_label_new_with_mnemonic ("Input _Type:");
  gtk_box_pack_start (GTK_BOX (hbox), lbl, false, false, 0);

  combo = gtk_combo_box_new_text ();
  gtk_label_set_mnemonic_widget (GTK_LABEL (lbl), combo);
  for (l = filters; l; l = l->next) {
    GtkFileFilter *filter = l->data;
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(chooser), filter);
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo), gtk_file_filter_get_name(filter));
  }
  g_slist_free (filters);
  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
  gtk_box_pack_start (GTK_BOX (hbox), combo, false, false, 0);
  g_object_set_data (G_OBJECT (chooser), "InputTypeCombo", combo);

  { // Testing URL reading interface
  GtkWidget *entry;

  lbl = gtk_label_new_with_mnemonic ("_URI:"); 
  gtk_box_pack_start (GTK_BOX (hbox), lbl, false, false, 0);
  entry = gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(entry), 20);
  gtk_label_set_mnemonic_widget (GTK_LABEL (lbl), entry);
  gtk_box_pack_start (GTK_BOX (hbox), entry, true, true, 0);
  g_object_set_data (G_OBJECT (chooser), "URLEntry", entry);
  }

  /*button = gtk_button_new_with_mnemonic("Enter _Location");
    gtk_box_pack_start(GTK_BOX(hbox), button, false, false, 0);
     g_signal_connect(G_OBJECT(button), "clicked", 
     G_CALLBACK(location_button_clicked_cb), chooser); */

  gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER (chooser), hbox);
  gtk_widget_show_all (hbox);

  return (chooser);
}

void configure_file_chooser(GtkWidget *chooser, ggobid *gg)
{
  if (GGOBI_IS_INPUT_SOURCE_FILE(gg->data_source)) {
    gchar *filename = ggobi_input_source_file_get_filename(
      GGOBI_INPUT_SOURCE_FILE(gg->data_source));
    if (filename) {
      gchar *dir = g_path_get_dirname(filename);
      gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (chooser), dir);
      g_free(filename);
      g_free(dir);
    } else g_warning("Could not get filename from file input source");
  }
}

/*--------------------------------------------------------------------------*/
/*                    reading files                                         */
/*--------------------------------------------------------------------------*/

void
filename_get_r (ggobid * gg)
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
filename_get_w (GtkWidget * w, ggobid * gg)
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
