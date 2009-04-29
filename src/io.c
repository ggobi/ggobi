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


/*--         (should be called fileio_ui.c)                   --*/

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

#include "plugin.h"

#define READ_FILESET   0
#define EXTEND_FILESET 1
#define WRITE_FILESET  2



void
filesel_ok (GtkWidget * chooser)
{
  extern const gchar *key_get (void);
  gchar *pluginModeName;
  ggobid *gg;
  guint action, len;
  gchar *fname, *filename;
  gboolean firsttime;

  gg = (ggobid *) g_object_get_data (G_OBJECT (chooser), key_get ());
  fname = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
  action = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (chooser), "action"));
  len = strlen (fname);

  switch (action) {
  case READ_FILESET:
    {
      gint which;
      GGobiPluginInfo *plugin;
      GtkWidget *combo;

      combo =
        (GtkWidget *) g_object_get_data (G_OBJECT (chooser),
                                         "PluginTypeCombo");
      which = gtk_combo_box_get_active (GTK_COMBO_BOX (combo));

      { // Testing URL reading interface
        GtkWidget *entry;
        gchar *url;
        GList *els, *l;
        gint k = 0;

        entry = (GtkWidget *) g_object_get_data (G_OBJECT (chooser),
                                                           "URLEntry");
        if (entry) {
          url = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
          if (g_utf8_strlen(url, -1) > 0) {
            fname = url; // Reset fname
            if (which == 0) {
              els = getInputPluginSelections (gg);
              for (l = els, k = 0; l; l = l->next, k++) {
                if (g_ascii_strncasecmp((gchar *) l->data, "url", 3) == 0)
                  break;
              }
              which = k;  // Reset combo box value
            }
          }
        }
      }

      plugin = getInputPluginByModeNameIndex (which, &pluginModeName);
      firsttime = (g_slist_length (gg->d) == 0);
      if (fileset_read_init (fname, pluginModeName, plugin, gg))
      /*-- destroy and rebuild the menu every time data is read in --*/
        display_menu_build (gg);

      g_free (pluginModeName);

      /*
       * If this is the first data read in, we need a call to
       * full_viewmode_set to initialize the mode and projection,
       * and to make the Options menu appear.
       */
      /*if (firsttime) {
        gg->pmode = XYPLOT;
        GGOBI (full_viewmode_set) (XYPLOT, DEFAULT_IMODE, gg);
        }*/
    }
    break;
  case EXTEND_FILESET:  /*-- not yet enabled --*/
    break;
  case WRITE_FILESET:
    switch (gg->save.format) {
    case XMLDATA:
    {
      XmlWriteInfo *info = g_new0(XmlWriteInfo, 1);

      /*-- if fname already contains ".xml", then don't add it --*/
      if (len >= 4 && g_strncasecmp (&fname[len - 4], ".xml", 4) == 0)
        filename = g_strdup (fname);
      else
        filename = g_strdup_printf ("%s.xml", fname);

      info->useDefault = true;
      write_xml ((const gchar *) filename, gg, info);
      g_free (filename);
      g_free(info);
    }
    break;
    case CSVDATA:
      /*-- if fname already contains ".csv", then don't add it --*/
      if (len >= 4 && g_strncasecmp (&fname[len - 4], ".csv", 4) == 0)
        filename = g_strdup (fname);
      else
        filename = g_strdup_printf ("%s.csv", fname);

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

  g_free (fname);
}

static void
filename_get_configure (GtkWidget * chooser, guint type, ggobid * gg)
{
  extern const gchar *key_get (void);

  const gchar *key = key_get ();
  g_object_set_data (G_OBJECT (chooser), "action", GINT_TO_POINTER (type));
  g_object_set_data (G_OBJECT (chooser), key, gg);

  if (gg->input && gg->input->baseName) {
    gchar *cwd = g_get_current_dir();
    gchar *dir;
    if (g_path_is_absolute(gg->input->dirName))
      dir = g_strdup(gg->input->dirName);
    else
      dir = g_build_filename(cwd, gg->input->dirName, NULL);
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (chooser), dir);
    g_free(dir);
    g_free(cwd);
  }
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
  GList *els, *l;

  els = getInputPluginSelections (gg);

  chooser =
    gtk_file_chooser_dialog_new (title, NULL, GTK_FILE_CHOOSER_ACTION_OPEN,
                                 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                 GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

  hbox = gtk_hbox_new (false, 5);

  lbl = gtk_label_new_with_mnemonic ("Input _Type:");
  gtk_box_pack_start (GTK_BOX (hbox), lbl, false, false, 0);

  combo = gtk_combo_box_new_text ();
  gtk_label_set_mnemonic_widget (GTK_LABEL (lbl), combo);
  for (l = els; l; l = l->next) {
    gtk_combo_box_append_text (GTK_COMBO_BOX (combo), l->data);
    g_free (l->data);
  }
  g_list_free (els);
  gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
  gtk_box_pack_start (GTK_BOX (hbox), combo, false, false, 0);
  g_object_set_data (G_OBJECT (chooser), "PluginTypeCombo", combo);

  { // Testing URL reading interface
  GtkWidget *entry;

  lbl = gtk_label_new_with_mnemonic ("_URL:"); 
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

/*--------------------------------------------------------------------------*/
/*                    reading files                                         */
/*--------------------------------------------------------------------------*/

void
filename_get_r (ggobid * gg)
{
  GtkWidget *chooser;
  chooser = createInputFileSelectionDialog ("Read ggobi data", gg);

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

  filename_get_configure (chooser, WRITE_FILESET, gg);

  if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT)
    filesel_ok (chooser);
  gtk_widget_destroy (chooser);
}
