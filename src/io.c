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
filesel_ok (GtkWidget *chooser)
{
  extern const gchar* key_get (void);
  gchar *pluginModeName;
  ggobid *gg;
  guint action, len;
  gchar *fname, *filename;
  gboolean firsttime;

  gg = (ggobid *) g_object_get_data(G_OBJECT(chooser), key_get());
  fname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
  action = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(chooser), "action"));
  len = strlen (fname);

  switch (action) {
    case READ_FILESET:
    {
      gint which;
      GGobiPluginInfo *plugin;
	  GtkWidget *combo;
	  
      combo = (GtkWidget *)g_object_get_data(G_OBJECT(chooser), "PluginTypeCombo");
      which = gtk_combo_box_get_active(GTK_COMBO_BOX(combo));
      plugin = getInputPluginByModeNameIndex(which, &pluginModeName);
      firsttime = (g_slist_length (gg->d) == 0);
      if (fileset_read_init (fname, pluginModeName, plugin, gg)) 
		  /*-- destroy and rebuild the menu every time data is read in --*/
        display_menu_build (gg);

	  g_free(pluginModeName);
	  
      /*
       * If this is the first data read in, we need a call to
       * full_viewmode_set to initialize the mode and projection,
       * and to make the Options menu appear.
      */
      if (firsttime) {
        gg->pmode = XYPLOT;
        GGOBI(full_viewmode_set) (XYPLOT, DEFAULT_IMODE, gg);
      }
    }
    break;
    case EXTEND_FILESET:  /*-- not yet enabled --*/
    break;
    case WRITE_FILESET:
      switch (gg->save.format) {
      case XMLDATA:
         {
           XmlWriteInfo info;

          /*-- if fname already contains ".xml", then don't add it --*/
          if (len >= 4 && g_strncasecmp (&fname[len-4], ".xml", 4) == 0)
            filename = g_strdup (fname);
          else
            filename = g_strdup_printf ("%s.xml", fname);

          memset(&info, '0',sizeof(XmlWriteInfo));
          info.useDefault = true;
          write_xml ((const gchar *) filename, gg, &info);
          g_free (filename);
        }
        break;
        case ASCIIDATA:
        {
          datad *d = NULL;
          GSList *l = gg->d;
          gchar *name;
          gint k;
          gint nd = g_slist_length (gg->d);
          
          /*-- if fname already contains ".dat", then strip it off --*/
          if (len >= 4 && g_strncasecmp (&fname[len-4], ".dat", 4) == 0)
            filename = g_strndup (fname, len-4);
          else
            filename = g_strdup (fname);

          k = 0;
          while (l) {
            d = (datad *) l->data;
            name = (nd > 1) ?
              g_strdup_printf ("%s%d", filename, k) : g_strdup (filename);
            ggobi_file_set_create (name, d, gg);
            l = l->next;
            k++;
            g_free (name);
          }


          g_free (filename);
        }
        break;
        case BINARYDATA:  /*-- not yet implemented --*/
        break;
        case MYSQL_DATA:  /*-- never will be implemented --*/
        break;
      }
      break;
  }
  
  g_free(fname);
}

static void
filename_get_configure (GtkWidget *chooser, guint type, ggobid *gg) 
{
  extern const gchar* key_get (void);

  const gchar *key = key_get();
  g_object_set_data(G_OBJECT (chooser), "action", GINT_TO_POINTER (type));
  g_object_set_data(G_OBJECT (chooser), key, gg);

  #if 0
  g_signal_connect (G_OBJECT (GTK_FILE_CHOOSER(chooser)->ok_button),
                      "clicked", G_CALLBACK (filesel_ok), (gpointer) chooser);
                            
  /*-- Ensure that the dialog box is destroyed. --*/
    
  g_signal_connect_swapped (G_OBJECT (GTK_FILE_CHOOSER(chooser)->ok_button),
                             "clicked", G_CALLBACK (gtk_widget_destroy),
                             G_OBJECT(chooser));

  g_signal_connect_swapped (G_OBJECT (GTK_FILE_CHOOSER(chooser)->cancel_button),
                             "clicked", G_CALLBACK (gtk_widget_destroy),
                             G_OBJECT(chooser));
#endif
}

#if 0
GtkWidget *
getFileSelectionWorkContainer(GtkWidget *chooser)
{
       GtkWidget *dlg, *vbox = NULL;

       GList *kids;
       kids = gtk_container_get_children(GTK_CONTAINER(chooser));
       dlg = g_list_nth_data(kids, 0);
       return(dlg);
/*
       kids = gtk_container_get_children(GTK_CONTAINER(dlg));
       vbox = g_list_nth_data(kids, 0);
       if(!GTK_IS_VBOX(vbox)) {
          g_printerr("Not vbox\n");
       }
*/

       return(vbox);
}
#endif
/*
static void
filename_mode_selection_cb(GtkList *l, GtkWidget *el, GtkWidget *data)
{
    
    gint *i = g_object_get_data(G_OBJECT(data), ".selectedElement");
    if(i) {
      *i = gtk_list_child_position(l, el);
    }
}

void
free_gdata(GtkObject *src, gpointer data)
{
  g_free(data);
}
*/

GtkWidget*
createOutputFileSelectionDialog(const gchar *title)
{
	GtkWidget *chooser;
	chooser = gtk_file_chooser_dialog_new(title, NULL, 
                        GTK_FILE_CHOOSER_ACTION_SAVE,
	   		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
			NULL);
	return(chooser);
}

GtkWidget*
createInputFileSelectionDialog(gchar *title, ggobid *gg)
{
       GtkWidget *chooser, *combo, *hbox, *lbl;
	   GList *els, *l;

       els = getInputPluginSelections(gg);

       chooser = gtk_file_chooser_dialog_new(title, NULL, GTK_FILE_CHOOSER_ACTION_OPEN,
	   		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
			NULL);
		
		hbox = gtk_hbox_new(false, 5);
		
		lbl = gtk_label_new_with_mnemonic("Input _Type:");
		gtk_box_pack_start(GTK_BOX(hbox), lbl, false, false, 0);
		
		combo = gtk_combo_box_new_text();
		gtk_label_set_mnemonic_widget(GTK_LABEL(lbl), combo);
		for(l = els; l; l = l->next) {
			gtk_combo_box_append_text(GTK_COMBO_BOX(combo), l->data);
			g_free(l->data);
		}
		g_list_free(els);
		gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
		gtk_box_pack_start(GTK_BOX(hbox), combo, false, false, 0);
		g_object_set_data(G_OBJECT(chooser), "PluginTypeCombo", combo);
		
		/*button = gtk_button_new_with_mnemonic("Enter _Location");
		gtk_box_pack_start(GTK_BOX(hbox), button, false, false, 0);
		g_signal_connect(G_OBJECT(button), "clicked", 
			G_CALLBACK(location_button_clicked_cb), chooser);*/
		
		gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(chooser), hbox);
		gtk_widget_show_all(hbox);
		
       return(chooser);
}
/*--------------------------------------------------------------------------*/
/*                    reading files                                         */
/*--------------------------------------------------------------------------*/

void
filename_get_r (ggobid *gg) 
{
  GtkWidget *chooser;
  chooser = createInputFileSelectionDialog("Read ggobi data", gg);

  if (gg->input && gg->input->baseName) {
    char buf[256];
    char *cwd;
    cwd = getcwd(buf, (size_t) 256);

    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), 
	 g_strdup_printf ("%s%c%s",
			  cwd,
			  G_DIR_SEPARATOR,
 			  gg->input->dirName));
    /*      gg->input->baseName); */
  }

  filename_get_configure (chooser, READ_FILESET, gg);

  if (gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_ACCEPT)
	  filesel_ok(chooser);
  gtk_widget_destroy(chooser);
}

/*--------------------------------------------------------------------------*/
/*                    writing files                                         */
/*--------------------------------------------------------------------------*/

void
filename_get_w (GtkWidget *w, ggobid *gg) 
{
  GtkWidget *chooser;
  const gchar *title;
  
  if (gg->save.format == XMLDATA)
    title = "Specify base name for new xml file";
  else
    title = "Specify base name for new file set";

  chooser = createOutputFileSelectionDialog(title);
  
  if (gg->input && gg->input->baseName) {
    char buf[256];
    char *cwd;
    cwd = getcwd(buf, (size_t) 256);

    /* This needs to be the full path name of the data directory */
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(chooser),
	 g_strdup_printf ("%s%c%s",
			  cwd,
			  G_DIR_SEPARATOR,
 			  gg->input->dirName));

    /* Come to think of it, it's dangerous to print the name of the
       current file, because it would be easy to overwrite it in error.
       dfs
    */
    /*
    gtk_file_chooser_set_filename(GTK_FILE_CHOOSER (chooser),
				  g_strdup_printf("%s%c%s.xml",
						 cwd, 
						 G_DIR_SEPARATOR,
                                  gg->input->baseName));
    */
  }

  filename_get_configure(chooser, WRITE_FILESET, gg);

  if (gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_ACCEPT)
	  filesel_ok(chooser);
  gtk_widget_destroy(chooser);
}
