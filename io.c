/*-- io.c -- routines that support the file io user interface --*/
/*--         (should be called fileio_ui.c)                   --*/
/*
    This software may only be used by you under license from AT&T Corp.
    ("AT&T").  A copy of AT&T's Source Code Agreement is available at
    AT&T's Internet website having the URL:
    <http://www.research.att.com/areas/stat/ggobi/license.html>
    If you received this software without first entering into a license
    with AT&T, you have an infringing copy of this software and cannot use
    it without violating AT&T's intellectual property rights.
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

#include "plugin.h"

#define READ_FILESET   0
#define EXTEND_FILESET 1
#define WRITE_FILESET  2



void
filesel_ok (GtkWidget *w, GtkFileSelection *fs)
{
  extern const gchar* const key_get (void);
  const gchar *fname;
  ggobid *gg;
  guint action, len;
  gchar *filename, *pluginModeName;
  gboolean firsttime;
  GtkWidget *combo;


  gg = (ggobid *) gtk_object_get_data (GTK_OBJECT (fs), key_get());
  fname = gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs));
  action = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (fs), "action"));
  len = strlen (fname);

  switch (action) {
    case READ_FILESET:
    {
      gint which;
      GGobiPluginInfo *plugin;
      combo = (GtkWidget *)gtk_object_get_data(GTK_OBJECT(fs), "PluginTypeCombo");
      pluginModeName = gtk_editable_get_chars(GTK_EDITABLE(GTK_COMBO(combo)->entry), 0, -1);
      which = *((gint *)gtk_object_get_data(GTK_OBJECT(fs), ".selectedElement"));
      plugin = getInputPluginByModeNameIndex(which);
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
        GGOBI(full_viewmode_set) (XYPLOT, gg);
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
}

static void
filename_get_configure (GtkWidget *fs, guint type, ggobid *gg) 
{
  extern const gchar* const key_get (void);

  const gchar *key = key_get();
  gtk_object_set_data (GTK_OBJECT (fs), "action", GINT_TO_POINTER (type));
  gtk_object_set_data (GTK_OBJECT (fs), key, gg);

  gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (fs)->ok_button),
                      "clicked", GTK_SIGNAL_FUNC (filesel_ok), (gpointer) fs);
                            
  /*-- Ensure that the dialog box is destroyed. --*/
    
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION (fs)->ok_button),
                             "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
                             (GtkObject*) fs);

  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION(fs)->cancel_button),
                             "clicked", GTK_SIGNAL_FUNC (gtk_widget_destroy),
                             (GtkObject*) fs);
}


GtkWidget *
getFileSelectionWorkContainer(GtkWidget *fs)
{
       GtkWidget *dlg, *vbox = NULL;
#if 0
       gtk_widget_realize(dlg);
       dlg = GTK_FILE_SELECTION(fs)->fileop_dialog;
       vbox = GTK_DIALOG(dlg)->vbox;
#else
       GList *kids;
       kids = gtk_container_children(GTK_CONTAINER(fs));
       dlg = g_list_nth_data(kids, 0);
       return(dlg);
/*
       kids = gtk_container_children(GTK_CONTAINER(dlg));
       vbox = g_list_nth_data(kids, 0);
       if(!GTK_IS_VBOX(vbox)) {
          g_printerr("Not vbox\n");
       }
*/
#endif

       return(vbox);
}

static void
filename_mode_selection_cb(GtkList *l, GtkWidget *el, GtkWidget *data)
{
    
    gint *i = gtk_object_get_data(GTK_OBJECT(data), ".selectedElement");
    if(i) {
      *i = gtk_list_child_position(l, el);
    }
}

void
free_gdata(GtkObject *src, gpointer data)
{
  g_free(data);
}

GtkWidget*
createInputFileSelectionDialog(gchar *title, ggobid *gg, GtkWidget **ocombo)
{
       GtkWidget *fs, *vbox, *combo, *box;
       GList *els;
       gint *i;

       els = getInputPluginSelections(gg);

       fs = gtk_file_selection_new(title);
       vbox = getFileSelectionWorkContainer(fs);

       box = gtk_frame_new("Reader Type");

       combo = gtk_combo_new();
       gtk_object_set_data(GTK_OBJECT(fs), "PluginTypeCombo", combo);
       if(ocombo)
           *ocombo = combo;
       gtk_combo_set_popdown_strings(GTK_COMBO(combo), els);

       gtk_container_add(GTK_CONTAINER(box), combo);
       gtk_widget_show_all(box);


       gtk_box_pack_start(GTK_BOX(vbox), box, false, false, 3);
        /* Shouldn't need to do this if we had a real dialog with action area and work area. */
       gtk_box_reorder_child(GTK_BOX(vbox), combo, 4);

       i = (gint *) g_malloc(sizeof(gint));
/*XXX Need to free this when we destroy the fileselection widget */
       gtk_signal_connect(GTK_OBJECT(fs), "destroy", 
         GTK_SIGNAL_FUNC(free_gdata), i);

       *i = -1;
       gtk_object_set_data(GTK_OBJECT(fs), ".selectedElement", i);
       gtk_signal_connect(GTK_OBJECT(GTK_COMBO(combo)->list), "select-child",
         GTK_SIGNAL_FUNC(filename_mode_selection_cb), fs);



       return(fs);
}
/*--------------------------------------------------------------------------*/
/*                    reading files                                         */
/*--------------------------------------------------------------------------*/

void
filename_get_r (ggobid *gg, guint action, GtkWidget *w) 
{
  GtkWidget *fs, *combo;
  fs = createInputFileSelectionDialog("read ggobi data", gg, &combo);
  gtk_file_selection_hide_fileop_buttons (GTK_FILE_SELECTION (fs)); 

  /*
   * I would like to put in the directory here without the
   * filename, but I don't know how -- dfs
  */
  if (gg->input && gg->input->baseName)
    gtk_file_selection_set_filename (GTK_FILE_SELECTION (fs),
                                     gg->input->baseName);

  filename_get_configure (fs, READ_FILESET, gg);

  gtk_widget_show (fs);
}

/*--------------------------------------------------------------------------*/
/*                    writing files                                         */
/*--------------------------------------------------------------------------*/

void
filename_get_w (GtkWidget *w, ggobid *gg) 
{
  GtkWidget *fs;

  if (gg->save.format == XMLDATA)
    fs = gtk_file_selection_new ("Specify base name for new xml file");
  else
    fs = gtk_file_selection_new ("Specify base name for new file set");

  gtk_file_selection_hide_fileop_buttons (GTK_FILE_SELECTION (fs));

  /*
   * I would like to put in the directory here without the
   * filename, but I don't know how -- dfs
  */
  if (gg->input && gg->input->baseName)
    gtk_file_selection_set_filename (GTK_FILE_SELECTION (fs),
                                     gg->input->baseName);

  filename_get_configure (fs, WRITE_FILESET, gg);

  gtk_widget_show (fs);
}
